# Projeto 1 — Processamento de Imagens

**Disciplina:** Computação Visual — Universidade Presbiteriana Mackenzie  
**Professsor:** André Kishimoto  

---

## Integrantes

| Nome | RA |
|------|----|
| [Kaue Henrique Matias Alves] | 10417894 |
| [Diego Spagnuolo Sugai] | 10417329 |

---

## O que é o projeto

Software de processamento de imagens com interface gráfica usando **SDL3**.  
Carrega uma imagem (PNG, JPG ou BMP), converte para escala de cinza, exibe o histograma e permite equalizar/restaurar a imagem.

---

## Como funciona

```
programa <caminho_da_imagem>
```

Ao iniciar, o programa:
1. Carrega a imagem e verifica erros (arquivo inexistente, formato inválido)
2. Detecta se a imagem é colorida ou já está em escala de cinza
3. Converte para escala de cinza usando: `Y = 0.2125·R + 0.7154·G + 0.0721·B`
4. Abre **duas janelas**:
   - **Janela principal** — exibe a imagem (tamanho adaptado, centralizada no monitor)
   - **Janela secundária** — histograma, análise estatística e botão de equalização

### Controles

| Ação | Resultado |
|------|-----------|
| Tecla `S` | Salva a imagem atual em `output_image.png` |
| Botão "Equalizar" | Aplica equalização de histograma |
| Botão "Ver original" | Reverte para a imagem em escala de cinza original |
| Fechar qualquer janela | Encerra o programa |

---

## Dependências

- **SDL3** ≥ 3.0
- **SDL3_image** ≥ 3.0 (para PNG, JPG, BMP)
- **SDL3_ttf** ≥ 3.0 (para exibir texto na janela secundária)
- **g++** ≥ 15.1.0 com suporte a C++17

### Instalação das dependências (Linux/Ubuntu)

```bash
sudo apt install libsdl3-dev libsdl3-image-dev libsdl3-ttf-dev
```

### Instalação das dependências (macOS com Homebrew)

```bash
brew install sdl3 sdl3_image sdl3_ttf
```

---

## Compilação e execução

```bash
# Compilar
make

# Executar com uma imagem
./programa teste_image.jpeg

# Ou usando o atalho do Makefile
make run teste_image.jpeg

# Limpar arquivos compilados
make clean
```
OU

g++ *.cpp -o programa.exe -lSDL3 -lSDL3_image -lSDL3_ttf

./programa.exe teste_image.jpeg


---

## Estrutura do projeto

```
proj1/
├── main.cpp              # Ponto de entrada
├── app.h / app.cpp       # Controlador principal (loop, eventos, render)
├── image_loader.h/.cpp   # Carregamento de imagens com SDL_image
├── image_data.h/.cpp     # Processamento: cinza, histograma, equalização
├── histogram_window.h/.cpp # Janela secundária: histograma e botão
├── Makefile
├── .gitignore
└── README.md
```

---

## Contribuições

- **[Kaue Henrque Matias Aves]:** `main.cpp`, `app.h/cpp`, `image_loader.h/cpp`, `Makefile`, `.gitignore`, `README.md`
- **[Diego Spagnuolo Sugai]:** Lógica de detecção/conversão em `image_data.cpp`, criação da janela secundária em `histogram_window.cpp`
- **[Kaue Henrque Matias Aves]:** Cálculo do histograma, média, desvio padrão e exibição gráfica em `histogram_window.cpp`
- **[Diego Spagnuolo Sugai]:** Algoritmo de equalização em `image_data.cpp`, botão interativo em `histogram_window.cpp`
