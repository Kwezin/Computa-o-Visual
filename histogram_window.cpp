#include "histogram_window.h"
#include "image_data.h"
#include <iostream>

// ─────────────────────────────────────────────────────────────
// Constantes de layout da janela secundária
// ─────────────────────────────────────────────────────────────
static constexpr int PADDING        = 16;
static constexpr int HIST_TOP       = 50;   // y onde o histograma começa
static constexpr int HIST_HEIGHT    = 200;  // altura da área do histograma
static constexpr int INFO_TOP       = HIST_TOP + HIST_HEIGHT + 20;
static constexpr int BTN_H          = 40;
static constexpr int BTN_MARGIN     = 16;

// ─────────────────────────────────────────────────────────────
// Construtor / Destrutor
// ─────────────────────────────────────────────────────────────
HistogramWindow::HistogramWindow(SDL_Window* parent, int x, int y,
                                 int w, int h, ImageData* data)
    : m_parent(parent), m_x(x), m_y(y), m_w(w), m_h(h), m_data(data)
{
    // Calcular rect do botão (canto inferior, largura menos padding)
    m_btnRect = {
        (float)BTN_MARGIN,
        (float)(h - BTN_H - BTN_MARGIN),
        (float)(w - 2 * BTN_MARGIN),
        (float)BTN_H
    };
}

HistogramWindow::~HistogramWindow() {
    if (m_font)     { TTF_CloseFont(m_font);        m_font     = nullptr; }
    if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window)   { SDL_DestroyWindow(m_window);  m_window   = nullptr; }
    TTF_Quit();
}

// ─────────────────────────────────────────────────────────────
// init — cria a janela secundária posicionada ao lado da principal
// ─────────────────────────────────────────────────────────────
bool HistogramWindow::init() {
    // Janela utilitária (sem barra de tarefas própria), filha da principal
    m_window = SDL_CreateWindow("Histograma", m_w, m_h,
                                SDL_WINDOW_UTILITY | SDL_WINDOW_NOT_RESIZABLE);
    if (!m_window) {
        std::cerr << "HistogramWindow: SDL_CreateWindow falhou: "
                  << SDL_GetError() << std::endl;
        return false;
    }

    // Vincular à janela principal (janela filha)
    SDL_SetWindowParent(m_window, m_parent);

    // Posicionar ao lado da janela principal
    SDL_SetWindowPosition(m_window, m_x, m_y);

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        std::cerr << "HistogramWindow: SDL_CreateRenderer falhou: "
                  << SDL_GetError() << std::endl;
        return false;
    }

    // Inicializar SDL_ttf para exibir textos
    if (!TTF_Init()) {
        std::cerr << "TTF_Init falhou: " << SDL_GetError() << std::endl;
        return false;
    }

    // Tentar carregar uma fonte do sistema; ajuste o caminho se necessário
    const char* fontPaths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/System/Library/Fonts/Helvetica.ttc",               // macOS
        "C:/Windows/Fonts/arial.ttf",                        // Windows
        nullptr
    };

    for (int i = 0; fontPaths[i] != nullptr; i++) {
        m_font = TTF_OpenFont(fontPaths[i], 13);
        if (m_font) break;
    }

    if (!m_font) {
        std::cerr << "Aviso: não foi possível carregar fonte. "
                     "Textos não serão exibidos." << std::endl;
        // Não é fatal — o histograma ainda é desenhado
    }

    return true;
}

// ─────────────────────────────────────────────────────────────
// Auxiliar: desenhar texto simples
// ─────────────────────────────────────────────────────────────
void HistogramWindow::drawText(const std::string& text, int x, int y,
                               SDL_Color color) {
    if (!m_font || !m_renderer) return;

    SDL_Surface* surf = TTF_RenderText_Blended(m_font, text.c_str(),
                                               text.size(), color);
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
// handleEvent — eventos desta janela (botão: implementado pelo Membro 4)
// ─────────────────────────────────────────────────────────────
void HistogramWindow::handleEvent(const SDL_Event& e) {
    if (!m_window) return;

    SDL_WindowID myID = SDL_GetWindowID(m_window);

    if (e.type == SDL_EVENT_MOUSE_MOTION &&
        e.motion.windowID == myID) {
        float mx = e.motion.x;
        float my = e.motion.y;
        bool over = mx >= m_btnRect.x && mx <= m_btnRect.x + m_btnRect.w &&
                    my >= m_btnRect.y && my <= m_btnRect.y + m_btnRect.h;

        if (over && m_btnState == BtnState::Normal)
            m_btnState = BtnState::Hover;
        else if (!over && m_btnState == BtnState::Hover)
            m_btnState = BtnState::Normal;
    }

    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        e.button.windowID == myID &&
        e.button.button == SDL_BUTTON_LEFT) {
        float mx = e.button.x;
        float my = e.button.y;
        if (mx >= m_btnRect.x && mx <= m_btnRect.x + m_btnRect.w &&
            my >= m_btnRect.y && my <= m_btnRect.y + m_btnRect.h) {
            m_btnState = BtnState::Pressed;
        }
    }

    if (e.type == SDL_EVENT_MOUSE_BUTTON_UP &&
        e.button.windowID == myID &&
        e.button.button == SDL_BUTTON_LEFT) {
        if (m_btnState == BtnState::Pressed) {
            // TODO (Membro 4): chamar m_data->equalize() ou revertToOriginal()
            //                   e notificar o App para atualizar a textura principal
            m_btnState = BtnState::Normal;
        }
    }
}

// ─────────────────────────────────────────────────────────────
// render — desenha toda a janela secundária
// ─────────────────────────────────────────────────────────────
void HistogramWindow::render() {
    if (!m_renderer) return;

    // Fundo escuro
    SDL_SetRenderDrawColor(m_renderer, 20, 20, 25, 255);
    SDL_RenderClear(m_renderer);

    drawHistogram();
    drawAnalysisInfo();
    drawButton();

    SDL_RenderPresent(m_renderer);
}

// ─────────────────────────────────────────────────────────────
// drawHistogram — barras proporcionais ao valor máximo do histograma
// Implementação inicial do Membro 2 (Membro 3 refina com análise)
// ─────────────────────────────────────────────────────────────
void HistogramWindow::drawHistogram() {
    const int* hist = m_data->getHistogram();

    // Título
    SDL_Color white = {220, 220, 220, 255};
    drawText("Histograma", PADDING, PADDING, white);

    // Encontrar o valor máximo para normalizar as barras
    int maxVal = 1;
    for (int i = 0; i < 256; i++)
        if (hist[i] > maxVal) maxVal = hist[i];

    // Área do histograma
    int histW = m_w - 2 * PADDING;
    float barW = (float)histW / 256.0f;

    // Fundo da área
    SDL_SetRenderDrawColor(m_renderer, 35, 35, 42, 255);
    SDL_FRect bgRect = {(float)PADDING, (float)HIST_TOP,
                        (float)histW, (float)HIST_HEIGHT};
    SDL_RenderFillRect(m_renderer, &bgRect);

    // Desenhar barras
    for (int i = 0; i < 256; i++) {
        float barH = ((float)hist[i] / maxVal) * HIST_HEIGHT;
        float bx   = PADDING + i * barW;
        float by   = HIST_TOP + HIST_HEIGHT - barH;

        // Cor da barra: gradiente de preto a branco conforme intensidade
        Uint8 shade = (Uint8)i;
        SDL_SetRenderDrawColor(m_renderer, shade, shade, shade, 255);

        SDL_FRect bar = {bx, by, std::max(barW, 1.0f), barH};
        SDL_RenderFillRect(m_renderer, &bar);
    }

    // Borda ao redor da área do histograma
    SDL_SetRenderDrawColor(m_renderer, 80, 80, 90, 255);
    SDL_RenderRect(m_renderer, &bgRect);

    // Eixo X: rótulos 0, 128, 255
    SDL_Color gray = {140, 140, 140, 255};
    drawText("0",   PADDING,               HIST_TOP + HIST_HEIGHT + 4, gray);
    drawText("128", PADDING + histW/2 - 8, HIST_TOP + HIST_HEIGHT + 4, gray);
    drawText("255", PADDING + histW - 20,  HIST_TOP + HIST_HEIGHT + 4, gray);
}

// ─────────────────────────────────────────────────────────────
// drawAnalysisInfo — média e desvio padrão
// Lógica de classificação implementada pelo Membro 3
// ─────────────────────────────────────────────────────────────
void HistogramWindow::drawAnalysisInfo() {
    float mean   = m_data->getMeanIntensity();
    float stddev = m_data->getStdDeviation();

    // Classificação de brilho
    std::string brightness;
    if      (mean > 170) brightness = "Clara";
    else if (mean > 85)  brightness = "Media";
    else                 brightness = "Escura";

    // Classificação de contraste
    std::string contrast;
    if      (stddev > 60) contrast = "Alto";
    else if (stddev > 30) contrast = "Medio";
    else                  contrast = "Baixo";

    SDL_Color labelColor = {160, 160, 170, 255};
    SDL_Color valueColor = {230, 230, 240, 255};

    char buf[64];
    int y = INFO_TOP;
    int lineH = 22;

    drawText("Analise", PADDING, y, {200, 200, 210, 255});
    y += lineH + 4;

    snprintf(buf, sizeof(buf), "Media: %.1f  ->  %s", mean, brightness.c_str());
    drawText(buf, PADDING, y, valueColor);
    y += lineH;

    snprintf(buf, sizeof(buf), "Desvio: %.1f  ->  Contraste %s", stddev, contrast.c_str());
    drawText(buf, PADDING, y, valueColor);
    y += lineH;

    // Indicar se a imagem original era colorida ou cinza
    std::string srcInfo = m_data->isGrayscale()
                          ? "Origem: ja em escala de cinza"
                          : "Origem: imagem colorida (convertida)";
    drawText(srcInfo, PADDING, y, labelColor);
}

// ─────────────────────────────────────────────────────────────
// drawButton — botão Equalizar / Ver original
// Estados: Normal (azul), Hover (azul claro), Pressed (azul escuro)
// Membro 4 conecta o clique à lógica de equalização
// ─────────────────────────────────────────────────────────────
void HistogramWindow::drawButton() {
    // Cor conforme estado
    SDL_Color bg;
    switch (m_btnState) {
        case BtnState::Hover:   bg = {100, 160, 240, 255}; break; // azul claro
        case BtnState::Pressed: bg = { 30,  80, 160, 255}; break; // azul escuro
        default:                bg = { 60, 120, 210, 255}; break; // azul neutro
    }

    // Fundo do botão
    SDL_SetRenderDrawColor(m_renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(m_renderer, &m_btnRect);

    // Borda do botão
    SDL_SetRenderDrawColor(m_renderer, 40, 90, 170, 255);
    SDL_RenderRect(m_renderer, &m_btnRect);

    // Texto centralizado
    std::string label = m_data->isEqualized() ? "Ver original" : "Equalizar";
    SDL_Color textColor = {255, 255, 255, 255};

    // Estimativa de largura do texto para centralizar (~7px por char)
    int textW = (int)label.size() * 7;
    int tx = (int)(m_btnRect.x + (m_btnRect.w - textW) / 2);
    int ty = (int)(m_btnRect.y + (m_btnRect.h - 13) / 2);
    drawText(label, tx, ty, textColor);
}