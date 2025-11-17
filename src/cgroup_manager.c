#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h> 
#include "cgroup.h"

#define CGROUP_BASE "/sys/fs/cgroup"

// Verifica se o diretório do cgroup existe
int cgroup_existe(const char *nome) {
    char caminho[256];
    snprintf(caminho, sizeof(caminho), "%s/%s", CGROUP_BASE, nome);
    return access(caminho, F_OK) == 0;
}

// Cria um novo cgroup (diretório dentro de /sys/fs/cgroup)
int cgroup_criar(const char *nome) {
    char caminho[256];
    snprintf(caminho, sizeof(caminho), "%s/%s", CGROUP_BASE, nome);

    if (mkdir(caminho, 0755) != 0) {
        perror("Erro ao criar cgroup");
        return -1;
    }

    printf("[OK] cgroup criado: %s\n", caminho);
    return 0;
}

// Move um processo (PID) para o cgroup escrevendo em cgroup.procs
int cgroup_mover_processo(const char *nome, int pid) {
    char caminho[256];
    snprintf(caminho, sizeof(caminho), "%s/%s/cgroup.procs", CGROUP_BASE, nome);

    FILE *f = fopen(caminho, "w");
    if (!f) {
        perror("Erro ao abrir cgroup.procs para escrever");
        return -1;
    }

    // Atribui o PID ao cgroup
    fprintf(f, "%d", pid);
    fclose(f);

    printf("[OK] Processo %d movido para cgroup '%s'\n", pid, nome);
    return 0;
}

// Aplica limites de CPU e memória no cgroup
int cgroup_aplicar_limites(const char *nome, int limite_cpu_usec, long limite_mem_bytes) {
    char caminho[256];
    FILE *f;

    // Limite de CPU → escreve em cpu.max (quota + periodo)
    snprintf(caminho, sizeof(caminho), "%s/%s/cpu.max", CGROUP_BASE, nome);
    f = fopen(caminho, "w");
    if (!f) {
        perror("Erro ao aplicar limite de CPU");
        return -1;
    }
    fprintf(f, "%d 100000", limite_cpu_usec);
    fclose(f);

    // Limite de memória → memory.max
    snprintf(caminho, sizeof(caminho), "%s/%s/memory.max", CGROUP_BASE, nome);
    f = fopen(caminho, "w");
    if (!f) {
        perror("Erro ao aplicar limite de memória");
        return -1;
    }
    fprintf(f, "%ld", limite_mem_bytes);
    fclose(f);

    printf("[OK] Limites aplicados: CPU=%dus, Memória=%ld bytes\n",
           limite_cpu_usec, limite_mem_bytes);

    return 0;
}

// Lê um arquivo linha a linha e imprime na tela
static void imprimir_arquivo_no_terminal(FILE *f) {
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), f)) {
        printf("%s", buffer);  // imprime mantendo as quebras de linha
    }
}

// Mostra estatísticas completas do cgroup direto do filesystem
int cgroup_ler_e_mostrar_estatisticas(const char *nome) {
    char caminho[256];
    FILE *f;

    printf("\n==================== ESTATÍSTICAS: %s ====================\n", nome);

    // --- CPU ---
    snprintf(caminho, sizeof(caminho), "%s/%s/cpu.stat", CGROUP_BASE, nome);
    printf("\n--- CPU (cpu.stat) ---\n");
    f = fopen(caminho, "r");
    if (f) { imprimir_arquivo_no_terminal(f); fclose(f); }
    else printf("(cpu.stat não disponível)\n");

    // Limite de CPU
    snprintf(caminho, sizeof(caminho), "%s/%s/cpu.max", CGROUP_BASE, nome);
    printf("\n--- LIMITE DE CPU (cpu.max) ---\n");
    f = fopen(caminho, "r");
    if (f) { imprimir_arquivo_no_terminal(f); fclose(f); }
    else printf("(cpu.max não disponível)\n");

    // --- Memória ---
    snprintf(caminho, sizeof(caminho), "%s/%s/memory.stat", CGROUP_BASE, nome);
    printf("\n--- MEMÓRIA (memory.stat) ---\n");
    f = fopen(caminho, "r");
    if (f) { imprimir_arquivo_no_terminal(f); fclose(f); }
    else printf("(memory.stat não disponível)\n");

    snprintf(caminho, sizeof(caminho), "%s/%s/memory.max", CGROUP_BASE, nome);
    printf("\n--- LIMITE DE MEMÓRIA (memory.max) ---\n");
    f = fopen(caminho, "r");
    if (f) { imprimir_arquivo_no_terminal(f); fclose(f); }
    else printf("(memory.max não disponível)\n");

    // --- IO ---
    snprintf(caminho, sizeof(caminho), "%s/%s/io.stat", CGROUP_BASE, nome);
    printf("\n--- I/O (io.stat) ---\n");
    f = fopen(caminho, "r");
    if (f) { imprimir_arquivo_no_terminal(f); fclose(f); }
    else printf("(io.stat não disponível)\n");

    // --- PIDs ---
    snprintf(caminho, sizeof(caminho), "%s/%s/pids.current", CGROUP_BASE, nome);
    printf("\n--- PIDs (pids.current) ---\n");
    f = fopen(caminho, "r");
    if (f) { imprimir_arquivo_no_terminal(f); fclose(f); }
    else printf("(pids.current não disponível)\n");

    snprintf(caminho, sizeof(caminho), "%s/%s/pids.max", CGROUP_BASE, nome);
    printf("\n--- Limite de PIDs (pids.max) ---\n");
    f = fopen(caminho, "r");
    if (f) { imprimir_arquivo_no_terminal(f); fclose(f); }
    else printf("(pids.max não disponível)\n");

    printf("\n=========================================================\n");
    return 0;
}

// Gera um JSON com as estatísticas do cgroup
int cgroup_gerar_relatorio(const char *nome) {
    char caminho_json[256];
    snprintf(caminho_json, sizeof(caminho_json), "docs/relatorio_cgroup.json");

    FILE *json = fopen(caminho_json, "w");
    if (!json) {
        perror("Erro ao criar relatorio_cgroup.json");
        return -1;
    }

    // Timestamp atual para registrar no JSON
    time_t agora = time(NULL);
    char tempo_buf[64];
    strftime(tempo_buf, sizeof(tempo_buf), "%Y-%m-%d %H:%M:%S", localtime(&agora));

    fprintf(json, "{\n");
    fprintf(json, "  \"cgroup\": \"%s\",\n", nome);
    fprintf(json, "  \"gerado_em\": \"%s\",\n", tempo_buf);

    char caminho[256], key[128];
    long long value;
    FILE *f;

    // ----- CPU -----
    fprintf(json, "  \"cpu\": {\n");
    snprintf(caminho, sizeof(caminho), "%s/%s/cpu.stat", CGROUP_BASE, nome);
    f = fopen(caminho, "r");
    if (f) {
        while (fscanf(f, "%127s %lld", key, &value) == 2)
            fprintf(json, "    \"%s\": %lld,\n", key, value);
        fclose(f);
    }
    fprintf(json, "  },\n");

    // ----- MEMÓRIA -----
    fprintf(json, "  \"memory\": {\n");
    snprintf(caminho, sizeof(caminho), "%s/%s/memory.stat", CGROUP_BASE, nome);
    f = fopen(caminho, "r");
    if (f) {
        while (fscanf(f, "%127s %lld", key, &value) == 2)
            fprintf(json, "    \"%s\": %lld,\n", key, value);
        fclose(f);
    }
    fprintf(json, "  },\n");

    // ----- IO -----
    fprintf(json, "  \"io\": {\n");
    snprintf(caminho, sizeof(caminho), "%s/%s/io.stat", CGROUP_BASE, nome);
    f = fopen(caminho, "r");
    if (f) {
        while (fscanf(f, "%127s", key) == 1) {
            if (strchr(key, '=')) {
                char *eq = strchr(key, '=');
                *eq = '\0';
                value = atoll(eq + 1);
                fprintf(json, "    \"%s\": %lld,\n", key, value);
            }
        }
        fclose(f);
    }
    fprintf(json, "  },\n");

    // ----- PIDs -----
    fprintf(json, "  \"pids\": {\n");

    snprintf(caminho, sizeof(caminho), "%s/%s/pids.current", CGROUP_BASE, nome);
    f = fopen(caminho, "r");
    if (f) {
        fscanf(f, "%lld", &value);
        fprintf(json, "    \"current\": %lld,\n", value);
        fclose(f);
    }

    snprintf(caminho, sizeof(caminho), "%s/%s/pids.max", CGROUP_BASE, nome);
    f = fopen(caminho, "r");
    if (f) {
        char buf[64];
        fscanf(f, "%63s", buf);
        fprintf(json, "    \"max\": \"%s\"\n", buf);
        fclose(f);
    }

    fprintf(json, "  }\n");
    fprintf(json, "}\n");
    fclose(json);

    printf("[OK] Relatório JSON salvo em docs/relatorio_cgroup.json\n");
    return 0;
}
