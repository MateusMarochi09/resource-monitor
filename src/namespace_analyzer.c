#define _GNU_SOURCE
#include <sched.h>
#include "namespace.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <sys/time.h>

#define TAM_BUFFER 1024

// =============================
// LISTAR NAMESPACES DE UM PID
// =============================
void listar_namespaces(int pid) {
    char caminho_diretorio[TAM_BUFFER], caminho_link[TAM_BUFFER];

    // caminho para /proc/PID/ns
    snprintf(caminho_diretorio, sizeof(caminho_diretorio), "/proc/%d/ns", pid);

    // abre diretório /proc/PID/ns
    DIR *diretorio = opendir(caminho_diretorio);
    if (!diretorio) {
        perror("Erro ao abrir diretório /proc");
        return;
    }

    struct dirent *entrada;
    printf("\nNamespaces do processo PID %d:\n", pid);

    // percorre os arquivos dentro da pasta ns
    while ((entrada = readdir(diretorio)) != NULL) {

        // ignora . e ..
        if (entrada->d_name[0] == '.') continue;

        // monta caminho do link simbólico
        snprintf(caminho_link, sizeof(caminho_link), "%s/%s", caminho_diretorio, entrada->d_name);

        char destino[TAM_BUFFER] = {0};

        // lê para onde o link aponta
        ssize_t n = readlink(caminho_link, destino, sizeof(destino) - 1);
        if (n != -1) {
            destino[n] = '\0';
            printf("  %s → %s\n", entrada->d_name, destino);  // exibe inode
        }
    }

    closedir(diretorio);
}


// =============================================
// LISTAR PROCESSOS QUE COMPARTILHAM UM NAMESPACE
// =============================================
void encontrar_processos_no_namespace(const char *tipo_ns, const char *inode_ns) {
    DIR *diretorio = opendir("/proc");
    if (!diretorio) {
        perror("Erro ao abrir /proc");
        return;
    }

    struct dirent *entrada;
    printf("\nProcessos no namespace %s (inode %s):\n", tipo_ns, inode_ns);

    // percorre todos os diretórios numéricos de /proc
    while ((entrada = readdir(diretorio)) != NULL) {

        // só entra se for número (PID)
        if (!isdigit(entrada->d_name[0])) continue;

        char caminho_ns[TAM_BUFFER];
        snprintf(caminho_ns, sizeof(caminho_ns), "/proc/%s/ns/%s", entrada->d_name, tipo_ns);

        char destino[TAM_BUFFER] = {0};

        // lê o inode do processo atual
        ssize_t n = readlink(caminho_ns, destino, sizeof(destino) - 1);
        if (n != -1) {
            destino[n] = '\0';

            // verifica se o inode bate
            if (strstr(destino, inode_ns)) {
                printf("  PID %s\n", entrada->d_name);
            }
        }
    }

    closedir(diretorio);
}


// =============================
// COMPARAR NAMESPACES ENTRE DOIS PROCESSOS
// =============================
void comparar_namespaces(int pid1, int pid2) {
    const char *tipos_ns[] = {"mnt", "uts", "pid", "net", "ipc", "user", "cgroup"};

    char caminho1[TAM_BUFFER], caminho2[TAM_BUFFER];
    char destino1[TAM_BUFFER], destino2[TAM_BUFFER];

    printf("\nComparando namespaces de PID %d e PID %d:\n", pid1, pid2);

    for (int i = 0; i < 7; i++) {

        // monta caminho para /proc/PID1/ns/tipo
        snprintf(caminho1, sizeof(caminho1), "/proc/%d/ns/%s", pid1, tipos_ns[i]);

        // monta caminho para /proc/PID2/ns/tipo
        snprintf(caminho2, sizeof(caminho2), "/proc/%d/ns/%s", pid2, tipos_ns[i]);

        // lê inode dos dois processos
        ssize_t n1 = readlink(caminho1, destino1, sizeof(destino1) - 1);
        ssize_t n2 = readlink(caminho2, destino2, sizeof(destino2) - 1);

        if (n1 == -1 || n2 == -1) {
            printf("  %s: erro ao ler namespace\n", tipos_ns[i]);
            continue;
        }

        destino1[n1] = '\0';
        destino2[n2] = '\0';

        // compara inodes
        if (strcmp(destino1, destino2) == 0)
            printf("  %s: MESMO namespace\n", tipos_ns[i]);
        else
            printf("  %s: DIFERENTE namespace\n", tipos_ns[i]);
    }
}


// =============================
// GERAR RELATÓRIO EM JSON
// =============================
void gerar_relatorio_namespaces(const char *nome_arquivo) {
    char arquivo_final[TAM_BUFFER];

    // copia nome + garante espaço
    strncpy(arquivo_final, nome_arquivo, sizeof(arquivo_final) - 1);
    arquivo_final[sizeof(arquivo_final) - 1] = '\0';

    // adiciona ".json" caso não tenha
    if (!strstr(arquivo_final, ".json")) {
        strncat(arquivo_final, ".json", sizeof(arquivo_final) - strlen(arquivo_final) - 1);
    }

    // cria arquivo
    FILE *arquivo = fopen(arquivo_final, "w");
    if (!arquivo) {
        perror("Erro ao criar arquivo de relatório");
        return;
    }

    // abre /proc
    DIR *diretorio = opendir("/proc");
    if (!diretorio) {
        perror("Erro ao abrir /proc");
        fclose(arquivo);
        return;
    }

    struct dirent *entrada;
    const char *tipos_ns[] = {"mnt", "uts", "pid", "net", "ipc", "user", "cgroup"};

    fprintf(arquivo, "{\n");
    fprintf(arquivo, "  \"namespace_report\": [\n");

    int primeiro = 1;

    // percorre todos os PIDs
    while ((entrada = readdir(diretorio)) != NULL) {

        // ignora não-PIDs
        if (!isdigit(entrada->d_name[0])) continue;

        if (!primeiro) fprintf(arquivo, ",\n");
        primeiro = 0;

        fprintf(arquivo, "    {\n");
        fprintf(arquivo, "      \"pid\": %s,\n", entrada->d_name);
        fprintf(arquivo, "      \"namespaces\": {\n");

        char caminho_ns[TAM_BUFFER], destino[TAM_BUFFER];

        // adiciona cada tipo de namespace ao JSON
        for (int i = 0; i < 7; i++) {
            snprintf(caminho_ns, sizeof(caminho_ns), "/proc/%s/ns/%s", entrada->d_name, tipos_ns[i]);

            ssize_t n = readlink(caminho_ns, destino, sizeof(destino) - 1);

            if (n != -1) {
                destino[n] = '\0';
                fprintf(arquivo, "        \"%s\": \"%s\"", tipos_ns[i], destino);
            } else {
                fprintf(arquivo, "        \"%s\": \"N/A\"", tipos_ns[i]);
            }

            if (i < 6) fprintf(arquivo, ",\n");
        }

        fprintf(arquivo, "\n      }\n");
        fprintf(arquivo, "    }");
    }

    fprintf(arquivo, "\n  ]\n");
    fprintf(arquivo, "}\n");

    closedir(diretorio);
    fclose(arquivo);

    printf("Relatório de namespaces gerado em '%s'\n", arquivo_final);
}


// =============================
// MEDIR OVERHEAD DE CRIAR NAMESPACE
// =============================
double medir_overhead_namespace() {
    struct timespec inicio, fim;

    // marca tempo inicial
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    // cria processo
    pid_t pid = fork();
    if (pid < 0) {
        perror("Erro ao criar processo filho");
        return -1;
    }

    if (pid == 0) {
        // FILHO → cria namespaces isolados
        if (unshare(CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS |
                    CLONE_NEWIPC | CLONE_NEWNET | CLONE_NEWCGROUP) < 0) {
            perror("Erro ao criar namespaces (unshare)");
            exit(1);
        }
        exit(0);
    }

    // pai espera o filho
    waitpid(pid, NULL, 0);

    // marca tempo final
    clock_gettime(CLOCK_MONOTONIC, &fim);

    // calcula tempo em microssegundos
    double tempo_us =
        (fim.tv_sec - inicio.tv_sec) * 1e6 +
        (fim.tv_nsec - inicio.tv_nsec) / 1e3;

    printf("%.3f µs\n", tempo_us);
    return tempo_us;
}
