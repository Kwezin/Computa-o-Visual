#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

class ImageData;

/*
 * HistogramWindow
 * Janela secundária (filha da principal). Tamanho fixo.
 * Exibe: histograma | análise estatística | botão equalizar
 *
 * Implementação dividida:
 *   Membro 2 — criação da janela, layout, drawHistogram() base, drawButton()
 *   Membro 3 — refina drawHistogram() e drawAnalysisInfo()
 *   Membro 4 — conecta handleEvent() à lógica de equalização
 */
class HistogramWindow {
public:
    HistogramWindow(SDL_Window* parent, int x, int y, int w, int h, ImageData* data);
    ~HistogramWindow();

    bool init();
    void handleEvent(const SDL_Event& e);
    void render();

    // Membro 4 chama isso depois de equalizar para atualizar o histograma exibido
    void refreshHistogram();

private:
    void drawHistogram();
    void drawAnalysisInfo();
    void drawButton();
    void drawText(const std::string& text, int x, int y, SDL_Color color);

    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    TTF_Font*     m_font     = nullptr;
    ImageData*    m_data     = nullptr;
    SDL_Window*   m_parent   = nullptr;

    int m_x, m_y, m_w, m_h;

    SDL_FRect m_btnRect = {};

    enum class BtnState { Normal, Hover, Pressed };
    BtnState m_btnState = BtnState::Normal;
};