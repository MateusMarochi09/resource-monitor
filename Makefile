# ---------------------- Compilador e flags ----------------------
CC = gcc
CFLAGS = -Iinclude -Wall -Wextra

# ---------------------- Diretórios ----------------------
SRC_DIR = src
TEST_DIR = tests
BIN_DIR = bin
OBJ_DIR = obj

# ---------------------- Arquivos do projeto ----------------------
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TARGET = $(BIN_DIR)/monitor

# ---------------------- Testes individuais ----------------------
TEST_CPU_OBJ    = $(OBJ_DIR)/test_cpu.test.o
TEST_MEMORY_OBJ = $(OBJ_DIR)/test_memory.test.o
TEST_IO_OBJ     = $(OBJ_DIR)/test_io.test.o

BIN_CPU    = $(BIN_DIR)/test_cpu
BIN_MEMORY = $(BIN_DIR)/test_memory
BIN_IO     = $(BIN_DIR)/test_io
BIN_ALL   = $(BIN_DIR)/all_tests

# ---------------------- Regras principais ----------------------
all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $@
	@echo "Executável criado: $@"

# ---------------------- Compilação de objetos ----------------------
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Objetos de teste
$(OBJ_DIR)/%.test.o: $(TEST_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------- Binários de teste ----------------------
$(BIN_CPU): $(TEST_CPU_OBJ) $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(filter-out $(OBJ_DIR)/main.o,$(OBJS)) $(TEST_CPU_OBJ) -o $@

$(BIN_MEMORY): $(TEST_MEMORY_OBJ) $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(filter-out $(OBJ_DIR)/main.o,$(OBJS)) $(TEST_MEMORY_OBJ) -o $@

$(BIN_IO): $(TEST_IO_OBJ) $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(filter-out $(OBJ_DIR)/main.o,$(OBJS)) $(TEST_IO_OBJ) -o $@

# all_tests (combina todos os testes)
$(BIN_ALL): $(TEST_DIR)/all_tests.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/all_tests.c -o $@

# ---------------------- Comandos úteis ----------------------
run: $(TARGET)
	./$(TARGET)

cpu-test: $(BIN_CPU)
	./$(BIN_CPU)

memory-test: $(BIN_MEMORY)
	./$(BIN_MEMORY)

io-test: $(BIN_IO)
	./$(BIN_IO)

all-tests: $(BIN_ALL)
	./$(BIN_ALL)

run-tests: $(BIN_CPU) $(BIN_MEMORY) $(BIN_IO)
	@echo "Testes prontos. Execute: ./bin/test_cpu, test_memory, test_io"

# ---------------------- Limpeza ----------------------
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Diretórios limpos!"