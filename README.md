# Resource Monitor

Este projeto implementa um sistema de **monitoramento e análise de recursos** no Linux, permitindo observar, limitar e documentar o uso de CPU, memória, I/O e rede por processos e containers. Ele explora mecanismos do kernel como **namespaces** e **control groups (cgroups)**, sendo uma ferramenta prática para experimentação e análise de overhead de isolamento e limitação de recursos.

## Funcionalidades

* **Resource Profiler:** Coleta métricas detalhadas de CPU, memória e I/O de processos, calculando percentuais de uso e taxas de acesso.
* **Namespace Analyzer:** Analisa e compara namespaces de processos, medindo o overhead de criação e o nível de isolamento.
* **Control Group Manager:** Cria e gerencia cgroups, aplica limites de CPU e memória, e gera relatórios de utilização.
* **Exportação de Dados:** Gera arquivos JSON e relatórios simples na pasta `docs`.
* **Scripts de Visualização (Python):** Gera gráficos e relatórios de métricas de performance.

## Pré-requisitos

* **Sistema:** Ubuntu 24.04+ ou outra distribuição Linux com suporte a namespaces e cgroups
* **Compilador:** `gcc` ou `g++` (C/C++23)
* **Python (opcional, para scripts de visualização):** Python 3.x, bibliotecas `matplotlib` e `pandas`

## Como Rodar o Projeto

### 1. Preparar cgroups para Métricas de I/O

Antes de rodar o programa, habilite métricas de I/O:

```bash
sudo sh -c 'echo +io > /sys/fs/cgroup/cgroup.subtree_control'
```

Para conferir se foi ativado:

```bash
cat /sys/fs/cgroup/cgroup.subtree_control
```

### 2. Compilar o Sistema

O projeto possui um Makefile configurado:

```bash
# Compila todos os componentes
make all

# Limpa binários e objetos
make clean
```

### 3. Executar o Programa Principal

Para rodar com permissões elevadas (necessário para cgroups):

```bash
sudo make run
```

ou diretamente:

```bash
sudo ./bin/monitor
```

### 4. Executar Testes

Testes individuais:

```bash
make cpu-test       # Teste de CPU
make memory-test    # Teste de memória
sudo make io-test   # Teste de I/O (precisa de sudo)
```

**Teste automático das três métricas (CPU, memória e I/O):**

O arquivo `all_tests.c` na pasta `tests/` permite rodar todos os testes de uma vez:

```bash
sudo make all-tests
```

### 5. Rodar Scripts Python de Visualização

**Criar e ativar ambiente virtual:**

```bash
python3 -m venv venv
source venv/bin/activate  # Linux / WSL
```

**Instalar dependências:**

```bash
pip install matplotlib pandas
```

**Executar scripts do projeto:**

```bash
python3 scripts/visualize.py
python3 scripts/compare_tools.py
```

**Desativar o ambiente virtual quando terminar:**

```bash
deactivate
```

💡 **Dica:** Sempre ative o venv antes de rodar scripts Python.

---

## Como Visualizar o Relatório HTML

O projeto possui páginas HTML geradas automaticamente na pasta `docs`. Para visualizar:

1. Entre na pasta `docs`:

```bash
cd docs
```

2. Inicie um servidor HTTP local:

```bash
python3 -m http.server
```

3. Abra no navegador:

```
http://localhost:8000/teste.html
```

---

## Testes e Experimentos

O projeto inclui experimentos para validar monitoramento, isolamento e limitação de recursos. Eles estão documentados na pasta `docs` no arquivo **`EXPERIMENTOS.md`**.

Os experimentos não são integrados ao código principal. Eles são executados separadamente, rodados diretamente pelo terminal, apenas para coleta e análise dos resultados.

### Teste do Valgrind

Também foi realizado o teste de vazamento de memória com **Valgrind**. Os resultados estão na pasta `docs`, no arquivo **`valgrind.md`**.

---

## Contribuição dos Alunos

### Aluno 1 e 4 – Resource Profiler + Integração / Control Group Manager + Análise

**Mateus Marochi Andretta de Castro**
Contribuiu implementando os módulos de CPU e memória (`cpu_monitor.c`, `memory_monitor.c`), cálculos de percentuais e taxas, integração dos três componentes no `main.c`, definição das estruturas e funções em `monitor.h`, criação do Makefile geral para compilação do projeto, implementação do Cgroup Manager (`cgroup_manager.c`, `cgroup.h`), scripts de visualização (`visualize.py`), ferramentas de comparação (`compare_tools.py`) e geração de relatórios em HTML (`teste.html`).

### Aluno 2 – Resource Profiler + Testes

**Matheus Mazzucco dos Santos**
Contribuiu implementando os módulos de I/O e Network (`io_monitor.c`, `network_monitor.c`) e desenvolvendo os programas de teste na pasta `tests/`, que incluem testes de CPU, memória, I/O e automação de todos os testes para validar a precisão das medições, realizou também o teste de vazemento de memória pelo método **Valgrind**

### Aluno 3 – Namespace Analyzer

**Anderson Cesar de Azevedo de Souza Filho**
Contribuiu implementando a análise de namespaces (`namespace_analyser.c`, `namespace.h`), incluindo identificação e comparação de namespaces entre processos, e documentando a arquitetura do sistema no arquivo `ARCHITECTURE.md`, realizou também a documentação dos experimentos.
