#include "app.h"
#include "image_loader.h"
#include "image_data.h"
#include "histogram_window.h"
#include <iostream>

// ─────────────────────────────────────────────
// Constantes
// ─────────────────────────────────────────────
static constexpr int SECONDARY_WIN_W = 360;
static constexpr int SECONDARY_WIN_H = 480;
static constexpr int WIN_GAP         = 10;   // espaço entre janela principal e secundária

// ─────────────────────────────────────────────
App::App() = default;
App::~App() = default;

// ─────────────────────────────────────────────
bool App::init(const std::string& imagePath) {
    m_imagePath = imagePath;

    // ── 1. Inicializar SDL ───────────────────
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << std::endl;
        return false;
    }

    // ── 2. Carregar imagem ───────────────────
    SDL_Surface* rawSurface = ImageLoader::load(imagePath);
    if (!rawSurface) {
        SDL_Quit();
        return false;
    }

    m_imgW = rawSurface->w;
    m_imgH = rawSurface->h;

    // ── 3. Processar imagem (Membro 2) ───────
    m_imageData = new ImageData(rawSurface);
    // rawSurface é gerenciado pelo ImageData a partir daqui
    // (ou liberado internamente — ver image_data.cpp)

    // ── 4. Criar janela principal ─────────────
    // Posição inicial: centralizada no monitor principal
    SDL_DisplayID displayID = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* dm = SDL_GetCurrentDisplayMode(displayID);

    int screenW = dm ? dm->w : 1920;
    int screenH = dm ? dm->h : 1080;

    int winX = (screenW - m_imgW) / 2;
    int winY = (screenH - m_imgH) / 2;

    m_mainWindow = SDL_CreateWindow(
        "Proj1 - Processamento de Imagens",
        m_imgW, m_imgH,
        SDL_WINDOW_RESIZABLE
    );
    if (!m_mainWindow) {
        std::cerr << "SDL_CreateWindow falhou: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }
    SDL_SetWindowPosition(m_mainWindow, winX, winY);

    m_mainRenderer = SDL_CreateRenderer(m_mainWindow, nullptr);
    if (!m_mainRenderer) {
        std::cerr << "SDL_CreateRenderer falhou: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(m_mainWindow);
        SDL_Quit();
        return false;
    }

    // ── 5. Criar textura inicial (imagem em escala de cinza) ─
    SDL_Surface* graySurface = m_imageData->getGrayscaleSurface();
    m_mainTexture = SDL_CreateTextureFromSurface(m_mainRenderer, graySurface);
    if (!m_mainTexture) {
        std::cerr << "SDL_CreateTextureFromSurface falhou: " << SDL_GetError() << std::endl;
        return false;
    }

    // ── 6. Criar janela secundária (Membro 2) ─
    m_histWindow = new HistogramWindow(
        m_mainWindow,
        winX + m_imgW + WIN_GAP,
        winY,
        SECONDARY_WIN_W,
        SECONDARY_WIN_H,
        m_imageData
    );
    if (!m_histWindow->init()) {
        std::cerr << "Falha ao criar janela secundária." << std::endl;
        return false;
    }

    m_running = true;
    return true;
}

// ─────────────────────────────────────────────
void App::run() {
    while (m_running) {
        handleEvents();
        render();
        if (m_histWindow) m_histWindow->render();
        SDL_Delay(16); // ~60 fps
    }
}

// ─────────────────────────────────────────────
void App::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_EVENT_QUIT:
                m_running = false;
                break;

            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                // Fechar qualquer janela encerra o programa
                m_running = false;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (e.key.key == SDLK_S) {
                    saveImage();
                }
                break;

            default:
                // Repassar eventos do mouse para a janela secundária
                if (m_histWindow) m_histWindow->handleEvent(e);
                break;
        }
    }
}

// ─────────────────────────────────────────────
void App::render() {
    SDL_SetRenderDrawColor(m_mainRenderer, 30, 30, 30, 255);
    SDL_RenderClear(m_mainRenderer);

    if (m_mainTexture) {
        SDL_FRect dst = { 0.f, 0.f, (float)m_imgW, (float)m_imgH };
        SDL_RenderTexture(m_mainRenderer, m_mainTexture, nullptr, &dst);
    }

    SDL_RenderPresent(m_mainRenderer);
}

// ─────────────────────────────────────────────
void App::saveImage() {
    // Salva a superfície atualmente exibida (original cinza ou equalizada)
    SDL_Surface* current = m_imageData->getCurrentSurface();
    if (!current) {
        std::cerr << "Nenhuma imagem para salvar." << std::endl;
        return;
    }

    const char* outPath = "output_image.png";
    if (IMG_SavePNG(current, outPath)) {
        std::cout << "Imagem salva em: " << outPath << std::endl;
    } else {
        std::cerr << "Erro ao salvar imagem: " << SDL_GetError() << std::endl;
    }
}

// ─────────────────────────────────────────────
void App::updateMainTexture(SDL_Surface* surface) {
    if (!surface || !m_mainRenderer) return;

    if (m_mainTexture) {
        SDL_DestroyTexture(m_mainTexture);
        m_mainTexture = nullptr;
    }
    m_mainTexture = SDL_CreateTextureFromSurface(m_mainRenderer, surface);
}

// ─────────────────────────────────────────────
void App::shutdown() {
    delete m_histWindow;
    m_histWindow = nullptr;

    delete m_imageData;
    m_imageData = nullptr;

    if (m_mainTexture)  { SDL_DestroyTexture(m_mainTexture);   m_mainTexture  = nullptr; }
    if (m_mainRenderer) { SDL_DestroyRenderer(m_mainRenderer); m_mainRenderer = nullptr; }
    if (m_mainWindow)   { SDL_DestroyWindow(m_mainWindow);     m_mainWindow   = nullptr; }

    IMG_Quit();
    SDL_Quit();
}
