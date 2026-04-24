# Makefile para o projeto Goell-CHM
#
# Alvos principais:
#   make          - Compila o solver (equivalente a 'make all').
#   make all      - Compila o solver e o coloca em build/goell_q_solver.
#   make clean    - Remove o diretório de build e todos os artefatos.
#

CXX = g++
EIGEN_DIR ?= /usr/include/eigen3
CPPFLAGS = -Iinclude -I$(EIGEN_DIR)
CXXFLAGS = -O3 -std=c++17 -Wall -Wextra -pedantic
LDFLAGS =

SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests

TARGET = $(BUILD_DIR)/goell_q_solver
TEST_TARGET = $(BUILD_DIR)/goell_core_smoke

SOURCES = $(shell find $(SRC_DIR) -name '*.cpp' | sort)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJECTS = $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_DIR)/tests/%.o,$(TEST_SOURCES))
CORE_OBJECTS = $(filter-out $(BUILD_DIR)/main.o,$(OBJECTS))
DEPS = $(OBJECTS:.o=.d) $(TEST_OBJECTS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $^ -o $@
	@echo "Solver compilado com sucesso: $@"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(TEST_TARGET): $(CORE_OBJECTS) $(TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $^ -o $@
	@echo "Smoke test compilado com sucesso: $@"

check-cpp: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	@echo "Limpando artefatos de build..."
	@rm -rf $(BUILD_DIR)

-include $(DEPS)

.PHONY: all clean check-cpp
