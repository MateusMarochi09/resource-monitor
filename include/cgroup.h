#ifndef CGROUP_H
#define CGROUP_H

int cgroup_existe(const char *nome);                   // Verifica se o cgroup existe
int cgroup_criar(const char *nome);                    // Cria um cgroup
int cgroup_mover_processo(const char *nome, int pid);  // Move um processo para o cgroup

// Aplica limites de CPU e memória
int cgroup_aplicar_limites(const char *nome, int limite_cpu_usec, long limite_mem_bytes);

int cgroup_ler_e_mostrar_estatisticas(const char *nome); // Exibe estatísticas do cgroup
int cgroup_gerar_relatorio(const char *nome_cgroup);     // Gera relatório do cgroup

#endif