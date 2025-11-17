#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "monitor.h"   // Biblioteca real que contém obter_uso_memoria()

/*
 * Verifica se o processo existe verificando se /proc/<pid> existe.
 */
int processo_existe(int pid) {
    char caminho[64];
    sprintf(caminho, "/proc/%d", pid);
    return access(caminho, F_OK) == 0;
}

int main(void) {
    int pid;
    int duration_sec;

    printf("===== TESTE DE MEMÓRIA =====\n\n");

    printf("Digite o PID do processo: ");
    if (scanf("%d", &pid) != 1) {
        fprintf(stderr, "Erro ao ler PID.\n");
        return 1;
    }

    printf("Digite o tempo de monitoramento (segundos): ");
    if (scanf("%d", &duration_sec) != 1 || duration_sec <= 0) {
        fprintf(stderr, "Tempo invalido.\n");
        return 1;
    }

    for (int i = 0; i < duration_sec; i++) {
        MetricasProcesso sample;

        // Verifica se processo ainda existe
        if (!processo_existe(pid)) {
            fprintf(stderr, "Processo %d não existe mais.\n", pid);
            return 1;
        }

        // Chama função oficial do monitor
        obter_uso_memoria(pid, &sample);

        // Gera timestamp
        time_t agora = time(NULL);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&agora));

        // Imprime relatório simples
        printf("\n---------------------------------------------\n");
        printf(" PID               : %d\n", sample.pid);
        printf(" Timestamp         : %s\n", buf);
        printf(" RSS               : %10ld bytes\n", sample.memoria_residente);
        printf(" VSZ               : %10ld bytes\n", sample.memoria_virtual);
        printf(" PageFaults (min)  : %10lu\n", sample.page_faults_minflt);
        printf(" PageFaults (maj)  : %10lu\n", sample.page_faults_majflt);
        printf(" Swap              : %10ld bytes\n", sample.memoria_swap);
        printf("---------------------------------------------\n");

        sleep(1);
    }

    return 0;
}
