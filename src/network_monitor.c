#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "monitor.h"

void obter_uso_rede(int pid, MetricasProcesso *dados) {
    char path[64];
    FILE *fp;

    // Zera os campos antes de ler
    dados->bytes_rx = 0;
    dados->bytes_tx = 0;
    dados->packets_rx = 0;
    dados->packets_tx = 0;
    dados->conexoes_ativas = 0;

    // -------- LEITURA DO TRÁFEGO GLOBAL /proc/net/dev --------
    fp = fopen("/proc/net/dev", "r");
    if (!fp) return;

    char linha[256];
    int linha_count = 0;

    // Percorre interfaces listadas no arquivo
    while (fgets(linha, sizeof(linha), fp)) {
        if (++linha_count <= 2) continue;  // pula cabeçalhos

        char iface[32];
        long rx_bytes, rx_packets, tx_bytes, tx_packets;

        // Lê bytes e pacotes recebidos/enviados
        sscanf(linha,
               " %[^:]: %ld %ld %*d %*d %*d %*d %*d %*d %ld %ld",
               iface, &rx_bytes, &rx_packets, &tx_bytes, &tx_packets);

        dados->bytes_rx += rx_bytes;
        dados->packets_rx += rx_packets;
        dados->bytes_tx += tx_bytes;
        dados->packets_tx += tx_packets;
    }
    fclose(fp);

    // -------- CONTAGEM DE CONEXÕES TCP DO PROCESSO --------
    sprintf(path, "/proc/%d/net/tcp", pid);
    fp = fopen(path, "r");
    if (!fp) return;

    int count = 0;

    // Cada linha representa uma conexão TCP
    while (fgets(linha, sizeof(linha), fp)) {
        if (strstr(linha, "sl")) continue;  // pula cabeçalho
        count++;
    }

    fclose(fp);

    // Salva total de conexões
    dados->conexoes_ativas = count;
}
