#include "histogram_window.h"
#include <iostream>

/*
 * STUB — esqueleto para compilar.
 * Membro 2 implementa init() com criação de janela filha.
 * Membro 3 implementa render() com histograma e textos de análise.
 * Membro 4 adiciona o botão e a lógica de equalização.
 */

HistogramWindow::HistogramWindow(SDL_Window* parent, int x, int y, int w, int h, ImageData* data)
    : m_parent(parent), m_x(x), m_y(y), m_w(w), m_h(h), m_data(data) {}

HistogramWindow::~HistogramWindow() {
    if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window)   { SDL_DestroyWindow(m_window);     m_window   = nullptr; }
}

bool HistogramWindow::init() {
    // TODO (Membro 2): criar janela filha real com SDL_WINDOW_UTILITY ou
    //                  posicionada ao lado da janela principal.
    //
    // Exemplo de criação:
    //   m_window = SDL_CreateWindow("Histograma", m_w, m_h, SDL_WINDOW_UTILITY);
    //   SDL_SetWindowPosition(m_window, m_x, m_y);
    //   m_renderer = SDL_CreateRenderer(m_window, nullptr);
    std::cout << "[HistogramWindow] stub init() chamado." << std::endl;
    return true;
}

void HistogramWindow::handleEvent(const SDL_Event& e) {
    // TODO (Membro 4): processar clique no botão
    (void)e;
}

void HistogramWindow::render() {
    // TODO (Membro 3 + 4): desenhar histograma, labels e botão
    if (!m_renderer) return;
    SDL_SetRenderDrawColor(m_renderer, 20, 20, 20, 255);
    SDL_RenderClear(m_renderer);
    SDL_RenderPresent(m_renderer);
}
