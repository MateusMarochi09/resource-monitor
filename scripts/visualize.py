import os            # manipulação de arquivos e pastas
import json          # leitura dos relatórios JSON
import matplotlib.pyplot as plt   # criação dos gráficos

# Pastas onde ficam os JSONs e onde os gráficos serão salvos
PASTA_DOCS = "docs"
PASTA_PLOTS = os.path.join(PASTA_DOCS, "plots")

# Cria pasta de plots se não existir
os.makedirs(PASTA_PLOTS, exist_ok=True)

def ler_json(caminho_arquivo):
    """Lê um arquivo JSON e retorna o conteúdo como dicionário."""
    with open(caminho_arquivo, "r") as f:
        return json.load(f)

def gerar_graficos_snapshot(dados, nome_arquivo):
    """Gera gráficos de CPU, memória, IO e rede do snapshot mais recente."""
    d = dados[0]  # usa o primeiro snapshot do JSON

    # ---------------------- CPU ----------------------
    labels_cpu = ["CPU (%)", "Utime", "Stime", "Threads", "Ctxt Vol", "Ctxt Invol", "Syscall"]
    valores_cpu = [
        d["cpu"], d["utime"], d["stime"], d["threads"],
        d["ctxt_voluntario"], d["ctxt_involuntario"], d["syscall"]
    ]

    plt.figure(figsize=(10, 6))
    plt.bar(labels_cpu, valores_cpu, color='skyblue')
    plt.title("CPU")           # título do gráfico
    plt.ylabel("Percentual / Quantidade")
    plt.grid(True, axis='y')   # grade no eixo Y
    plt.tight_layout()         # ajusta layout
    plt.savefig(os.path.join(PASTA_PLOTS, f"{nome_arquivo}_cpu.png"))
    plt.close()

    # ---------------------- MEMÓRIA ----------------------
    labels_mem = ["RSS (KB)", "VSZ (KB)", "Swap (KB)", "MinFLT", "MajFLT"]
    valores_mem = [
        d["rss_kb"], d["vsz_kb"], d["swap_kb"], d["minflt"], d["majflt"]
    ]

    plt.figure(figsize=(10, 6))
    plt.bar(labels_mem, valores_mem, color='lightgreen')
    plt.title("Memória")
    plt.ylabel("Valores (KB / Faltas de Página)")
    plt.grid(True, axis='y')
    plt.tight_layout()
    plt.savefig(os.path.join(PASTA_PLOTS, f"{nome_arquivo}_memoria.png"))
    plt.close()

    # ---------------------- IO / DISCO ----------------------
    labels_io = ["Bytes Lidos", "Bytes Escritos", "Sys Read", "Sys Write", "Disk Ops"]
    valores_io = [
        d["bytes_lidos"], d["bytes_escritos"],
        d["sys_leitura"], d["sys_escrita"], d["disk_ops"]
    ]

    plt.figure(figsize=(10, 6))
    plt.bar(labels_io, valores_io, color='orange')
    plt.title("IO - Disco")
    plt.ylabel("Valores (Bytes / Ops)")
    plt.grid(True, axis='y')
    plt.tight_layout()
    plt.savefig(os.path.join(PASTA_PLOTS, f"{nome_arquivo}_io.png"))
    plt.close()

    # ---------------------- REDE ----------------------
    labels_net = ["Bytes RX", "Bytes TX", "Packets RX", "Packets TX", "Conexões Ativas"]
    valores_net = [
        d["bytes_rx"], d["bytes_tx"],
        d["packets_rx"], d["packets_tx"], d["conexoes_ativas"]
    ]

    plt.figure(figsize=(10, 6))
    plt.bar(labels_net, valores_net, color='violet')
    plt.title("Rede")
    plt.ylabel("Valores (Bytes / Pacotes / Conexões)")
    plt.grid(True, axis='y')
    plt.tight_layout()
    plt.savefig(os.path.join(PASTA_PLOTS, f"{nome_arquivo}_rede.png"))
    plt.close()

    print("Todos os gráficos foram gerados com sucesso!")

# ---------------------- MAIN ----------------------
if __name__ == "__main__":
    # pega todos os arquivos JSON do tipo dados_*.json
    arquivos = sorted([f for f in os.listdir(PASTA_DOCS)
                       if f.startswith("dados_") and f.endswith(".json")])

    if not arquivos:
        print("Nenhum arquivo dados_*.json encontrado na pasta docs")
        exit(1)

    # lê o JSON mais recente
    arquivo = os.path.join(PASTA_DOCS, arquivos[-1])
    dados = ler_json(arquivo)

    # nome base para salvar os gráficos
    nome_base = os.path.splitext(arquivos[-1])[0]

    # gera os gráficos
    gerar_graficos_snapshot(dados, nome_base)
