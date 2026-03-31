#include "histogram_window.h"
#include "image_data.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdio>

// ─────────────────────────────────────────────────────────────
// Constantes de layout
// ─────────────────────────────────────────────────────────────
static constexpr int PADDING        = 16;
static constexpr int TITLE_H        = 28;

static constexpr int HIST_TOP       = PADDING + TITLE_H;
static constexpr int HIST_HEIGHT    = 180;
static constexpr int HIST_BOTTOM    = HIST_TOP + HIST_HEIGHT;

static constexpr int AXIS_LABEL_H   = 18;   // altura da linha de rótulos do eixo X

static constexpr int DIVIDER_Y      = HIST_BOTTOM + AXIS_LABEL_H + 10;

static constexpr int INFO_TOP       = DIVIDER_Y + 12;
static constexpr int LINE_H         = 22;

static constexpr int BTN_H          = 40;
static constexpr int BTN_MARGIN     = 16;

// ─────────────────────────────────────────────────────────────
// Construtor / Destrutor
// ─────────────────────────────────────────────────────────────
HistogramWindow::HistogramWindow(SDL_Window* parent, int x, int y,
                                 int w, int h, ImageData* data)
    : m_parent(parent), m_x(x), m_y(y), m_w(w), m_h(h), m_data(data)
{
    m_btnRect = {
        (float)BTN_MARGIN,
        (float)(h - BTN_H - BTN_MARGIN),
        (float)(w - 2 * BTN_MARGIN),
        (float)BTN_H
    };
}

HistogramWindow::~HistogramWindow() {
    if (m_font)     { TTF_CloseFont(m_font);          m_font     = nullptr; }
    if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window)   { SDL_DestroyWindow(m_window);    m_window   = nullptr; }
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
        std::cerr << "Aviso: fonte nao carregada — textos nao serao exibidos.\n";

    return true;
}

// ─────────────────────────────────────────────────────────────
// refreshHistogram — chamado pelo Membro 4 após equalizar/reverter
// ─────────────────────────────────────────────────────────────
void HistogramWindow::refreshHistogram() {
    // Nada a fazer além de redesenhar no próximo render()
    // (m_data já foi atualizado pelo ImageData internamente)
}

// ─────────────────────────────────────────────────────────────
// handleEvent
// ─────────────────────────────────────────────────────────────
void HistogramWindow::handleEvent(const SDL_Event& e) {
    if (!m_window) return;
    SDL_WindowID myID = SDL_GetWindowID(m_window);

    if (e.type == SDL_EVENT_MOUSE_MOTION && e.motion.windowID == myID) {
        float mx = e.motion.x, my = e.motion.y;
        bool over = mx >= m_btnRect.x && mx <= m_btnRect.x + m_btnRect.w &&
                    my >= m_btnRect.y && my <= m_btnRect.y + m_btnRect.h;
        if (over && m_btnState == BtnState::Normal)  m_btnState = BtnState::Hover;
        if (!over && m_btnState == BtnState::Hover)  m_btnState = BtnState::Normal;
    }

    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        e.button.windowID == myID && e.button.button == SDL_BUTTON_LEFT) {
        float mx = e.button.x, my = e.button.y;
        if (mx >= m_btnRect.x && mx <= m_btnRect.x + m_btnRect.w &&
            my >= m_btnRect.y && my <= m_btnRect.y + m_btnRect.h)
            m_btnState = BtnState::Pressed;
    }

    if (e.type == SDL_EVENT_MOUSE_BUTTON_UP &&
        e.button.windowID == myID && e.button.button == SDL_BUTTON_LEFT) {
        if (m_btnState == BtnState::Pressed) {
            // TODO (Membro 4): chamar equalize() ou revertToOriginal()
            m_btnState = BtnState::Normal;
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
// drawText — helper com alinhamento opcional
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
// drawHistogram
//   • Barras proporcionais ao valor máximo (escala linear)
//   • Linha de média vertical em amarelo
//   • Grid horizontal em 25 %, 50 %, 75 %
//   • Rótulos no eixo X: 0, 64, 128, 192, 255
//   • Título dinâmico: "Histograma" / "Histograma (equalizado)"
// ─────────────────────────────────────────────────────────────
void HistogramWindow::drawHistogram() {
    const int* hist = m_data->getHistogram();

    // ── Título ──────────────────────────────
    SDL_Color white  = {220, 220, 225, 255};
    std::string title = m_data->isEqualized()
                        ? "Histograma (equalizado)"
                        : "Histograma";
    drawText(title, PADDING, PADDING, white);

    // ── Área de fundo ────────────────────────
    int histW = m_w - 2 * PADDING;
    SDL_FRect bgRect = { (float)PADDING, (float)HIST_TOP,
                         (float)histW,   (float)HIST_HEIGHT };
    SDL_SetRenderDrawColor(m_renderer, 28, 28, 35, 255);
    SDL_RenderFillRect(m_renderer, &bgRect);

    // ── Valor máximo para normalizar ─────────
    int maxVal = 1;
    for (int i = 0; i < 256; i++)
        if (hist[i] > maxVal) maxVal = hist[i];

    // ── Grid horizontal (25 %, 50 %, 75 %) ──
    SDL_SetRenderDrawColor(m_renderer, 50, 50, 60, 255);
    for (int pct : {25, 50, 75}) {
        float gy = HIST_TOP + HIST_HEIGHT * (1.0f - pct / 100.0f);
        SDL_RenderLine(m_renderer,
                       PADDING,         (int)gy,
                       PADDING + histW, (int)gy);
    }

    // ── Barras ───────────────────────────────
    float barW = (float)histW / 256.0f;
    for (int i = 0; i < 256; i++) {
        float barH = ((float)hist[i] / maxVal) * HIST_HEIGHT;
        float bx   = PADDING + i * barW;
        float by   = HIST_TOP + HIST_HEIGHT - barH;

        // Cor: tons de cinza proporcional à intensidade i
        Uint8 shade = (Uint8)i;
        SDL_SetRenderDrawColor(m_renderer, shade, shade, shade, 255);

        SDL_FRect bar = { bx, by, std::max(barW, 1.0f), barH };
        SDL_RenderFillRect(m_renderer, &bar);
    }

    // ── Linha da média (amarelo) ─────────────
    float mean   = m_data->getMeanIntensity();
    float meanX  = PADDING + (mean / 255.0f) * histW;
    SDL_SetRenderDrawColor(m_renderer, 255, 210, 60, 220);
    SDL_RenderLine(m_renderer, (int)meanX, HIST_TOP,
                               (int)meanX, HIST_BOTTOM);

    // ── Borda da área ────────────────────────
    SDL_SetRenderDrawColor(m_renderer, 70, 70, 85, 255);
    SDL_RenderRect(m_renderer, &bgRect);

    // ── Rótulos eixo X: 0, 64, 128, 192, 255 ─
    SDL_Color axisColor = {120, 120, 135, 255};
    const int ticks[] = {0, 64, 128, 192, 255};
    for (int t : ticks) {
        float tx = PADDING + (t / 255.0f) * histW;
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", t);
        // Centralizar rótulo sobre o tick
        int labelOffset = (t == 0) ? 0 : (t == 255) ? -16 : -8;
        drawText(buf, (int)tx + labelOffset,
                 HIST_BOTTOM + 4, axisColor);
    }

    // ── Legenda da linha amarela ──────────────
    SDL_Color yellow = {255, 210, 60, 255};
    char meanBuf[32];
    snprintf(meanBuf, sizeof(meanBuf), "media: %.1f", mean);
    drawText(meanBuf, (int)meanX + 4, HIST_TOP + 2, yellow);
}

// ─────────────────────────────────────────────────────────────
// drawAnalysisInfo
//   • Classificação de brilho (clara / média / escura) com badge colorido
//   • Classificação de contraste (alto / médio / baixo) com badge colorido
//   • Origem da imagem (colorida convertida ou já cinza)
//   • Linha divisória acima da seção
// ─────────────────────────────────────────────────────────────
void HistogramWindow::drawAnalysisInfo() {
    float mean   = m_data->getMeanIntensity();
    float stddev = m_data->getStdDeviation();

    // ── Classificação de brilho ──────────────
    // Limites: média < 85 → escura; 85–170 → média; > 170 → clara
    std::string brightness;
    SDL_Color   brightColor;
    if (mean > 170.f) {
        brightness   = "Clara";
        brightColor  = {100, 220, 120, 255};  // verde
    } else if (mean > 85.f) {
        brightness   = "Media";
        brightColor  = {220, 190, 80,  255};  // amarelo
    } else {
        brightness   = "Escura";
        brightColor  = {180,  80,  80, 255};  // vermelho
    }

    // ── Classificação de contraste ───────────
    // Limites: desvio < 30 → baixo; 30–60 → médio; > 60 → alto
    std::string contrast;
    SDL_Color   contrastColor;
    if (stddev > 60.f) {
        contrast      = "Alto";
        contrastColor = {100, 190, 255, 255}; // azul
    } else if (stddev > 30.f) {
        contrast      = "Medio";
        contrastColor = {220, 190, 80,  255}; // amarelo
    } else {
        contrast      = "Baixo";
        contrastColor = {180,  80,  80, 255}; // vermelho
    }

    // ── Linha divisória ──────────────────────
    SDL_SetRenderDrawColor(m_renderer, 55, 55, 68, 255);
    SDL_RenderLine(m_renderer, PADDING, DIVIDER_Y,
                   m_w - PADDING, DIVIDER_Y);

    SDL_Color sectionTitle = {180, 180, 195, 255};
    SDL_Color labelColor   = {140, 140, 155, 255};
    SDL_Color valueColor   = {220, 220, 232, 255};

    int y = INFO_TOP;

    drawText("Analise da imagem", PADDING, y, sectionTitle);
    y += LINE_H + 4;

    // ── Brilho ───────────────────────────────
    char buf[64];
    drawText("Brilho (media):", PADDING, y, labelColor);
    snprintf(buf, sizeof(buf), "%.1f", mean);
    drawText(buf, PADDING + 115, y, valueColor);
    drawText("->", PADDING + 165, y, labelColor);
    drawText(brightness, PADDING + 185, y, brightColor);
    y += LINE_H;

    // ── Contraste ────────────────────────────
    drawText("Contraste (dp):", PADDING, y, labelColor);
    snprintf(buf, sizeof(buf), "%.1f", stddev);
    drawText(buf, PADDING + 115, y, valueColor);
    drawText("->", PADDING + 165, y, labelColor);
    drawText(contrast, PADDING + 185, y, contrastColor);
    y += LINE_H;

    // ── Origem ───────────────────────────────
    SDL_Color originColor = {110, 110, 125, 255};
    std::string origin = m_data->isGrayscale()
                         ? "Origem: cinza (sem conversao)"
                         : "Origem: colorida -> convertida";
    drawText(origin, PADDING, y, originColor);
    y += LINE_H;

    // ── Estado atual ─────────────────────────
    std::string state = m_data->isEqualized()
                        ? "Estado: equalizada"
                        : "Estado: escala de cinza original";
    SDL_Color stateColor = m_data->isEqualized()
                           ? SDL_Color{100, 200, 255, 255}
                           : SDL_Color{130, 130, 145, 255};
    drawText(state, PADDING, y, stateColor);
}

// ─────────────────────────────────────────────────────────────
// drawButton
// ─────────────────────────────────────────────────────────────
void HistogramWindow::drawButton() {
    SDL_Color bg;
    switch (m_btnState) {
        case BtnState::Hover:   bg = {100, 160, 240, 255}; break;
        case BtnState::Pressed: bg = { 30,  80, 160, 255}; break;
        default:                bg = { 60, 120, 210, 255}; break;
    }

    SDL_SetRenderDrawColor(m_renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(m_renderer, &m_btnRect);

    SDL_SetRenderDrawColor(m_renderer, 40, 90, 170, 255);
    SDL_RenderRect(m_renderer, &m_btnRect);

    std::string label = m_data->isEqualized() ? "Ver original" : "Equalizar";
    SDL_Color textColor = {255, 255, 255, 255};

    int textW = (int)label.size() * 7;
    int tx = (int)(m_btnRect.x + (m_btnRect.w - textW) / 2);
    int ty = (int)(m_btnRect.y + (m_btnRect.h - 13) / 2);
    drawText(label, tx, ty, textColor);
}