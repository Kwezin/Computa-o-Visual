#include "image_loader.h"
#include <iostream>
#include <filesystem>

SDL_Surface* ImageLoader::load(const std::string& path) {
    // ── 1. Verificar se o arquivo existe ─────
    if (!std::filesystem::exists(path)) {
        std::cerr << "Erro: arquivo não encontrado: " << path << std::endl;
        return nullptr;
    }

    // ── 2. Inicializar SDL_image ──────────────
    // IMG_Init retorna as flags que foram inicializadas com sucesso
    int flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(flags) & flags) != flags) {
        std::cerr << "IMG_Init falhou: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    // ── 3. Carregar a imagem ──────────────────
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Erro ao carregar imagem '" << path << "': "
                  << SDL_GetError() << std::endl;
        return nullptr;
    }

    // ── 4. Verificar se é um formato de imagem válido ─
    // (IMG_Load já faz isso, mas registramos dimensões para debug)
    if (surface->w <= 0 || surface->h <= 0) {
        std::cerr << "Erro: imagem com dimensões inválidas." << std::endl;
        SDL_DestroySurface(surface);
        return nullptr;
    }

    std::cout << "Imagem carregada: " << path
              << " (" << surface->w << "x" << surface->h << ")" << std::endl;

    // ── 5. Converter para RGBA8888 para facilitar acesso aos pixels ─
    SDL_Surface* converted = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA8888);
    SDL_DestroySurface(surface);

    if (!converted) {
        std::cerr << "Erro ao converter formato de pixel: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    return converted;
}
