#pragma once

#include <SDL3/SDL.h>

class ImageData;

/*
 * HistogramWindow
 * ──────────────────────────────────────────────────────────────
 * IMPLEMENTAÇÃO: Membro 2 (criação da janela) + Membro 3 (histograma)
 *                + Membro 4 (botão equalizar)
 *
 * Janela filha da janela principal. Tamanho fixo definido em app.cpp.
 * ──────────────────────────────────────────────────────────────
 */
class HistogramWindow {
public:
    HistogramWindow(SDL_Window* parent, int x, int y, int w, int h, ImageData* data);
    ~HistogramWindow();

    bool init();
    void handleEvent(const SDL_Event& e);
    void render();

private:
    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    ImageData*    m_data     = nullptr;
    SDL_Window*   m_parent   = nullptr;
    int m_x, m_y, m_w, m_h;
};
