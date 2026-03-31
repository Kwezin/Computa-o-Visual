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
    r = p[0]; g = p[1]; b = p[2]; a = p[3];
}

static inline void setPixel(SDL_Surface* s, int x, int y,
                             Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    Uint8* p = (Uint8*)s->pixels + y * s->pitch + x * 4;
    p[0] = r; p[1] = g; p[2] = b; p[3] = a;
}

// ─────────────────────────────────────────────────────────────
// Detecta se imagem já é cinza (amostra até 10 000 pixels)
// ─────────────────────────────────────────────────────────────
static bool detectGrayscale(SDL_Surface* surface) {
    int total = surface->w * surface->h;
    int step  = std::max(1, total / 10000);
    for (int i = 0; i < total; i += step) {
        Uint8 r, g, b, a;
        getPixel(surface, i % surface->w, i / surface->w, r, g, b, a);
        if (r != g || g != b) return false;
    }
    return true;
}

// ─────────────────────────────────────────────────────────────
// Converte para cinza: Y = 0.2125R + 0.7154G + 0.0721B
// ─────────────────────────────────────────────────────────────
static SDL_Surface* convertToGrayscale(SDL_Surface* src) {
    SDL_Surface* gray = SDL_CreateSurface(src->w, src->h,
                                          SDL_PIXELFORMAT_RGBA8888);
    if (!gray) return nullptr;

    for (int y = 0; y < src->h; y++) {
        for (int x = 0; x < src->w; x++) {
            Uint8 r, g, b, a;
            getPixel(src, x, y, r, g, b, a);
            Uint8 lum = (Uint8)(0.2125f * r + 0.7154f * g + 0.0721f * b);
            setPixel(gray, x, y, lum, lum, lum, a);
        }
    }
    return gray;
}

// ─────────────────────────────────────────────────────────────
// Calcula histograma, média e desvio padrão de uma superfície cinza
// ─────────────────────────────────────────────────────────────
static void computeStats(SDL_Surface* gray, int histogram[256],
                          float& mean, float& stddev) {
    std::memset(histogram, 0, 256 * sizeof(int));

    long long total = (long long)gray->w * gray->h;

    for (int y = 0; y < gray->h; y++) {
        for (int x = 0; x < gray->w; x++) {
            Uint8 r, g, b, a;
            getPixel(gray, x, y, r, g, b, a);
            histogram[r]++;
        }
    }

    // Média ponderada
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
    m_wasGray  = detectGrayscale(rawSurface);

    if (m_wasGray) {
        std::cout << "Imagem ja esta em escala de cinza." << std::endl;
        m_gray = SDL_DuplicateSurface(rawSurface);
    } else {
        std::cout << "Imagem colorida — convertendo para escala de cinza." << std::endl;
        m_gray = convertToGrayscale(rawSurface);
    }

    if (!m_gray) {
        std::cerr << "Falha ao obter superficie em escala de cinza." << std::endl;
        return;
    }

    // Calcular estatísticas da imagem cinza original
    computeStats(m_gray, m_histogram, m_mean, m_stddev);

    std::cout << "Media: " << m_mean
              << " | Desvio padrao: " << m_stddev << std::endl;
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
// recalcStats — recalcula histograma/média/desvio da superfície atual
// Chamado pelo Membro 4 após equalizar ou reverter
// ─────────────────────────────────────────────────────────────
void ImageData::recalcStats() {
    SDL_Surface* src = getCurrentSurface();
    if (!src) return;
    computeStats(src, m_histogram, m_mean, m_stddev);
}

// ─────────────────────────────────────────────────────────────
// equalize() e revertToOriginal() — implementados pelo Membro 4
// ─────────────────────────────────────────────────────────────
void ImageData::equalize() {
    // TODO (Membro 4): implementar equalização por CDF
    m_isEq = true;
    recalcStats(); // atualizar histograma após equalizar
}

void ImageData::revertToOriginal() {
    // TODO (Membro 4): liberar m_equalized, voltar para m_gray
    m_isEq = false;
    recalcStats(); // restaurar histograma original
}