#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>

class ImageData;
class HistogramWindow;

class App {
public:
    App();
    ~App();

    bool init(const std::string& imagePath);
    void run();
    void shutdown();

    // Chamado pela HistogramWindow (Membro 4) para trocar a textura exibida
    void updateMainTexture(SDL_Surface* surface);

    SDL_Window*   getMainWindow()   const { return m_mainWindow; }
    SDL_Renderer* getMainRenderer() const { return m_mainRenderer; }

private:
    void handleEvents();
    void render();
    void saveImage();

    SDL_Window*      m_mainWindow   = nullptr;
    SDL_Renderer*    m_mainRenderer = nullptr;
    SDL_Texture*     m_mainTexture  = nullptr;
    HistogramWindow* m_histWindow   = nullptr;
    ImageData*       m_imageData    = nullptr;

    std::string m_imagePath;
    bool        m_running = false;
    int         m_imgW    = 0;
    int         m_imgH    = 0;
};