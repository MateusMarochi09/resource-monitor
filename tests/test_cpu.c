#include <stdio.h>
#include <unistd.h>
#include <time.h>

// Estrutura usada apenas neste teste para armazenar
// métricas básicas de CPU coletadas do /proc/<pid>.
typedef struct {
    int pid;
    double cpu_percent;   // Porcentagem de uso de CPU (calculada pelo delta de utime e stime)
    double user_time;     // Tempo em modo usuário
    double system_time;   // Tempo em modo kernel
    int threads;          // Número de threads do processo
    long ctx_switches;    // Trocas de contexto voluntárias + involuntárias
} MetricasProcessoCPU;

 // Verifica se o processo existe conferindo
 // se o diretório /proc/<pid> está presente.
int processo_existe(int pid) {
    char path[64];
    sprintf(path, "/proc/%d", pid);
    return access(path, F_OK) == 0;
}

// Obtém métricas de CPU localmente, sem conflito com a função do monitor real.
// Lê dados de /proc/<pid>/stat e /proc/<pid>/status.
int obter_uso_cpu_local(int pid, MetricasProcessoCPU *prev, MetricasProcessoCPU *sample) {
    char path[64], buf[128];
    sprintf(path, "/proc/%d/stat", pid);

    FILE *fp = fopen(path, "r");
    if (!fp) return 0;

    // Campos que queremos capturar
    char tmp[64];
    long utime, stime;
    int threads;

    //fscanf com %* ignora campos que não queremos.
    //Lemos utime, stime e número de threads
    fscanf(
        fp,
        "%*d %s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u "
        "%ld %ld %*d %*d %*d %*d %d",
        tmp, &utime, &stime, &threads
    );
    fclose(fp);

    // Leitura de trocas de contexto no /proc/<pid>/status
    long voluntary = 0, nonvoluntary = 0;
    sprintf(path, "/proc/%d/status", pid);
    fp = fopen(path, "r");
    if (!fp) return 0;

    while (fgets(buf, sizeof(buf), fp)) {
        sscanf(buf, "voluntary_ctxt_switches: %ld", &voluntary);
        sscanf(buf, "nonvoluntary_ctxt_switches: %ld", &nonvoluntary);
    }
    fclose(fp);

    long ticks = sysconf(_SC_CLK_TCK); // ticks por segundo

    // Preenche a struct de saída
    sample->pid = pid;
    sample->user_time = (double)utime / ticks;
    sample->system_time = (double)stime / ticks;
    sample->threads = threads;
    sample->ctx_switches = voluntary + nonvoluntary;


    //Se há uma leitura anterior, calculamos o delta para estimar o uso de CPU.
    if (prev != NULL) {
        long delta_utime = utime - (long)(prev->user_time * ticks);
        long delta_stime = stime - (long)(prev->system_time * ticks);
        sample->cpu_percent = (double)(delta_utime + delta_stime) * 100.0;
    } else {
        sample->cpu_percent = 0.0;
    }

    return 1;
}

int main(void) {
    int pid, duration;

    printf("===== TESTE DE CPU =====\nPID: ");
    scanf("%d", &pid);

    printf("Digite o tempo de monitoramento (segundos): ");
    scanf("%d", &duration);

    // Struct que guarda a leitura anterior
    MetricasProcessoCPU prev = {0};

    for (int i = 0; i < duration; i++) {
        MetricasProcessoCPU sample;

        if (!processo_existe(pid)) {
            printf("Processo %d não existe.\n", pid);
            break;
        }

        // Envia prev apenas a partir da segunda medição
        obter_uso_cpu_local(pid, i == 0 ? NULL : &prev, &sample);

        // Timestamp bonito
        time_t now = time(NULL);
        char tbuf[32];
        strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", localtime(&now));

        // Imprime relatório simples
        printf(
            "\n------------------------------\n"
            "PID: %d\n"
            "Timestamp: %s\n"
            "CPU %%: %.2f\n"
            "User: %.2fs\n"
            "System: %.2fs\n"
            "Threads: %d\n"
            "CtxSwitch: %ld\n"
            "------------------------------\n",
            sample.pid, tbuf, sample.cpu_percent,
            sample.user_time, sample.system_time,
            sample.threads, sample.ctx_switches
        );

        // Atualiza o anterior
        prev = sample;
        sleep(1);
    }

    return 0;
}
