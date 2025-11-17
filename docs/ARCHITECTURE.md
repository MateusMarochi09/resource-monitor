# Arquitetura do Projeto de Monitoramento de Processos

## 1. Visão Geral

O projeto consiste em um **sistema de monitoramento de processos** no Linux, capaz de:

- Obter métricas de CPU, memória, I/O e rede de processos individuais.
- Gerenciar **namespaces** do Linux e gerar relatórios.
- Criar e controlar **control groups (cgroups)**, aplicando limites de CPU e memória e gerando estatísticas.
- Exportar métricas para arquivos JSON.
- Possuir uma arquitetura modular, facilitando a adição de novos recursos e testes independentes.

O sistema é escrito em **C**, utiliza a interface `/proc` do Linux para coletar métricas e organiza código, headers, testes e binários de forma modular.

---

## 2. Estrutura de Pastas

```
resource-monitor/
├── README.md
├── Makefile
├── docs/
│   ├── ARCHITECTURE.md
│   └── arquivos_de_relatorios
├── include/
│   ├── monitor.h
│   ├── namespaces.h
│   └── cgroup.h
├── src/
│   ├── cpu_monitor.c
│   ├── memory_monitor.c
│   ├── io_monitor.h
│   ├── network_monitor.c
│   ├── namespace_analyzer.c
│   ├── cgroup_manager.c
│   └── main.c
├── tests/
│   ├── test_cpu.c
│   ├── test_io.c
│   └──  test_memory.c
├── scripts/
│   ├── visualize.py
│   └── compare_tools.py
```

---

## 3. Componentes do Projeto

### 3.1 Módulo de Monitoramento (`monitor.h` / `monitor.c`)

- **Funções principais**:
  - `obter_uso_cpu(pid, MetricasProcesso*)`
  - `obter_uso_memoria(pid, MetricasProcesso*)`
  - `obter_uso_io(pid, MetricasProcesso*)`
  - `obter_uso_rede(pid, MetricasProcesso*)`
  - `exportar_para_json(MetricasProcesso, const char*)`
- **Estrutura de dados**: `MetricasProcesso` armazena todas as métricas de CPU, memória, I/O e rede.
- **Uso**: Integrado ao menu principal e aos testes unitários.

### 3.2 Módulo Namespace (`namespace.h` / `namespace.c`)

- **Funcionalidades**:
  - Listar namespaces de um processo (`listar_namespaces`)
  - Encontrar processos que compartilham um namespace (`encontrar_processos_no_namespace`)
  - Comparar namespaces entre dois processos (`comparar_namespaces`)
  - Gerar relatório completo de namespaces (`gerar_relatorio_namespaces`)
  - Medir overhead de criação de namespaces (`medir_overhead_namespace`)
- **Objetivo**: Auditoria e análise de isolamento de processos via namespaces.

### 3.3 Módulo Control Group (`cgroup.h` / `cgroup_manager.c`)

- **Funcionalidades**:
  - Criar/verificar cgroups (`cgroup_criar`, `cgroup_existe`)
  - Mover processos para cgroups (`cgroup_mover_processo`)
  - Aplicar limites de CPU/memória (`cgroup_aplicar_limites`)
  - Ler estatísticas e gerar relatórios (`cgroup_ler_e_mostrar_estatisticas`, `cgroup_gerar_relatorio`)
- **Objetivo**: Controle e limitação de recursos de processos.

### 3.4 Menu Principal (`main.c`)

- Menu hierárquico:
  1. Resource Profile (CPU, memória, I/O, rede)
  2. Namespace Analyser
  3. Control Group Manager
  4. Sair
- Chama funções específicas dos módulos garantindo modularidade e expansibilidade.

### 3.5 Testes Unitários (`tests/`)

- Cada teste compilado em **binário separado**.
- Valida métricas de CPU, memória e I/O sem executar o sistema completo.

### 3.6 Makefile

- Automatiza:
  - Compilação do projeto (`all`, `run`)
  - Compilação/execução de testes (`cpu-test`, `memory-test`, `io-test`, `run-tests`)
  - Limpeza (`clean`)
- Cria objetos separados para `src` e `tests` e binários na pasta `bin`.

### 3.7 Scripts de Visualização

- Gerar gráficos e relatórios visuais a partir de arquivos JSON (`docs/dados_*.json`).

---

## 4. Fluxo de Execução

1. **Menu principal (`main.c`)** → Escolha de módulo.
2. **Módulo específico** → Coleta métricas/processa dados.
3. **Exportação de métricas** → JSON em `docs/`.
4. **(Opcional)** → Visualização gráfica via scripts Python.
