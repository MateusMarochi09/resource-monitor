#ifndef NAMESPACE_H
#define NAMESPACE_H

// Lista todos os namespaces associados a um processo
void listar_namespaces(int pid);

// Encontra processos que compartilham um namespace específico
void encontrar_processos_no_namespace(const char *ns_type, const char *ns_inode);

// Compara namespaces entre dois processos
void comparar_namespaces(int pid1, int pid2);

// Gera um relatório completo de namespaces do sistema
void gerar_relatorio_namespaces(const char *nome_arquivo);

// Mede o overhead de criação de novos namespaces (OBRIGATÓRIO)
double medir_overhead_namespace();

#endif
