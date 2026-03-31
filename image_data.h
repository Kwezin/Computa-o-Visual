#pragma once

#include <SDL3/SDL.h>

/*
 * ImageData
 * ──────────────────────────────────────────────────────────────
 * Membro 1 — criou o header com a interface
 * Membro 2 — implementou detecção/conversão cinza + cálculo básico
 * Membro 3 — refinou cálculo de estatísticas e adicionou recalcStats()
 * Membro 4 — implementará equalize() e revertToOriginal()
 * ──────────────────────────────────────────────────────────────
 */
class ImageData {
public:
    explicit ImageData(SDL_Surface* rawSurface);
    ~ImageData();

    // ── Membro 2 ─────────────────────────────
    bool           isGrayscale()         const;
    SDL_Surface*   getGrayscaleSurface() const;

    // ── Membro 3 ─────────────────────────────
    // histogram[i] = número de pixels com intensidade i (0..255)
    // Reflete sempre a imagem ATUAL (cinza ou equalizada)
    const int*     getHistogram()        const;
    float          getMeanIntensity()    const;
    float          getStdDeviation()     const;

    // Recalcula histograma/média/desvio da superfície atual.
    // O Membro 4 deve chamar isso após equalize() e revertToOriginal().
    void           recalcStats();

    // ── Membro 4 ─────────────────────────────
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