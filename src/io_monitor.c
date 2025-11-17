#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "monitor.h"

void obter_uso_io(int pid, MetricasProcesso *dados) {
    char caminho[64];
    FILE *arquivo;

    // Monta o caminho para o arquivo de estatísticas de IO do processo
    sprintf(caminho, "/proc/%d/io", pid);
    arquivo = fopen(caminho, "r");
    if (!arquivo) {
        perror("Erro ao abrir /proc/[pid]/io");  // Aviso caso o processo tenha terminado ou não exista
        return;
    }

    // Inicializa os campos antes da leitura
    dados->bytes_lidos = 0;
    dados->bytes_escritos = 0;
    dados->syscr = 0;   // syscalls de leitura
    dados->syscw = 0;   // syscalls de escrita
    dados->syscall = 0;
    dados->disk_ops = 0;

    char linha[128];

    // Percorre linha a linha do /proc/[pid]/io
    while (fgets(linha, sizeof(linha), arquivo)) {

        // Cada linha possui o formato "chave: valor"
        if (strncmp(linha, "read_bytes:", 11) == 0)
            sscanf(linha, "read_bytes: %ld", &dados->bytes_lidos);

        else if (strncmp(linha, "write_bytes:", 12) == 0)
            sscanf(linha, "write_bytes: %ld", &dados->bytes_escritos);

        else if (strncmp(linha, "syscr:", 6) == 0)
            sscanf(linha, "syscr: %ld", &dados->syscr);

        else if (strncmp(linha, "syscw:", 6) == 0)
            sscanf(linha, "syscw: %ld", &dados->syscw);
    }

    fclose(arquivo);

    // Soma total de syscalls de IO
    dados->syscall = dados->syscr + dados->syscw;

    // Estimativa de operações reais de disco em blocos de 4 KB
    dados->disk_ops = (dados->bytes_lidos + dados->bytes_escritos) / 4096;
}
