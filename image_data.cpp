#include "image_data.h"
#include <cstring>
#include <cmath>
#include <iostream>

// ─────────────────────────────────────────────────────────────
// Helpers para acesso a pixels RGBA8888
// ─────────────────────────────────────────────────────────────
static inline void getPixel(SDL_Surface* s, int x, int y,
                             Uint8& r, Uint8& g, Uint8& b, Uint8& a) {
    Uint8* p = (Uint8*)s->pixels + y * s->pitch + x * 4;
    // SDL_PIXELFORMAT_RGBA8888: R=p[0], G=p[1], B=p[2], A=p[3]
    r = p[0]; g = p[1]; b = p[2]; a = p[3];
}

static inline void setPixel(SDL_Surface* s, int x, int y,
                             Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    Uint8* p = (Uint8*)s->pixels + y * s->pitch + x * 4;
    p[0] = r; p[1] = g; p[2] = b; p[3] = a;
}

// ─────────────────────────────────────────────────────────────
// Detecta se a imagem já está em escala de cinza
// Critério: R == G == B para todos os pixels (amostra até 10k pixels)
// ─────────────────────────────────────────────────────────────
static bool detectGrayscale(SDL_Surface* surface) {
    int w = surface->w;
    int h = surface->h;
    int total = w * h;
    int step = std::max(1, total / 10000); // amostrar até 10 000 pixels

    for (int i = 0; i < total; i += step) {
        int x = i % w;
        int y = i / w;
        Uint8 r, g, b, a;
        getPixel(surface, x, y, r, g, b, a);
        if (r != g || g != b) return false;
    }
    return true;
}

// ─────────────────────────────────────────────────────────────
// Converte para escala de cinza usando a fórmula do enunciado:
//   Y = 0.2125*R + 0.7154*G + 0.0721*B
// ─────────────────────────────────────────────────────────────
static SDL_Surface* convertToGrayscale(SDL_Surface* src) {
    int w = src->w;
    int h = src->h;

    SDL_Surface* gray = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA8888);
    if (!gray) {
        std::cerr << "Erro ao criar superfície cinza: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Uint8 r, g, b, a;
            getPixel(src, x, y, r, g, b, a);

            Uint8 lum = (Uint8)(0.2125f * r + 0.7154f * g + 0.0721f * b);
            setPixel(gray, x, y, lum, lum, lum, a);
        }
    }
    return gray;
}

// ─────────────────────────────────────────────────────────────
// Calcula histograma, média e desvio padrão da imagem cinza
// ─────────────────────────────────────────────────────────────
static void computeHistogram(SDL_Surface* gray, int histogram[256],
                              float& mean, float& stddev) {
    std::memset(histogram, 0, 256 * sizeof(int));

    int w = gray->w;
    int h = gray->h;
    long long total = (long long)w * h;

    // Contagem
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Uint8 r, g, b, a;
            getPixel(gray, x, y, r, g, b, a);
            histogram[r]++;
        }
    }

    // Média
    double sum = 0.0;
    for (int i = 0; i < 256; i++) sum += (double)i * histogram[i];
    mean = (float)(sum / total);

    // Desvio padrão
    double var = 0.0;
    for (int i = 0; i < 256; i++) {
        double diff = i - mean;
        var += diff * diff * histogram[i];
    }
    stddev = (float)std::sqrt(var / total);
}

// ─────────────────────────────────────────────────────────────
// Construtor
// ─────────────────────────────────────────────────────────────
ImageData::ImageData(SDL_Surface* rawSurface) {
    m_original = rawSurface;

    m_wasGray = detectGrayscale(rawSurface);

    if (m_wasGray) {
        // Já é cinza — apenas duplica para m_gray
        std::cout << "Imagem já está em escala de cinza." << std::endl;
        m_gray = SDL_DuplicateSurface(rawSurface);
    } else {
        std::cout << "Imagem colorida — convertendo para escala de cinza." << std::endl;
        m_gray = convertToGrayscale(rawSurface);
    }

    if (!m_gray) {
        std::cerr << "Falha ao obter superfície em escala de cinza." << std::endl;
        return;
    }

    computeHistogram(m_gray, m_histogram, m_mean, m_stddev);
}

// ─────────────────────────────────────────────────────────────
// Destrutor
// ─────────────────────────────────────────────────────────────
ImageData::~ImageData() {
    if (m_equalized) { SDL_DestroySurface(m_equalized); m_equalized = nullptr; }
    if (m_gray)      { SDL_DestroySurface(m_gray);      m_gray      = nullptr; }
    if (m_original)  { SDL_DestroySurface(m_original);  m_original  = nullptr; }
}

// ─────────────────────────────────────────────────────────────
// Getters
// ─────────────────────────────────────────────────────────────
bool         ImageData::isGrayscale()         const { return m_wasGray; }
SDL_Surface* ImageData::getGrayscaleSurface() const { return m_gray; }
const int*   ImageData::getHistogram()        const { return m_histogram; }
float        ImageData::getMeanIntensity()    const { return m_mean; }
float        ImageData::getStdDeviation()     const { return m_stddev; }
SDL_Surface* ImageData::getEqualizedSurface() const { return m_equalized; }
bool         ImageData::isEqualized()         const { return m_isEq; }

SDL_Surface* ImageData::getCurrentSurface() const {
    if (m_isEq && m_equalized) return m_equalized;
    return m_gray;
}

// ─────────────────────────────────────────────────────────────
// equalize() e revertToOriginal() — implementados pelo Membro 4
// ─────────────────────────────────────────────────────────────
void ImageData::equalize() {
    // TODO (Membro 4)
    m_isEq = true;
}

void ImageData::revertToOriginal() {
    // TODO (Membro 4)
    m_isEq = false;
}