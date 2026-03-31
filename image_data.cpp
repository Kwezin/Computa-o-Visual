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
    for (int y = 0; y < src->h; y++)
        for (int x = 0; x < src->w; x++) {
            Uint8 r, g, b, a;
            getPixel(src, x, y, r, g, b, a);
            Uint8 lum = (Uint8)(0.2125f * r + 0.7154f * g + 0.0721f * b);
            setPixel(gray, x, y, lum, lum, lum, a);
        }
    return gray;
}

// ─────────────────────────────────────────────────────────────
// Calcula histograma, média e desvio padrão
// ─────────────────────────────────────────────────────────────
static void computeStats(SDL_Surface* gray, int histogram[256],
                          float& mean, float& stddev) {
    std::memset(histogram, 0, 256 * sizeof(int));
    long long total = (long long)gray->w * gray->h;

    for (int y = 0; y < gray->h; y++)
        for (int x = 0; x < gray->w; x++) {
            Uint8 r, g, b, a;
            getPixel(gray, x, y, r, g, b, a);
            histogram[r]++;
        }

    double sum = 0.0;
    for (int i = 0; i < 256; i++) sum += (double)i * histogram[i];
    mean = (float)(sum / total);

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

void ImageData::recalcStats() {
    SDL_Surface* src = getCurrentSurface();
    if (src) computeStats(src, m_histogram, m_mean, m_stddev);
}

// ─────────────────────────────────────────────────────────────
// equalize — equalização de histograma por CDF
//
// Algoritmo:
//   1. Calcular histograma da imagem cinza
//   2. Calcular CDF (distribuição acumulada)
//   3. Normalizar: new_val = round((CDF[i] - CDF_min) / (N - CDF_min) * 255)
//   4. Aplicar mapeamento pixel a pixel em uma nova superfície
// ─────────────────────────────────────────────────────────────
void ImageData::equalize() {
    if (!m_gray) return;

    int w = m_gray->w;
    int h = m_gray->h;
    long long N = (long long)w * h; // total de pixels

    // ── 1. Histograma da cinza original ──────
    int hist[256] = {};
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            Uint8 r, g, b, a;
            getPixel(m_gray, x, y, r, g, b, a);
            hist[r]++;
        }

    // ── 2. CDF acumulada ─────────────────────
    long long cdf[256] = {};
    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++)
        cdf[i] = cdf[i - 1] + hist[i];

    // ── 3. CDF mínima (primeiro valor não-zero) ─
    long long cdf_min = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] > 0) { cdf_min = cdf[i]; break; }
    }

    // ── 4. Tabela de remapeamento ─────────────
    // Fórmula: eq[i] = round((cdf[i] - cdf_min) / (N - cdf_min) * 255)
    Uint8 lut[256] = {};
    long long denom = N - cdf_min;
    for (int i = 0; i < 256; i++) {
        if (denom <= 0) {
            lut[i] = (Uint8)i;
        } else {
            double val = ((double)(cdf[i] - cdf_min) / denom) * 255.0;
            lut[i] = (Uint8)std::round(std::max(0.0, std::min(255.0, val)));
        }
    }

    // ── 5. Criar nova superfície equalizada ───
    if (m_equalized) {
        SDL_DestroySurface(m_equalized);
        m_equalized = nullptr;
    }

    m_equalized = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA8888);
    if (!m_equalized) {
        std::cerr << "equalize: falha ao criar superficie: "
                  << SDL_GetError() << std::endl;
        return;
    }

    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            Uint8 r, g, b, a;
            getPixel(m_gray, x, y, r, g, b, a);
            Uint8 eq = lut[r];
            setPixel(m_equalized, x, y, eq, eq, eq, a);
        }

    m_isEq = true;

    // Atualizar histograma/stats para refletir a imagem equalizada
    recalcStats();

    std::cout << "Histograma equalizado. Nova media: " << m_mean
              << " | Novo desvio: " << m_stddev << std::endl;
}

// ─────────────────────────────────────────────────────────────
// revertToOriginal — volta para a imagem cinza sem recarregar do disco
// ─────────────────────────────────────────────────────────────
void ImageData::revertToOriginal() {
    // A superfície m_gray já está em memória — basta mudar o flag
    m_isEq = false;

    // Liberar a equalizada para liberar memória
    if (m_equalized) {
        SDL_DestroySurface(m_equalized);
        m_equalized = nullptr;
    }

    // Restaurar histograma/stats da imagem cinza original
    recalcStats();

    std::cout << "Imagem revertida para escala de cinza original." << std::endl;
}