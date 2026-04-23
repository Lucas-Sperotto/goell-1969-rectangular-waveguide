# Makefile para o projeto Goell-CHM
#
# Alvos principais:
#   make          - Compila o solver (equivalente a 'make all').
#   make all      - Compila o solver e o coloca em build/goell_q_solver.
#   make clean    - Remove o diretório de build e todos os artefatos.
#

# Compilador e flags
CXX = g++
CXXFLAGS = -O3 -std=c++17 -I /usr/include/eigen3 -Wall -Wextra -pedantic
LDFLAGS =

# Diretórios
SRC_DIR = src
BUILD_DIR = build

# Alvo
TARGET = $(BUILD_DIR)/goell_q_solver

# Arquivos-fonte C++
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)

# Arquivos-objeto (colocados no diretório de build)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Alvo padrão (o que 'make' faz sem argumentos)
all: $(TARGET)

# Regra de linkagem para criar o executável final
$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $^ -o $@
	@echo "Solver compilado com sucesso: $@"

# Regra para compilar cada arquivo-objeto a partir de seu .cpp
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Alvo para limpeza
clean:
	@echo "Limpando artefatos de build..."
	@rm -rf $(BUILD_DIR)

.PHONY: all clean