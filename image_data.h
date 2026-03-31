#pragma once

#include <SDL3/SDL.h>

/*
 * ImageData
 * ──────────────────────────────────────────────────────────────
 * Kauê Henrique Matias Alves
 * Diego Spagnuolo Sugai
 * ──────────────────────────────────────────────────────────────
 */
class ImageData {
public:
    explicit ImageData(SDL_Surface* rawSurface);
    ~ImageData();

    bool           isGrayscale()         const;
    SDL_Surface*   getGrayscaleSurface() const;

    // histogram[i] = número de pixels com intensidade i (0..255)
    // Reflete sempre a imagem ATUAL (cinza ou equalizada)
    const int*     getHistogram()        const;
    float          getMeanIntensity()    const;
    float          getStdDeviation()     const;

    // Recalcula histograma/média/desvio da superfície atual.
    // chamar isso após equalize() e revertToOriginal().
    void           recalcStats();

    SDL_Surface*   getEqualizedSurface() const;
    bool           isEqualized()         const;
    void           equalize();
    void           revertToOriginal();

    // ── Geral ────────────────────────────────
    SDL_Surface*   getCurrentSurface()   const;

private:
    SDL_Surface* m_original  = nullptr;
    SDL_Surface* m_gray      = nullptr;
    SDL_Surface* m_equalized = nullptr;
    bool         m_isEq      = false;
    bool         m_wasGray   = false;
    int          m_histogram[256] = {};
    float        m_mean      = 0.f;
    float        m_stddev    = 0.f;
};
