#ifndef MONITOR_H
#define MONITOR_H

// Estrutura para armazenar as métricas do processo
typedef struct {
    int pid;                                      // ID do processo
    double cpu_porcentagem;                       // Uso de CPU em porcentagem
    unsigned long tempo_usuario, tempo_sistema;   // Tempos de CPU em modo usuário e sistema
    int threads;                                  // Número de threads
    unsigned long trocas_ctx_voluntarias;         // Trocas de contexto voluntárias
    unsigned long trocas_ctx_involuntarias;       // Trocas de contexto involuntárias
    unsigned long page_faults_minflt;           // Page faults leves (minflt)
    unsigned long page_faults_majflt;          // Page faults graves (majflt)
    long memoria_residente;                    // Memória residente (RSS)
    long memoria_virtual;                      // Memória virtual (VSZ)
    long memoria_swap;                         // Memória em swap
    long bytes_lidos;                             // Bytes lidos
    long bytes_escritos;                          // Bytes escritos
    long syscr;                                   // Chamadas de leitura ao sistema
    long syscw;                                   // Chamadas de escrita ao sistema
    long syscall;                                 // Total de chamadas de sistema
    long disk_ops;                         // Operações no disco
    long bytes_rx;                                // Bytes recebidos
    long bytes_tx;                                // Bytes enviados
    long packets_rx;                              // Pacotes recebidos
    long packets_tx;                              // Pacotes enviados
    int conexoes_ativas;                          // Número de conexões TCP ativas
} MetricasProcesso;

// Funções de monitoramento
int processo_existe(int pid);                          // Verifica se o processo existe
void obter_uso_cpu(int pid, MetricasProcesso *dados);  // Coleta uso de CPU
void obter_uso_memoria(int pid, MetricasProcesso *dados); // Coleta uso de memória
void obter_uso_io(int pid, MetricasProcesso *dados);   // Coleta métricas de I/O
void obter_uso_rede(int pid, MetricasProcesso *dados); // Coleta métricas de rede
void exportar_para_json(MetricasProcesso dados);       // Exporta dados para JSON

#endif
