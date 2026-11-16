# ============================================================
# Compiladores e ferramentas
# ============================================================

CC = gcc
AR = ar

ARM_CC ?= arm-none-eabi-gcc
ARM_AR ?= arm-none-eabi-ar

# ============================================================
# Flags
# ============================================================

CFLAGS = -std=c99 -Wall -Wextra -Werror -pedantic
DEPFLAGS = -MMD -MP
COVERAGE_FLAGS = --coverage -O0 -g

# Linux x86_64
LINUX_CFLAGS = $(CFLAGS) -m64

# ARM Cortex-M4 (cross-compilation)
ARM_CPU ?= cortex-m4
ARM_ARCH_FLAGS = -mcpu=$(ARM_CPU) -mthumb

ARM_CFLAGS = $(CFLAGS) $(ARM_ARCH_FLAGS) \
             -ffunction-sections -fdata-sections -Os

INCLUDES = -Ilib/inc -Iapp/inc -Itests

# ============================================================
# Diretórios
# ============================================================

BUILD = build
ARM_BUILD = $(BUILD)/arm

# ============================================================
# Biblioteca - Linux
# ============================================================

LIB_SRC = $(wildcard lib/src/*.c)
LIB_OBJ = $(patsubst lib/src/%.c,$(BUILD)/%.o,$(LIB_SRC))
LIB_DEP = $(LIB_OBJ:.o=.d)

LIB_BIN = $(BUILD)/libpos.a

# ============================================================
# Aplicação
# ============================================================

APP_SRC = $(wildcard app/src/*.c)
APP_OBJ = $(patsubst app/src/%.c,$(BUILD)/app_%.o,$(APP_SRC))
APP_DEP = $(APP_OBJ:.o=.d)

# ============================================================
# Demo
# ============================================================

DEMO_SRC = $(wildcard demo/src/*.c)
DEMO_OBJ = $(patsubst demo/src/%.c,$(BUILD)/demo_%.o,$(DEMO_SRC))
DEMO_DEP = $(DEMO_OBJ:.o=.d)

DEMO_BIN = $(BUILD)/pos_demo

# ============================================================
# Testes
# ============================================================

TEST_SRC = $(wildcard tests/*.c) \
           $(wildcard tests/unit/*.c) \
           $(wildcard tests/integration/*.c)

TEST_BIN = $(BUILD)/test_pos

# ============================================================
# Biblioteca - ARM Cortex-M
# ============================================================

ARM_LIB_OBJ = $(patsubst lib/src/%.c,$(ARM_BUILD)/%.o,$(LIB_SRC))
ARM_LIB_DEP = $(ARM_LIB_OBJ:.o=.d)

ARM_LIB_BIN = $(ARM_BUILD)/libpos.a

# ============================================================
# Targets principais
# ============================================================

all: linux

# ============================================================
# Linux x86_64
# ============================================================

linux: lib

lib: $(LIB_BIN)

$(LIB_BIN): $(LIB_OBJ)
	rm -f $@
	$(AR) rcs $@ $^

$(BUILD)/%.o: lib/src/%.c
	mkdir -p $(BUILD)
	$(CC) $(LINUX_CFLAGS) $(DEPFLAGS) $(INCLUDES) \
		-MF $(BUILD)/$*.d -c $< -o $@

$(BUILD)/app_%.o: app/src/%.c
	mkdir -p $(BUILD)
	$(CC) $(LINUX_CFLAGS) $(DEPFLAGS) $(INCLUDES) \
		-MF $(BUILD)/app_$*.d -c $< -o $@

$(BUILD)/demo_%.o: demo/src/%.c
	mkdir -p $(BUILD)
	$(CC) $(LINUX_CFLAGS) $(DEPFLAGS) $(INCLUDES) \
		-MF $(BUILD)/demo_$*.d -c $< -o $@

# ============================================================
# ARM Cortex-M - compilação cruzada real
# ============================================================

arm:
	@command -v $(ARM_CC) >/dev/null 2>&1 || { \
		echo "ERRO: $(ARM_CC) não encontrado."; \
		echo "Instale a ARM GNU Toolchain para compilar para Cortex-M."; \
		exit 1; \
	}
	@command -v $(ARM_AR) >/dev/null 2>&1 || { \
		echo "ERRO: $(ARM_AR) não encontrado."; \
		exit 1; \
	}
	$(MAKE) arm-lib

arm-lib: $(ARM_LIB_BIN)

$(ARM_LIB_BIN): $(ARM_LIB_OBJ)
	rm -f $@
	$(ARM_AR) rcs $@ $^

$(ARM_BUILD)/%.o: lib/src/%.c
	mkdir -p $(ARM_BUILD)
	$(ARM_CC) $(ARM_CFLAGS) $(DEPFLAGS) $(INCLUDES) \
		-MF $(ARM_BUILD)/$*.d -c $< -o $@

# ============================================================
# ARM Cortex-M - simulação da configuração
# ============================================================

arm-sim:
	@echo ""
	@echo "=========================================="
	@echo " Simulação de cross-compilation ARM"
	@echo "=========================================="
	@echo "Compilador: $(ARM_CC)"
	@echo "Arquitetura: $(ARM_CPU)"
	@echo "Modo: Thumb"
	@echo "Destino: $(ARM_LIB_BIN)"
	@echo ""
	@echo "Comandos que seriam executados:"
	$(MAKE) -Bn arm-lib

# ============================================================
# Testes
# ============================================================

test: lib $(APP_OBJ)
	$(CC) $(LINUX_CFLAGS) $(INCLUDES) \
		$(TEST_SRC) $(APP_OBJ) $(LIB_BIN) -o $(TEST_BIN)
	./$(TEST_BIN)

# ============================================================
# Demo
# ============================================================

demo: lib $(APP_OBJ) $(DEMO_OBJ)
	$(CC) $(LINUX_CFLAGS) $(APP_OBJ) $(DEMO_OBJ) \
		$(LIB_BIN) -o $(DEMO_BIN)
	./$(DEMO_BIN)

# ============================================================
# Coverage
# ============================================================

coverage: clean
	$(MAKE) CFLAGS="$(CFLAGS) $(COVERAGE_FLAGS)" test

	@echo ""
	@echo "Gerando coverage da biblioteca..."
	@gcov -b -c -o $(BUILD) $(LIB_SRC)

	@echo ""
	@echo "Gerando coverage da aplicação..."
	@for src in $(APP_SRC); do \
		obj="$(BUILD)/app_$$(basename $$src .c).o"; \
		gcov -b -c -o $$obj $$src; \
	done

	$(MAKE) clean

# ============================================================
# Limpeza
# ============================================================

clean:
	rm -rf $(BUILD)

# ============================================================
# Dependências automáticas
# ============================================================

-include $(LIB_DEP) $(APP_DEP) $(DEMO_DEP) $(ARM_LIB_DEP)

.PHONY: all linux lib arm arm-lib arm-sim test demo coverage clean