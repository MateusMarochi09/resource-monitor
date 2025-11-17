#include <stdio.h>
#include <unistd.h>
#include <time.h>

int main() {
    int pid, duration;

    printf("===== TESTE DE I/O =====\nPID: ");
    scanf("%d", &pid);

    printf("Digite o tempo de monitoramento (segundos): ");
    scanf("%d", &duration);

    for (int i = 0; i < duration; i++) {
        char path[64], buf[128];

        // Valores lidos do /proc/<pid>/io
        unsigned long read_bytes = 0, write_bytes = 0;
        unsigned long syscr = 0, syscw = 0;

        sprintf(path, "/proc/%d/io", pid);
        FILE *fp = fopen(path, "r");

        if (!fp) {
            printf("Processo %d não existe ou não é possível acessar.\n", pid);
            break;
        }


         // Cada linha do /proc/<pid>/io contém uma métrica.
         // Usamos sscanf para capturar só as que queremos.
        while (fgets(buf, sizeof(buf), fp)) {
            sscanf(buf, "read_bytes: %lu", &read_bytes);
            sscanf(buf, "write_bytes: %lu", &write_bytes);
            sscanf(buf, "syscr: %lu", &syscr);
            sscanf(buf, "syscw: %lu", &syscw);
        }
        fclose(fp);

        // Conversão para megabytes
        double r_mb = read_bytes / (1024.0 * 1024.0);
        double w_mb = write_bytes / (1024.0 * 1024.0);

        // Timestamp
        time_t now = time(NULL);
        char ts[32];
        strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));

        // Saída simples de monitoramento
        printf(
            "PID %d | %s\n"
            "Syscalls: %lu (R:%lu / W:%lu) | "
            "Leitura: %.2f MB | Escrita: %.2f MB | DiskOps: %lu\n\n",
            pid, ts, syscr + syscw, syscr, syscw,
            r_mb, w_mb, read_bytes + write_bytes
        );

        sleep(1);
    }

    return 0;
}
