import os     # usado para listar arquivos na pasta docs
import json   # leitura dos relatórios JSON

PASTA_DOCS = "docs"   # pasta onde ficam os dados coletados

# Lê um arquivo JSON e retorna como dicionário
def ler_json(caminho_arquivo):
    with open(caminho_arquivo, "r") as f:
        return json.load(f)

# Compara dois arquivos JSON mostrando CPU, memória e IO
def compare_tools(arquivo1, arquivo2):
    p1 = ler_json(arquivo1)
    p2 = ler_json(arquivo2)

    # Caso os relatórios venham como lista, usamos o primeiro snapshot
    if isinstance(p1, list):
        p1 = p1[0]
    if isinstance(p2, list):
        p2 = p2[0]

    print(f"\n===== COMPARAÇÃO ENTRE {arquivo1} E {arquivo2} =====\n")

    # CPU
    print("--- CPU ---")
    print(f"Exec1: {p1['cpu']}% | Exec2: {p2['cpu']}% | Diferença: {p2['cpu'] - p1['cpu']:.2f}%")

    # Memória
    print("\n--- MEMÓRIA (RSS KB) ---")
    print(f"Exec1: {p1['rss_kb']} | Exec2: {p2['rss_kb']} | Diferença: {p2['rss_kb'] - p1['rss_kb']}")

    # IO leitura
    print("\n--- IO READ (bytes) ---")
    print(f"Exec1: {p1['bytes_lidos']} | Exec2: {p2['bytes_lidos']} | Diferença: {p2['bytes_lidos'] - p1['bytes_lidos']}")

    # IO escrita
    print("\n--- IO WRITE (bytes) ---")
    print(f"Exec1: {p1['bytes_escritos']} | Exec2: {p2['bytes_escritos']} | Diferença: {p2['bytes_escritos'] - p1['bytes_escritos']}")

    # Page faults maiores
    print("\n--- PAGE FAULTS MAJ ---")
    print(f"Exec1: {p1['majflt']} | Exec2: {p2['majflt']} | Diferença: {p2['majflt'] - p1['majflt']}")

    print("\n============================================\n")


# Menu simples para escolher os arquivos a comparar
def menu_compare():
    # lista todos os JSON do tipo dados_*.json
    arquivos = sorted([
        f for f in os.listdir(PASTA_DOCS)
        if f.startswith("dados_") and f.endswith(".json")
    ])

    total = len(arquivos)

    if total < 2:
        print("É necessário pelo menos 2 arquivos para comparar.")
        return

    # Mostra os arquivos disponíveis para escolha
    print(f"\nArquivos disponíveis ({total}):")
    for i, f in enumerate(arquivos, 1):
        print(f"{i} - {f}")

    # Usuário escolhe dois arquivos pelo índice
    escolha1 = int(input("Escolha o primeiro arquivo (número): "))
    escolha2 = int(input("Escolha o segundo arquivo (número): "))

    # Validação das escolhas
    if escolha1 == escolha2 or escolha1 < 1 or escolha1 > total or escolha2 < 1 or escolha2 > total:
        print("Escolha inválida.")
        return

    # Monta caminhos completos
    arq1 = os.path.join(PASTA_DOCS, arquivos[escolha1 - 1])
    arq2 = os.path.join(PASTA_DOCS, arquivos[escolha2 - 1])

    # Compara os dois relatórios
    compare_tools(arq1, arq2)


# Executa o menu se o script for rodado diretamente
if __name__ == "__main__":
    menu_compare()
