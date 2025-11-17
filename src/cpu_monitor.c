#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "monitor.h"

// Função auxiliar para obter tempo total da CPU (de /proc/stat)
static long obter_tempo_total_cpu() {
    FILE *fp = fopen("/proc/stat", "r");            // abre /proc/stat
    if (!fp) return -1;                             // falha ao abrir

    char linha[512];
    fgets(linha, sizeof(linha), fp);                // lê primeira linha
    fclose(fp);                                     // fecha arquivo

    char cpu_label[10];
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
    sscanf(linha, "%s %lu %lu %lu %lu %lu %lu %lu %lu",
           cpu_label, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal); // extrai campos

    return user + nice + system + idle + iowait + irq + softirq + steal; // soma total
}

// Função principal: coleta CPU e trocas de contexto
void obter_uso_cpu(int pid, MetricasProcesso *dados) {
    char caminho_proc[256];
    FILE *arquivo;

    // ======= Parte 1: Leitura de /proc/[pid]/stat =======
    sprintf(caminho_proc, "/proc/%d/stat", pid);    // monta caminho
    arquivo = fopen(caminho_proc, "r");             // abre stat
    if (!arquivo) {
        perror("Erro ao abrir /proc/[pid]/stat");
        return;
    }

    char buffer[4096];
    fgets(buffer, sizeof(buffer), arquivo);         // lê linha
    fclose(arquivo);

    unsigned long utime = 0, stime = 0, threads = 0;
    unsigned long page_faults_minflt = 0;
    unsigned long page_faults_majflt = 0;
    char *token = strtok(buffer, " ");              // tokeniza campos
    int i = 1;

    while (token != NULL && i <= 24) {              // percorre campos
        if (i == 10) page_faults_minflt = atol(token);  // falhas menores
        if (i == 12) page_faults_majflt = atol(token);  // falhas maiores
        if (i == 14) utime = atol(token);               // tempo user
        if (i == 15) stime = atol(token);               // tempo kernel
        if (i == 20) threads = atol(token);             // nº threads
        token = strtok(NULL, " ");                      // próximo campo
        i++;
    }

    dados->tempo_usuario = utime;                  // salva tempo user
    dados->tempo_sistema = stime;                  // salva tempo kernel
    dados->threads = threads;                      // salva threads
    dados->page_faults_minflt = page_faults_minflt; // salva minor faults
    dados->page_faults_majflt = page_faults_majflt; // salva major faults

    // ======= Parte 2: Trocas de contexto (/proc/[pid]/status) =======
    sprintf(caminho_proc, "/proc/%d/status", pid);  // caminho status
    arquivo = fopen(caminho_proc, "r");             // abre status
    if (!arquivo) {
        perror("Erro ao abrir /proc/[pid]/status");
        return;
    }

    char linha[512];
    dados->trocas_ctx_voluntarias = 0;             // inicializa contadores
    dados->trocas_ctx_involuntarias = 0;

    while (fgets(linha, sizeof(linha), arquivo)) { // lê linha a linha
        if (strstr(linha, "voluntary_ctxt_switches:"))  // procura voluntárias
            sscanf(linha, "voluntary_ctxt_switches: %lu",
                   &dados->trocas_ctx_voluntarias);
        else if (strstr(linha, "nonvoluntary_ctxt_switches:")) // procura involuntárias
            sscanf(linha, "nonvoluntary_ctxt_switches: %lu",
                   &dados->trocas_ctx_involuntarias);
    }
    fclose(arquivo);

    // ======= Parte 3: Cálculo do uso de CPU (%) =======
    MetricasProcesso antes = *dados;              // copia valores iniciais
    long total_antes = obter_tempo_total_cpu();   // tempo total antes

    usleep(500000);                               // espera 0.5s

    sprintf(caminho_proc, "/proc/%d/stat", pid);  // reabre stat
    arquivo = fopen(caminho_proc, "r");
    if (!arquivo) return;
    fgets(buffer, sizeof(buffer), arquivo);       // lê nova linha
    fclose(arquivo);

    utime = stime = 0;
    token = strtok(buffer, " ");                  // tokeniza novamente
    i = 1;
    while (token != NULL && i <= 15) {
        if (i == 14) utime = atol(token);         // novo tempo user
        if (i == 15) stime = atol(token);         // novo tempo system
        token = strtok(NULL, " ");
        i++;
    }

    long total_depois = obter_tempo_total_cpu();  // total depois

    long delta_proc = (utime + stime) - (antes.tempo_usuario + antes.tempo_sistema); // variação proc
    long delta_total = total_depois - total_antes;                                   // variação total

    double uso = 0.0;
    if (delta_total > 0)
        uso = 100.0 * ((double)delta_proc / (double)delta_total); // calcula %

    dados->cpu_porcentagem = uso;                 // salva resultado
}
