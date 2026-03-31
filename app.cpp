#include "app.h"
#include "image_loader.h"
#include "image_data.h"
#include "histogram_window.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>

static constexpr int SECONDARY_WIN_W = 360;
static constexpr int SECONDARY_WIN_H = 480;
static constexpr int WIN_GAP         = 10;

App::App() = default;
App::~App() = default;

bool App::init(const std::string& imagePath) {
    m_imagePath = imagePath;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_Surface* rawSurface = ImageLoader::load(imagePath);
    if (!rawSurface) { SDL_Quit(); return false; }

    m_imgW = rawSurface->w;
    m_imgH = rawSurface->h;

    m_imageData = new ImageData(rawSurface);

    // Posição centralizada no monitor principal
    SDL_DisplayID displayID = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* dm = SDL_GetCurrentDisplayMode(displayID);
    int screenW = dm ? dm->w : 1920;
    int screenH = dm ? dm->h : 1080;
    int winX = (screenW - m_imgW) / 2;
    int winY = (screenH - m_imgH) / 2;

    m_mainWindow = SDL_CreateWindow(
        "Proj1 - Processamento de Imagens",
        m_imgW, m_imgH, SDL_WINDOW_RESIZABLE);
    if (!m_mainWindow) {
        std::cerr << "SDL_CreateWindow falhou: " << SDL_GetError() << std::endl;
        SDL_Quit(); return false;
    }
    SDL_SetWindowPosition(m_mainWindow, winX, winY);

    m_mainRenderer = SDL_CreateRenderer(m_mainWindow, nullptr);
    if (!m_mainRenderer) {
        std::cerr << "SDL_CreateRenderer falhou: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(m_mainWindow); SDL_Quit(); return false;
    }

    SDL_Surface* graySurface = m_imageData->getGrayscaleSurface();
    m_mainTexture = SDL_CreateTextureFromSurface(m_mainRenderer, graySurface);
    if (!m_mainTexture) {
        std::cerr << "SDL_CreateTextureFromSurface falhou: " << SDL_GetError() << std::endl;
        return false;
    }

    // Membro 4: passar "this" para que HistogramWindow possa chamar updateMainTexture()
    m_histWindow = new HistogramWindow(
        m_mainWindow,
        winX + m_imgW + WIN_GAP, winY,
        SECONDARY_WIN_W, SECONDARY_WIN_H,
        m_imageData,
        this          // <── novo parâmetro adicionado pelo Membro 4
    );
    if (!m_histWindow->init()) {
        std::cerr << "Falha ao criar janela secundaria." << std::endl;
        return false;
    }

    m_running = true;
    return true;
}

void App::run() {
    while (m_running) {
        handleEvents();
        render();
        if (m_histWindow) m_histWindow->render();
        SDL_Delay(16);
    }
}

void App::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_EVENT_QUIT:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                m_running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (e.key.key == SDLK_S) saveImage();
                break;
            default:
                if (m_histWindow) m_histWindow->handleEvent(e);
                break;
        }
    }
}

void App::render() {
    SDL_SetRenderDrawColor(m_mainRenderer, 30, 30, 30, 255);
    SDL_RenderClear(m_mainRenderer);
    if (m_mainTexture) {
        SDL_FRect dst = { 0.f, 0.f, (float)m_imgW, (float)m_imgH };
        SDL_RenderTexture(m_mainRenderer, m_mainTexture, nullptr, &dst);
    }
    SDL_RenderPresent(m_mainRenderer);
}

void App::saveImage() {
    SDL_Surface* current = m_imageData->getCurrentSurface();
    if (!current) { std::cerr << "Nenhuma imagem para salvar.\n"; return; }

    const char* outPath = "output_image.png";
    if (IMG_SavePNG(current, outPath))
        std::cout << "Imagem salva em: " << outPath << std::endl;
    else
        std::cerr << "Erro ao salvar: " << SDL_GetError() << std::endl;
}

// Chamado pela HistogramWindow após equalizar/reverter
void App::updateMainTexture(SDL_Surface* surface) {
    if (!surface || !m_mainRenderer) return;
    if (m_mainTexture) { SDL_DestroyTexture(m_mainTexture); m_mainTexture = nullptr; }
    m_mainTexture = SDL_CreateTextureFromSurface(m_mainRenderer, surface);
}

void App::shutdown() {
    delete m_histWindow;  m_histWindow  = nullptr;
    delete m_imageData;   m_imageData   = nullptr;
    if (m_mainTexture)  { SDL_DestroyTexture(m_mainTexture);   m_mainTexture  = nullptr; }
    if (m_mainRenderer) { SDL_DestroyRenderer(m_mainRenderer); m_mainRenderer = nullptr; }
    if (m_mainWindow)   { SDL_DestroyWindow(m_mainWindow);     m_mainWindow   = nullptr; }
    SDL_Quit();
}
