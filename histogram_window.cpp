#include "histogram_window.h"
#include "image_data.h"
#include "app.h"
#include <iostream>
#include <cmath>
#include <cstdio>

// ─────────────────────────────────────────────────────────────
// Constantes de layout
// ─────────────────────────────────────────────────────────────
static constexpr int PADDING        = 16;
static constexpr int TITLE_H        = 28;
static constexpr int HIST_TOP       = PADDING + TITLE_H;
static constexpr int HIST_HEIGHT    = 180;
static constexpr int HIST_BOTTOM    = HIST_TOP + HIST_HEIGHT;
static constexpr int AXIS_LABEL_H   = 18;
static constexpr int DIVIDER_Y      = HIST_BOTTOM + AXIS_LABEL_H + 10;
static constexpr int INFO_TOP       = DIVIDER_Y + 12;
static constexpr int LINE_H         = 22;
static constexpr int BTN_H          = 40;
static constexpr int BTN_MARGIN     = 16;

// ─────────────────────────────────────────────────────────────
// Construtor / Destrutor
// ─────────────────────────────────────────────────────────────
HistogramWindow::HistogramWindow(SDL_Window* parent, int x, int y,
                                 int w, int h, ImageData* data, App* app)
    : m_parent(parent), m_x(x), m_y(y), m_w(w), m_h(h),
      m_data(data), m_app(app)
{
    m_btnRect = {
        (float)BTN_MARGIN,
        (float)(h - BTN_H - BTN_MARGIN),
        (float)(w - 2 * BTN_MARGIN),
        (float)BTN_H
    };
}

HistogramWindow::~HistogramWindow() {
    if (m_font)     { TTF_CloseFont(m_font);           m_font     = nullptr; }
    if (m_renderer) { SDL_DestroyRenderer(m_renderer);  m_renderer = nullptr; }
    if (m_window)   { SDL_DestroyWindow(m_window);     m_window   = nullptr; }
    TTF_Quit();
}

// ─────────────────────────────────────────────────────────────
// init
// ─────────────────────────────────────────────────────────────
bool HistogramWindow::init() {
    m_window = SDL_CreateWindow("Histograma", m_w, m_h,
                                SDL_WINDOW_UTILITY | SDL_WINDOW_NOT_RESIZABLE);
    if (!m_window) {
        std::cerr << "HistogramWindow: SDL_CreateWindow falhou: "
                  << SDL_GetError() << std::endl;
        return false;
    }

    SDL_SetWindowParent(m_window, m_parent);
    SDL_SetWindowPosition(m_window, m_x, m_y);

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        std::cerr << "HistogramWindow: SDL_CreateRenderer falhou: "
                  << SDL_GetError() << std::endl;
        return false;
    }

    if (!TTF_Init()) {
        std::cerr << "TTF_Init falhou: " << SDL_GetError() << std::endl;
        return false;
    }

    const char* fontPaths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "C:/Windows/Fonts/arial.ttf",
        nullptr
    };
    for (int i = 0; fontPaths[i]; i++) {
        m_font = TTF_OpenFont(fontPaths[i], 13);
        if (m_font) break;
    }
    if (!m_font)
        std::cerr << "Aviso: fonte nao carregada.\n";

    return true;
}

// ─────────────────────────────────────────────────────────────
// refreshHistogram — atualiza exibição após equalizar/reverter
// ─────────────────────────────────────────────────────────────
void HistogramWindow::refreshHistogram() {
    // Redesenha no próximo render() — m_data já foi atualizado
    // Também notifica a janela principal para trocar a textura
    if (m_app) {
        m_app->updateMainTexture(m_data->getCurrentSurface());
    }
}

// ─────────────────────────────────────────────────────────────
// handleEvent — processa clique no botão equalizar/reverter
// ─────────────────────────────────────────────────────────────
void HistogramWindow::handleEvent(const SDL_Event& e) {
    if (!m_window) return;
    SDL_WindowID myID = SDL_GetWindowID(m_window);

    // ── Hover ────────────────────────────────
    if (e.type == SDL_EVENT_MOUSE_MOTION && e.motion.windowID == myID) {
        float mx = e.motion.x, my = e.motion.y;
        bool over = mx >= m_btnRect.x && mx <= m_btnRect.x + m_btnRect.w &&
                    my >= m_btnRect.y && my <= m_btnRect.y + m_btnRect.h;
        if (over && m_btnState == BtnState::Normal)  m_btnState = BtnState::Hover;
        if (!over && m_btnState == BtnState::Hover)  m_btnState = BtnState::Normal;
    }

    // ── Pressionar ───────────────────────────
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        e.button.windowID == myID && e.button.button == SDL_BUTTON_LEFT) {
        float mx = e.button.x, my = e.button.y;
        if (mx >= m_btnRect.x && mx <= m_btnRect.x + m_btnRect.w &&
            my >= m_btnRect.y && my <= m_btnRect.y + m_btnRect.h) {
            m_btnState = BtnState::Pressed;
        }
    }

    // ── Soltar — executar ação ────────────────
    if (e.type == SDL_EVENT_MOUSE_BUTTON_UP &&
        e.button.windowID == myID && e.button.button == SDL_BUTTON_LEFT) {
        if (m_btnState == BtnState::Pressed) {
            m_btnState = BtnState::Normal;

            // Alternar entre equalizar e reverter
            if (m_data->isEqualized()) {
                m_data->revertToOriginal();
                std::cout << "Botao: revertendo para imagem original." << std::endl;
            } else {
                m_data->equalize();
                std::cout << "Botao: equalizando histograma." << std::endl;
            }

            // Atualizar textura na janela principal
            refreshHistogram();
        }
    }
}

// ─────────────────────────────────────────────────────────────
// render
// ─────────────────────────────────────────────────────────────
void HistogramWindow::render() {
    if (!m_renderer) return;

    SDL_SetRenderDrawColor(m_renderer, 18, 18, 22, 255);
    SDL_RenderClear(m_renderer);

    drawHistogram();
    drawAnalysisInfo();
    drawButton();

    SDL_RenderPresent(m_renderer);
}

// ─────────────────────────────────────────────────────────────
// drawText
// ─────────────────────────────────────────────────────────────
void HistogramWindow::drawText(const std::string& text, int x, int y,
                               SDL_Color color) {
    if (!m_font || !m_renderer || text.empty()) return;

    SDL_Surface* surf = TTF_RenderText_Blended(
        m_font, text.c_str(), text.size(), color);
    if (!surf) return;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(m_renderer, surf);
    SDL_DestroySurface(surf);
    if (!tex) return;

    float tw, th;
    SDL_GetTextureSize(tex, &tw, &th);
    SDL_FRect dst = { (float)x, (float)y, tw, th };
    SDL_RenderTexture(m_renderer, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}

// ─────────────────────────────────────────────────────────────
// drawHistogram — barras + grid + linha da média + rótulos eixo X
// ─────────────────────────────────────────────────────────────
void HistogramWindow::drawHistogram() {
    const int* hist = m_data->getHistogram();

    SDL_Color white = {220, 220, 225, 255};
    std::string title = m_data->isEqualized()
                        ? "Histograma (equalizado)"
                        : "Histograma";
    drawText(title, PADDING, PADDING, white);

    int histW = m_w - 2 * PADDING;
    SDL_FRect bgRect = { (float)PADDING, (float)HIST_TOP,
                         (float)histW,   (float)HIST_HEIGHT };
    SDL_SetRenderDrawColor(m_renderer, 28, 28, 35, 255);
    SDL_RenderFillRect(m_renderer, &bgRect);

    int maxVal = 1;
    for (int i = 0; i < 256; i++)
        if (hist[i] > maxVal) maxVal = hist[i];

    // Grid 25/50/75%
    SDL_SetRenderDrawColor(m_renderer, 50, 50, 62, 255);
    for (int pct : {25, 50, 75}) {
        float gy = HIST_TOP + HIST_HEIGHT * (1.0f - pct / 100.0f);
        SDL_RenderLine(m_renderer, PADDING, (int)gy, PADDING + histW, (int)gy);
    }

    // Barras
    float barW = (float)histW / 256.0f;
    for (int i = 0; i < 256; i++) {
        float barH = ((float)hist[i] / maxVal) * HIST_HEIGHT;
        float bx   = PADDING + i * barW;
        float by   = HIST_TOP + HIST_HEIGHT - barH;
        Uint8 shade = (Uint8)i;
        SDL_SetRenderDrawColor(m_renderer, shade, shade, shade, 255);
        SDL_FRect bar = { bx, by, std::max(barW, 1.0f), barH };
        SDL_RenderFillRect(m_renderer, &bar);
    }

    // Linha da média (amarelo)
    float mean  = m_data->getMeanIntensity();
    float meanX = PADDING + (mean / 255.0f) * histW;
    SDL_SetRenderDrawColor(m_renderer, 255, 210, 60, 220);
    SDL_RenderLine(m_renderer, (int)meanX, HIST_TOP, (int)meanX, HIST_BOTTOM);

    // Borda
    SDL_SetRenderDrawColor(m_renderer, 70, 70, 85, 255);
    SDL_RenderRect(m_renderer, &bgRect);

    // Rótulos eixo X
    SDL_Color axisColor = {120, 120, 135, 255};
    const int ticks[] = {0, 64, 128, 192, 255};
    for (int t : ticks) {
        float tx = PADDING + (t / 255.0f) * histW;
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", t);
        int off = (t == 0) ? 0 : (t == 255) ? -16 : -8;
        drawText(buf, (int)tx + off, HIST_BOTTOM + 4, axisColor);
    }

    // Legenda da média
    SDL_Color yellow = {255, 210, 60, 255};
    char meanBuf[32];
    snprintf(meanBuf, sizeof(meanBuf), "media: %.1f", mean);
    drawText(meanBuf, (int)meanX + 4, HIST_TOP + 2, yellow);
}

// ─────────────────────────────────────────────────────────────
// drawAnalysisInfo — classificação de brilho, contraste e estado
// ─────────────────────────────────────────────────────────────
void HistogramWindow::drawAnalysisInfo() {
    float mean   = m_data->getMeanIntensity();
    float stddev = m_data->getStdDeviation();

    std::string brightness;
    SDL_Color   brightColor;
    if (mean > 170.f)      { brightness = "Clara";  brightColor = {100, 220, 120, 255}; }
    else if (mean > 85.f)  { brightness = "Media";  brightColor = {220, 190,  80, 255}; }
    else                   { brightness = "Escura"; brightColor = {180,  80,  80, 255}; }

    std::string contrast;
    SDL_Color   contrastColor;
    if (stddev > 60.f)     { contrast = "Alto";  contrastColor = {100, 190, 255, 255}; }
    else if (stddev > 30.f){ contrast = "Medio"; contrastColor = {220, 190,  80, 255}; }
    else                   { contrast = "Baixo"; contrastColor = {180,  80,  80, 255}; }

    SDL_SetRenderDrawColor(m_renderer, 55, 55, 68, 255);
    SDL_RenderLine(m_renderer, PADDING, DIVIDER_Y, m_w - PADDING, DIVIDER_Y);

    SDL_Color sectionTitle = {180, 180, 195, 255};
    SDL_Color labelColor   = {140, 140, 155, 255};
    SDL_Color valueColor   = {220, 220, 232, 255};

    int y = INFO_TOP;
    drawText("Analise da imagem", PADDING, y, sectionTitle);
    y += LINE_H + 4;

    char buf[64];

    drawText("Brilho (media):", PADDING, y, labelColor);
    snprintf(buf, sizeof(buf), "%.1f", mean);
    drawText(buf, PADDING + 115, y, valueColor);
    drawText("->", PADDING + 165, y, labelColor);
    drawText(brightness, PADDING + 185, y, brightColor);
    y += LINE_H;

    drawText("Contraste (dp):", PADDING, y, labelColor);
    snprintf(buf, sizeof(buf), "%.1f", stddev);
    drawText(buf, PADDING + 115, y, valueColor);
    drawText("->", PADDING + 165, y, labelColor);
    drawText(contrast, PADDING + 185, y, contrastColor);
    y += LINE_H;

    SDL_Color originColor = {110, 110, 125, 255};
    std::string origin = m_data->isGrayscale()
                         ? "Origem: cinza (sem conversao)"
                         : "Origem: colorida -> convertida";
    drawText(origin, PADDING, y, originColor);
    y += LINE_H;

    std::string state = m_data->isEqualized()
                        ? "Estado: equalizada"
                        : "Estado: escala de cinza original";
    SDL_Color stateColor = m_data->isEqualized()
                           ? SDL_Color{100, 200, 255, 255}
                           : SDL_Color{130, 130, 145, 255};
    drawText(state, PADDING, y, stateColor);
}

// ─────────────────────────────────────────────────────────────
// drawButton — botão com 3 estados de cor + texto dinâmico
// ─────────────────────────────────────────────────────────────
void HistogramWindow::drawButton() {
    // Cor de fundo conforme estado
    SDL_Color bg;
    switch (m_btnState) {
        case BtnState::Hover:   bg = {100, 160, 240, 255}; break; // azul claro
        case BtnState::Pressed: bg = { 30,  80, 160, 255}; break; // azul escuro
        default:                bg = { 60, 120, 210, 255}; break; // azul neutro
    }

    // Fundo
    SDL_SetRenderDrawColor(m_renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(m_renderer, &m_btnRect);

    // Borda
    SDL_SetRenderDrawColor(m_renderer, 40, 90, 170, 255);
    SDL_RenderRect(m_renderer, &m_btnRect);

    // Texto dinâmico: "Equalizar" ou "Ver original"
    std::string label = m_data->isEqualized() ? "Ver original" : "Equalizar";
    int textW = (int)label.size() * 7;
    int tx    = (int)(m_btnRect.x + (m_btnRect.w - textW) / 2);
    int ty    = (int)(m_btnRect.y + (m_btnRect.h - 13) / 2);
    drawText(label, tx, ty, {255, 255, 255, 255});
}