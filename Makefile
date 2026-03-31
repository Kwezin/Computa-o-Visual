# ─────────────────────────────────────────────────────────
# Makefile — Projeto 1: Processamento de Imagens
# Compilador: g++ (C++17)
# Dependências: SDL3, SDL3_image, SDL3_ttf
# ─────────────────────────────────────────────────────────

CXX      := g++
TARGET   := programa
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 \
            $(shell pkg-config --cflags sdl3 sdl3-image sdl3-ttf)
LDFLAGS  := $(shell pkg-config --libs   sdl3 sdl3-image sdl3-ttf)

# Todos os .cpp na raiz do projeto
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)

# ─────────────────────────────────────────────────────────
.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Build OK → ./$(TARGET)"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) output_image.png

run: all
	./$(TARGET) $(IMG)
# Uso: make run IMG=foto.png

# ─────────────────────────────────────────────────────────
# Dependências de cabeçalho (geradas automaticamente)
-include $(OBJS:.o=.d)
%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM -MF $@ $<
