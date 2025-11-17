# Compilador e flags (inclui diretório include/)
CC = gcc
CFLAGS = -Iinclude

# Diretórios do projeto
SRC_DIR = src
TEST_DIR = tests
BIN_DIR = bin
OBJ_DIR = obj

# Arquivos .c da pasta src → vira lista de .o
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Executável final
TARGET = $(BIN_DIR)/monitor

# Testes (gera objetos separados)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS = $(TEST_SRCS:$(TEST_DIR)/%.c=$(OBJ_DIR)/%.test.o)

# Objetos específicos por teste
TEST_CPU_OBJ = $(OBJ_DIR)/test_cpu.test.o
TEST_MEMORY_OBJ = $(OBJ_DIR)/test_memory.test.o
TEST_IO_OBJ = $(OBJ_DIR)/test_io.test.o

# Binários finais dos testes
BIN_CPU = $(BIN_DIR)/test_cpu
BIN_MEMORY = $(BIN_DIR)/test_memory
BIN_IO = $(BIN_DIR)/test_io

# ----------- Regras principais -----------

# Compila tudo (padrão)
all: $(TARGET)

# Gera o binário principal
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)     # garante bin/
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)
	@echo " Executável criado em $(TARGET)"

# Compila cada .c do src/ em .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compila arquivos de teste .c → .test.o
$(OBJ_DIR)/%.test.o: $(TEST_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ----------- Binários de Teste -----------

# Teste de CPU (remove main.o para não ter 2 mains)
$(BIN_CPU): $(TEST_CPU_OBJ) $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(filter-out $(OBJ_DIR)/main.o,$(OBJS)) $(TEST_CPU_OBJ) -o $@

# Teste de memória
$(BIN_MEMORY): $(TEST_MEMORY_OBJ) $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(filter-out $(OBJ_DIR)/main.o,$(OBJS)) $(TEST_MEMORY_OBJ) -o $@

# Teste de IO
$(BIN_IO): $(TEST_IO_OBJ) $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(filter-out $(OBJ_DIR)/main.o,$(OBJS)) $(TEST_IO_OBJ) -o $@

# ----------- Comandos úteis -----------

# Roda o monitor
run: $(TARGET)
	./$(TARGET)

# Roda testes individuais
cpu-test: $(BIN_CPU)
	./$(BIN_CPU)

memory-test: $(BIN_MEMORY)
	./$(BIN_MEMORY)

io-test: $(BIN_IO)
	./$(BIN_IO)

# Compila todos os testes
run-tests: $(BIN_CPU) $(BIN_MEMORY) $(BIN_IO)
	@echo " Testes prontos. Execute: ./bin/test_cpu, test_memory, test_io"

# ----------- Limpeza -----------

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo " Diretórios limpos!"
