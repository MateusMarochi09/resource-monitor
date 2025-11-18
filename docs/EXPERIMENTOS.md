# Experimentos do Projeto de Monitoramento de Processos

Este documento registra os experimentos realizados com o sistema de monitoramento, incluindo comandos utilizados e resultados simulados.

---

## Experimento 1: Overhead de Monitoramento

**Objetivo:** Medir o impacto do próprio profiler no sistema.

**Procedimento e comandos:**

1. Executar workload de referência sem monitoramento:

```bash
time ./bin/workload_reference
```

2. Executar o mesmo workload com monitoramento em intervalos diferentes (1s, 0.5s):

```bash
time ./bin/main --profile workload_reference --interval 1
time ./bin/main --profile workload_reference --interval 0.5
```

**Resultados simulados:**

| Intervalo         | Tempo de execução (s) | CPU overhead (%) | Latência de sampling (ms) |
| ----------------- | --------------------- | ---------------- | ------------------------- |
| Sem monitoramento | 10.12                 | 0                | -                         |
| 1 s               | 10.75                 | 6.3              | 50                        |
| 0.5 s             | 11.05                 | 9.2              | 25                        |

**Observação:** O overhead aumenta conforme a frequência de sampling diminui o intervalo.

---

## Experimento 2: Isolamento via Namespaces

**Objetivo:** Validar efetividade do isolamento via namespaces.

**Procedimento e comandos:**

1. Criar processos com diferentes namespaces:

```bash
sudo unshare -p -n --fork bash
sudo unshare -u -m -i --fork bash
```

2. Verificar visibilidade de recursos:

```bash
ps aux | grep <pid>
ip addr
mount | grep <pid>
```

3. Medir tempo de criação de cada namespace:

```bash
time unshare -p bash
time unshare -n bash
time unshare -u bash
```

**Resultados simulados:**

| Tipo de Namespace | Overhead de Criação (µs) | Número de processos isolados | Observações                 |
| ----------------- | ------------------------ | ---------------------------- | --------------------------- |
| PID               | 150                      | 1                            | Isolamento efetivo          |
| Network           | 320                      | 1                            | Rede separada               |
| Mount             | 280                      | 1                            | Sistema de arquivos isolado |
| UTS               | 100                      | 1                            | Nome do host separado       |

---

## Experimento 3: Throttling de CPU via Cgroups

**Objetivo:** Avaliar precisão de limitação de CPU.

**Procedimento e comandos:**

1. Executar processo CPU-intensive sem limite:

```bash
./bin/cpu_stress
```

2. Criar cgroups e aplicar limites:

```bash
sudo cgcreate -g cpu:/cpu_limit
sudo cgset -r cpu.cfs_quota_us=25000 cpu_limit   # 0.25 cores
sudo cgexec -g cpu:/cpu_limit ./bin/cpu_stress
```

3. Repetir para 0.5, 1.0 e 2.0 cores.

**Resultados simulados:**

| Limite de CPU | CPU% medido | Desvio (%) | Throughput (it/s) |
| ------------- | ----------- | ---------- | ----------------- |
| 0.25 cores    | 24          | -4         | 120               |
| 0.5 cores     | 48          | -4         | 240               |
| 1.0 cores     | 98          | -2         | 480               |
| 2.0 cores     | 100         | 0          | 480               |

---

## Experimento 4: Limitação de Memória

**Objetivo:** Testar comportamento ao atingir limite de memória.

**Procedimento e comandos:**

1. Criar cgroup com limite de 100MB:

```bash
sudo cgcreate -g memory:/mem_limit
sudo cgset -r memory.limit_in_bytes=104857600 mem_limit
```

2. Executar processo que aloca memória incrementalmente:

```bash
sudo cgexec -g memory:/mem_limit ./bin/mem_stress
```

3. Observar comportamento:

```bash
cat /sys/fs/cgroup/memory/mem_limit/memory.failcnt
```

**Resultados simulados:**

| Memória Alocada Máxima | Falhas de Alocação | Observação                                         |
| ---------------------- | ------------------ | -------------------------------------------------- |
| 100 MB                 | 3                  | OOM killer não acionou; processo parou normalmente |

---

## Experimento 5: Limitação de I/O

**Objetivo:** Avaliar precisão de limitação de I/O.

**Procedimento e comandos:**

1. Criar cgroup para I/O:

```bash
sudo cgcreate -g blkio:/io_limit
sudo cgset -r blkio.throttle.read_bps_device="8:0 1048576" io_limit
sudo cgset -r blkio.throttle.write_bps_device="8:0 524288" io_limit
```

2. Executar workload I/O-intensive:

```bash
sudo cgexec -g blkio:/io_limit ./bin/io_stress
```

3. Medir throughput real:

```bash
iostat -dx 1
```

**Resultados simulados:**

| Limite Configurado | Throughput Medido | Latência I/O (ms) | Observação                   |
| ------------------ | ----------------- | ----------------- | ---------------------------- |
| Read: 1 MB/s       | 0.98 MB/s         | 12                | Limite aplicado corretamente |
| Write: 0.5 MB/s    | 0.52 MB/s         | 15                | Pequeno desvio, aceitável    |

---

### Observações Gerais

- Os experimentos seguem o **procedimento de replicação com cgroups e namespaces**.
- Resultados são consistentes com expectativas de overhead e limitações impostas.
