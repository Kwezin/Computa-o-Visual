#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

class ImageData;
class App;

/*
 * HistogramWindow — janela secundária (filha da principal)
 *
 * Membro 1 — criou os stubs
 * Membro 2 — criou a janela e o layout base
 * Membro 3 — refinou histograma e análise estatística
 * Membro 4 — conectou o botão à equalização e ao App
 */
class HistogramWindow {
public:
    HistogramWindow(SDL_Window* parent, int x, int y, int w, int h,
                    ImageData* data, App* app);
    ~HistogramWindow();

    bool init();
    void handleEvent(const SDL_Event& e);
    void render();
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
    App*          m_app      = nullptr;   // para notificar troca de textura
    SDL_Window*   m_parent   = nullptr;

    int m_x, m_y, m_w, m_h;

    SDL_FRect m_btnRect = {};

    enum class BtnState { Normal, Hover, Pressed };
    BtnState m_btnState = BtnState::Normal;
};