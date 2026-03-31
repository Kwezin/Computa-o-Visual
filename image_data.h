#pragma once

#include <SDL3/SDL.h>

/*
 * ImageData
 * ──────────────────────────────────────────────────────────────
 * IMPLEMENTAÇÃO COMPLETA: Membro 2 (grayscale) + Membro 3 (histograma)
 *                         + Membro 4 (equalização)
 *
 * Este header define a interface que o app.cpp usa.
 * O Membro 1 criou os stubs para o projeto compilar.
 * ──────────────────────────────────────────────────────────────
 */
class ImageData {
public:
    explicit ImageData(SDL_Surface* rawSurface);
    ~ImageData();

    // ── Membro 2 implementa ──────────────────
    bool           isGrayscale()         const;
    SDL_Surface*   getGrayscaleSurface() const;

    // ── Membro 3 implementa ──────────────────
    // histogram[i] = número de pixels com intensidade i (0..255)
    const int*     getHistogram()        const;
    float          getMeanIntensity()    const;
    float          getStdDeviation()     const;

    // ── Membro 4 implementa ──────────────────
    SDL_Surface*   getEqualizedSurface() const;
    bool           isEqualized()         const;
    void           equalize();
    void           revertToOriginal();

    // ── Usado por app.cpp (save) ─────────────
    // Retorna a superfície atualmente ativa (cinza ou equalizada)
    SDL_Surface*   getCurrentSurface()   const;

private:
    SDL_Surface* m_original   = nullptr; // RGBA original (colorida)
    SDL_Surface* m_gray       = nullptr; // Escala de cinza
    SDL_Surface* m_equalized  = nullptr; // Equalizada
    bool         m_isEq       = false;
    bool         m_wasGray    = false;
    int          m_histogram[256] = {};
    float        m_mean       = 0.f;
    float        m_stddev     = 0.f;
};
