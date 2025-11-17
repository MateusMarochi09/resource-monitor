#include <stdio.h>
#include <stdlib.h>

void menu_resource_profiler() {
    int opcao;

    while (1) {
        printf("\n========== RESOURCE PROFILER ==========\n");
        printf("1. Monitorar CPU\n");
        printf("2. Monitorar Memória\n");
        printf("3. Monitorar I/O\n");
        printf("4. Monitorar todos os recursos\n");
        printf("0. Voltar ao menu principal\n");
        printf("---------------------------------------\n");
        printf("Escolha uma opção: ");

        if (scanf("%d", &opcao) != 1) {
            printf("Entrada inválida! Digite apenas números.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (opcao) {
            case 1:
                printf("Opção 1: Monitorar CPU (em breve)\n");
                break;
            case 2:
                printf("Opção 2: Monitorar Memória (em breve)\n");
                break;
            case 3:
                printf("Opção 3: Monitorar I/O (em breve)\n");
                break;
            case 4:
                printf("Opção 4: Monitorar todos os recursos (em breve)\n");
                break;
            case 0:
                return; // volta para o menu principal
            default:
                printf("Opção inválida!\n");
        }
    }
}

void menu_principal() {
    int opcao;

    while (1) {
        printf("\n====================================\n");
        printf("      SISTEMA DE MONITORAMENTO\n");
        printf("====================================\n");
        printf("1. Resource Profiler\n");
        printf("2. Namespace Analyzer\n");
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
                menu_resource_profiler(); // chama o submenu do Resource
                break;

            case 2:
                printf("\n[Namespace Analyzer ainda não implementado]\n");
                break;

            case 3:
                printf("\n[Control Group Manager ainda não implementado]\n");
                break;

            case 0:
                printf("\nEncerrando o programa...\n");
                exit(0);

            default:
                printf("\nOpção inválida! Tente novamente.\n");
        }
    }
}


int main() {
    menu_principal();
    return 0;
}


