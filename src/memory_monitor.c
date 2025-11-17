#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "monitor.h"

void obter_uso_memoria(int pid, MetricasProcesso *dados) {
    char caminho[64], linha[256];
    FILE *arquivo;

    // Caminho para /proc/<pid>/status — contém RSS, memória virtual e swap
    sprintf(caminho, "/proc/%d/status", pid);
    arquivo = fopen(caminho, "r");
    if (!arquivo) return;  // Processo pode ter terminado

    // Varre linha por linha procurando os campos de memória
    while (fgets(linha, sizeof(linha), arquivo)) {
        if (strncmp(linha, "VmRSS:", 6) == 0)
            sscanf(linha, "VmRSS: %ld", &dados->memoria_residente);   // Memória física usada
        else if (strncmp(linha, "VmSize:", 7) == 0)
            sscanf(linha, "VmSize: %ld", &dados->memoria_virtual);    // Espaço total alocado
        else if (strncmp(linha, "VmSwap:", 7) == 0)
            sscanf(linha, "VmSwap: %ld", &dados->memoria_swap);       // Swap utilizado
    }

    fclose(arquivo);

    // Agora abrimos /proc/<pid>/stat para coletar page faults
    sprintf(caminho, "/proc/%d/stat", pid);
    arquivo = fopen(caminho, "r");
    if (!arquivo) return;

    // Page faults leves (minflt) e pesados (majflt)
    long minflt, majflt;

    // Ignoramos vários campos até chegar no 10° e 12° valores
    fscanf(arquivo,
           "%*d %*s %*c "          // pid, nome, estado
           "%*d %*d %*d %*d %*d "  // prioridade, threads, etc.
           "%*u %*u %ld %*u %ld",  // minflt (10°), ignora 11°, majflt (12°)
           &minflt, &majflt);

    dados->page_faults_minflt = minflt;  // Falhas de página sem acesso ao disco
    dados->page_faults_majflt = majflt;  // Falhas de página com acesso ao disco

    fclose(arquivo);

    // Guarda o PID correspondente aos dados coletados
    dados->pid = pid;
}
