#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>


//TESTE DE CPU
typedef struct {
    int pid;
    double cpu_percent;
    double user_time;
    double system_time;
    int threads;
    long ctx_switches;
} MetricasProcessoCPU;

// Verifica se o processo existe
int processo_existe(int pid) {
    char path[64];
    sprintf(path, "/proc/%d", pid);
    return access(path, F_OK) == 0; // retorna 1 se existe
}

// Coleta métricas de CPU de um processo
int obter_uso_cpu_local(int pid, MetricasProcessoCPU *prev, MetricasProcessoCPU *sample) {
    char path[64], buf[128];
    sprintf(path, "/proc/%d/stat", pid);

    FILE *fp = fopen(path, "r");
    if (!fp) return 0; // processo não existe

    char tmp[64];
    long utime, stime;
    int threads;

    fscanf(
        fp,
        "%*d %s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u "
        "%ld %ld %*d %*d %*d %*d %d",
        tmp, &utime, &stime, &threads
    );
    fclose(fp);

    long voluntary = 0, nonvoluntary = 0;
    sprintf(path, "/proc/%d/status", pid);
    fp = fopen(path, "r");
    if (!fp) return 0; // não conseguiu abrir status
    while (fgets(buf, sizeof(buf), fp)) {
        sscanf(buf, "voluntary_ctxt_switches: %ld", &voluntary);
        sscanf(buf, "nonvoluntary_ctxt_switches: %ld", &nonvoluntary);
    }
    fclose(fp);

    long ticks = sysconf(_SC_CLK_TCK); // conversão ticks → segundos

    sample->pid = pid;
    sample->user_time = (double)utime / ticks;
    sample->system_time = (double)stime / ticks;
    sample->threads = threads;
    sample->ctx_switches = voluntary + nonvoluntary;

    if (prev != NULL) { // calcula delta de CPU
        long delta_utime = utime - (long)(prev->user_time * ticks);
        long delta_stime = stime - (long)(prev->system_time * ticks);
        sample->cpu_percent = (double)(delta_utime + delta_stime) * 100.0;
    } else {
        sample->cpu_percent = 0.0; // primeira amostra
    }

    return 1;
}


// TESTE DE I/O
void run_io_test(int pid, int duration) {
    char path[64], buf[128];

    for (int i = 0; i < duration; i++) {
        unsigned long read_bytes = 0, write_bytes = 0;
        unsigned long syscr = 0, syscw = 0;

        sprintf(path, "/proc/%d/io", pid);
        FILE *fp = fopen(path, "r");
        if (!fp) {
            printf("Processo %d não existe ou não é possível acessar.\n", pid);
            break; // encerra se não existe
        }

        while (fgets(buf, sizeof(buf), fp)) {
            sscanf(buf, "read_bytes: %lu", &read_bytes);
            sscanf(buf, "write_bytes: %lu", &write_bytes);
            sscanf(buf, "syscr: %lu", &syscr);
            sscanf(buf, "syscw: %lu", &syscw);
        }
        fclose(fp);

        double r_mb = read_bytes / (1024.0 * 1024.0); // bytes → MB
        double w_mb = write_bytes / (1024.0 * 1024.0);

        time_t now = time(NULL);
        char ts[32];
        strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));

        printf(
            "\n===== TESTE DE I/O =====\nPID %d | %s\n"
            "Syscalls: %lu (R:%lu / W:%lu) | Leitura: %.2f MB | Escrita: %.2f MB | DiskOps: %lu\n",
            pid, ts, syscr + syscw, syscr, syscw, r_mb, w_mb, read_bytes + write_bytes
        );

        sleep(1);
    }
}


//TESTE DE MEMÓRIA
typedef struct {
    int pid;
    long memoria_residente;
    long memoria_virtual;
    unsigned long page_faults_minflt;
    unsigned long page_faults_majflt;
    long memoria_swap;
} MetricasProcessoMem;

// Coleta métricas de memória
void obter_uso_memoria(int pid, MetricasProcessoMem *sample) {
    char path[64];
    sprintf(path, "/proc/%d/statm", pid);
    FILE *fp = fopen(path, "r");
    if (!fp) return; // processo não existe

    long rss, vsz;
    fscanf(fp, "%ld %ld", &vsz, &rss);
    fclose(fp);

    sample->pid = pid;
    sample->memoria_residente = rss * sysconf(_SC_PAGESIZE); // páginas → bytes
    sample->memoria_virtual = vsz * sysconf(_SC_PAGESIZE);
    sample->memoria_swap = 0; // simplificação
    sample->page_faults_minflt = 0;
    sample->page_faults_majflt = 0;
}

void run_mem_test(int pid, int duration) {
    for (int i = 0; i < duration; i++) {
        MetricasProcessoMem sample;
        if (!processo_existe(pid)) { // verifica existência
            printf("Processo %d não existe mais.\n", pid);
            break;
        }

        obter_uso_memoria(pid, &sample);

        time_t now = time(NULL);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));

        printf(
            "\n===== TESTE DE MEMÓRIA =====\nPID: %d | Timestamp: %s\n"
            "RSS: %ld bytes | VSZ: %ld bytes | Swap: %ld bytes\n",
            sample.pid, buf, sample.memoria_residente, sample.memoria_virtual, sample.memoria_swap
        );

        sleep(1);
    }
}


 //MAIN - ALL TESTS
int main() {
    int pid, duration;

    printf("Digite o PID do processo a ser monitorado: ");
    scanf("%d", &pid);

    printf("Digite o tempo de monitoramento (s): ");
    scanf("%d", &duration);

    MetricasProcessoCPU prev_cpu = {0}; // armazena dados da CPU da última amostra

    // Teste CPU
    for (int i = 0; i < duration; i++) {
        MetricasProcessoCPU sample;
        if (!processo_existe(pid)) break; // interrompe se processo finalizou

        obter_uso_cpu_local(pid, i == 0 ? NULL : &prev_cpu, &sample);

        time_t now = time(NULL);
        char tbuf[32];
        strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", localtime(&now));

        printf(
            "\n===== TESTE DE CPU =====\nPID: %d | Timestamp: %s\nCPU %%: %.2f | User: %.2fs | System: %.2fs | Threads: %d | CtxSwitch: %ld\n",
            sample.pid, tbuf, sample.cpu_percent, sample.user_time, sample.system_time,
            sample.threads, sample.ctx_switches
        );

        prev_cpu = sample; // atualiza último estado
        sleep(1);
    }

    // Teste I/O
    run_io_test(pid, duration);

    // Teste Memória
    run_mem_test(pid, duration);

    printf("\n===== TODOS OS TESTES CONCLUÍDOS =====\n");
    return 0;
}
