#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> 
#include "monitor.h"
#include "namespace.h"
#include "cgroup.h"
#include <unistd.h>
#include <sys/stat.h>


// Função para verificar se um processo existe
int processo_existe(int pid) {
    char caminho[64];
    sprintf(caminho, "/proc/%d", pid);
    return access(caminho, F_OK) == 0; // retorna 1 se o diretório do processo existe
}

// -------------------- MENU RESOURCE PROFILER --------------------
// Responsável por medir CPU, Memória, I/O, Rede e monitoramento contínuo
void menu_resource_profiler() {
    int opcao;


    while (1) {
        printf("\n========== RESOURCE PROFILE ==========\n");
        printf("1. Monitorar CPU\n");
        printf("2. Monitorar Memória\n");
        printf("3. Monitorar Entrada/Saída (I/O)\n");
        printf("4. Monitorar Rede\n"); 
        printf("5. Monitorar todos os recursos\n");
        printf("0. Voltar ao menu principal\n");
        printf("-----------------------------------------\n");
        printf("Escolha uma opção: ");

        // Garante entrada numérica válida
        if (scanf("%d", &opcao) != 1) {

            printf("Entrada inválida! Digite apenas números.\n");
            while (getchar() != '\n');
            continue;
        }

        int pid, intervalo;
        MetricasProcesso dados;

        switch (opcao) {
            case 1:
                printf("Digite o PID do processo: ");
                scanf("%d", &pid);
                if (!processo_existe(pid)) {
                    printf("Erro: processo %d não encontrado!\n", pid);
                    break;

                }

                obter_uso_cpu(pid, &dados);

                printf("PID %d → CPU: %.2f%% | User time: %.2fs | System time: %.2fs | Threads: %d | CtxSwitch: %lu\n",
                    pid, dados.cpu_porcentagem,
                    (double)dados.tempo_usuario / sysconf(_SC_CLK_TCK),
                    (double)dados.tempo_sistema / sysconf(_SC_CLK_TCK),
                    dados.threads, dados.trocas_ctx_voluntarias + dados.trocas_ctx_involuntarias);
                break;

            case 2:
                printf("Digite o PID do processo: ");
                scanf("%d", &pid);
                obter_uso_memoria(pid, &dados);
                printf("PID %d → Memória Física (RSS): %.2f MB | Memória Virtual (VSZ): %.2f MB | Swap: %.2f MB | PageFaults: %lu\n",

                       pid,
                       dados.memoria_residente / 1024.0,
                       dados.memoria_virtual / 1024.0,
                       dados.memoria_swap / 1024.0,
                       dados.page_faults_minflt + dados.page_faults_majflt);
                break;
           case 3:
                printf("Digite o PID do processo: ");
                scanf("%d", &pid);
                if (!processo_existe(pid)) {
                    printf("Erro: processo %d não encontrado!\n", pid);
                break;

                }

            obter_uso_io(pid, &dados);

            printf("PID %d → Syscalls: %ld (R:%ld / W:%ld) | Leitura: %.2f MB | Escrita: %.2f MB | DiskOps: %ld\n",
                pid, dados.syscall, dados.syscr, dados.syscw,
                dados.bytes_lidos / (1024.0 * 1024.0),
                dados.bytes_escritos / (1024.0 * 1024.0),
                dados.disk_ops
);
            break;
            case 4:
            printf("Digite o PID do processo: ");
            scanf("%d", &pid);
            if (!processo_existe(pid)) {
                printf("Erro: processo %d não encontrado!\n", pid);
                break;
            }

            obter_uso_rede(pid, &dados);

            printf("PID %d → NET: RX: %.2f MB | TX: %.2f MB | Pkts RX: %ld | Pkts TX: %ld | Conexões: %d\n",
            pid,
            dados.bytes_rx / (1024.0 * 1024.0),
            dados.bytes_tx / (1024.0 * 1024.0),
            dados.packets_rx, dados.packets_tx,
            dados.conexoes_ativas);
    break;


           case 5:
    printf("Digite o PID do processo: ");
    scanf("%d", &pid);
    printf("Intervalo de atualização (s): ");
    scanf("%d", &intervalo);

    // limpa o \n que ficou no buffer
    getchar();

    printf("\nMonitorando todos os recursos...\n");
    printf("Pressione ENTER para parar e voltar ao menu.\n");

    while (1) {

        // Verifica se o usuário apertou ENTER (sem bloquear)
        fd_set fds;
        struct timeval tv = {0, 0};

        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        // select() = verifica se tem entrada do teclado
        if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
            char c = getchar();
            if (c == '\n') {
                printf("\nMonitoramento interrompido pelo usuário.\n");
                break;
            }
        }

        dados.pid = pid;       
        obter_uso_cpu(pid, &dados);
        obter_uso_memoria(pid, &dados);
        obter_uso_io(pid, &dados);
        obter_uso_rede(pid, &dados);

        printf("\n=============================================\n");
        printf("CPU: %.2f%% | Threads: %d\n", dados.cpu_porcentagem, dados.threads);
        printf("MEM: RSS %.2f MB | VSZ %.2f MB | SWAP %.2f MB\n",
            dados.memoria_residente/1024.0, dados.memoria_virtual/1024.0, dados.memoria_swap/1024.0);

        printf("I/O: Leitura %.2f MB | Escrita %.2f MB\n",
            dados.bytes_lidos/(1024.0*1024.0), dados.bytes_escritos/(1024.0*1024.0));
        printf("REDE: RX %.2f MB | TX %.2f MB | Conexões %d\n",
            dados.bytes_rx/(1024.0*1024.0), dados.bytes_tx/(1024.0*1024.0), dados.conexoes_ativas);

        exportar_para_json(dados);

        sleep(intervalo);
    }

    break;


            case 0:
                return;

            default:
                printf("Opção inválida!\n");
        }
    }
}
void menu_namespace_analyser() {
    int opcao;

    while (1) {

        printf("\n========== NAMESPACE ANALYSER ==========\n");
        printf("1. Listar namespaces de um processo\n");
        printf("2. Encontrar processos em um namespace\n");
        printf("3. Comparar namespaces entre dois processos\n");
        printf("4. Gerar relatório de namespaces do sistema\n");
        printf("5. Medir overhead de criação de namespaces\n");
        printf("0. Voltar ao menu principal\n");
        printf("----------------------------------------------\n");
        printf("Escolha uma opção: ");

        if (scanf("%d", &opcao) != 1) {
            printf("Entrada inválida! Digite apenas números.\n");
            while (getchar() != '\n');

            continue;
        }

        int pid1, pid2;
        char tipo_ns[10], inode_ns[50];

        switch (opcao) {
            case 1:
                printf("Digite o PID do processo: ");
                scanf("%d", &pid1);
                if (!processo_existe(pid1)) {
                    printf("Erro: processo %d não encontrado!\n", pid1);
                    break;
                }
                listar_namespaces(pid1);
                break;

            case 2:
                printf("Digite o tipo do namespace (mnt, pid, net, ipc, uts, user, cgroup): ");
                scanf("%s", tipo_ns);
                printf("Digite o inode do namespace: ");
                scanf("%s", inode_ns);
                encontrar_processos_no_namespace(tipo_ns, inode_ns);
                break;

            case 3:
                printf("Digite PID 1: "); scanf("%d", &pid1);
                printf("Digite PID 2: "); scanf("%d", &pid2);
                if (!processo_existe(pid1) || !processo_existe(pid2)) {
                    printf("Erro: um dos PIDs não existe!\n");
                    break;
                }
                comparar_namespaces(pid1, pid2);
                break;

            case 4:
                gerar_relatorio_namespaces("docs/relatorio_namespace");
                break;

            case 5:
                medir_overhead_namespace();
                break;

            case 0:
                return;

            default:
                printf("Opção inválida!\n");
        }
    }
}



// função do menu do cgroup (colocar em src ou diretamente no main.c)
void menu_cgroup_manager() {
    int opcao;
    char nome[64];
    int pid;

    int lim_cpu;
    long lim_mem;

    while (1) {
        printf("\n========== CONTROL GROUP MANAGER ==========\n");
        printf("1. Criar cgroup\n");
        printf("2. Mover processo para cgroup\n");
        printf("3. Aplicar limites (CPU e Memória)\n"); 
        printf("4. Mostrar estatísticas do cgroup\n");
        printf("5. Gerar relatório\n");
        printf("0. Voltar ao menu principal\n");
        printf("-------------------------------------------\n");
        printf("Escolha uma opção: ");

        if (scanf("%d", &opcao) != 1) {
            printf("Entrada inválida! Digite apenas números.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (opcao) {
            case 1:
                printf("Nome do cgroup: ");
                scanf("%s", nome);
                cgroup_criar(nome);
                break;

            case 2:
                printf("Nome do cgroup: ");
                scanf("%s", nome);
                printf("PID do processo: ");
                scanf("%d", &pid);
                cgroup_mover_processo(nome, pid);
                break;

            case 3:
                printf("Nome do cgroup: ");
                scanf("%s", nome);

                printf("Limite de CPU (µs por 100ms): ");
                scanf("%d", &lim_cpu);

                printf("Limite de Memória (bytes): ");

                scanf("%ld", &lim_mem);

                cgroup_aplicar_limites(nome, lim_cpu, lim_mem);
                break;

            case 4:
                printf("Nome do cgroup: ");
                scanf("%s", nome);
                cgroup_ler_e_mostrar_estatisticas(nome);
                break;

            case 5:
            printf("Nome do cgroup: ");
            scanf("%s", nome);
            cgroup_gerar_relatorio(nome);
            break;

            case 0:
                return;

            default:
                printf("Opção inválida!\n");
        }
    }
}


// ---------- MENU PRINCIPAL ----------
void menu_principal() {
    int opcao;

    while (1) {
        printf("\n====================================\n");
        printf("      SISTEMA DE MONITORAMENTO\n");
        printf("====================================\n");
        printf("1. Recource Profile\n");
        printf("2. Namespace Analyser\n");
        printf("3. Control Group Manager\n");
        printf("0. Sair\n");
        printf("------------------------------------\n");
        printf("Escolha uma opção: ");

        if (scanf("%d", &opcao) != 1) {
            printf("Entrada inválida! Digite apenas números.\n");
            while (getchar() != '\n'); // limpa buffer
            continue;
        }

        switch (opcao) {
            case 1:
                menu_resource_profiler();
                break;

            case 2:
                menu_namespace_analyser();
                break;

            case 3:
                menu_cgroup_manager();
                break;


            case 0:
                printf("\nEncerrando o programa...\n");
                exit(0);

            default:
                printf("\nOpção inválida! Tente novamente.\n");
        }
    }
}

int proximo_indice_arquivo(const char *prefixo) {
    int maior = 0;
    DIR *d = opendir("docs");
    if (!d) return 1;

    struct dirent *entry;
    while ((entry = readdir(d))) {
        int n;
        if (sscanf(entry->d_name, "%*[^_]_%d.json", &n) == 1) {
            if (n > maior) maior = n;
        }
    }
    closedir(d);
    return maior + 1;
}

void exportar_para_json(MetricasProcesso dados) {
    char caminho[256];

    // Garante que a pasta docs exista
    struct stat st;
    if (stat("docs", &st) == -1) mkdir("docs", 0700);

    // Gera nome automaticamente
    int idx = proximo_indice_arquivo("dados");
    snprintf(caminho, sizeof(caminho), "docs/dados_%d.json", idx);


    FILE *arquivo = fopen(caminho, "w");
    if (!arquivo) {
        perror("Erro ao abrir JSON");
        return;
    }

    // Início do array
    fprintf(arquivo, "[\n");

    // Escreve o primeiro e único item
    fprintf(arquivo,
        "  { \"pid\": %d, "
        "\"cpu\": %.2f, "
        "\"utime\": %lu, \"stime\": %lu, "
        "\"threads\": %d, "
        "\"ctxt_voluntario\": %lu, \"ctxt_involuntario\": %lu, "
        "\"rss_kb\": %ld, \"vsz_kb\": %ld, \"swap_kb\": %ld, "
        "\"minflt\": %lu, \"majflt\": %lu, "
        "\"bytes_lidos\": %ld, \"bytes_escritos\": %ld, "
        "\"sys_leitura\": %ld, \"sys_escrita\": %ld, "
        "\"syscall\": %ld, \"disk_ops\": %ld, "
        "\"bytes_rx\": %ld, \"bytes_tx\": %ld, "
        "\"packets_rx\": %ld, \"packets_tx\": %ld, "
        "\"conexoes_ativas\": %d }",
        dados.pid, dados.cpu_porcentagem,
        dados.tempo_usuario, dados.tempo_sistema,
        dados.threads,
        dados.trocas_ctx_voluntarias, dados.trocas_ctx_involuntarias,
        dados.memoria_residente, dados.memoria_virtual, dados.memoria_swap,
        dados.page_faults_minflt, dados.page_faults_majflt,
        dados.bytes_lidos, dados.bytes_escritos,
        dados.syscr, dados.syscw, dados.syscall, dados.disk_ops,
        dados.bytes_rx, dados.bytes_tx,
        dados.packets_rx, dados.packets_tx,
        dados.conexoes_ativas
    );

    // Fecha JSON
    fprintf(arquivo, "\n]");
    fclose(arquivo);
}

int main() {
    menu_principal();
    return 0;
}
