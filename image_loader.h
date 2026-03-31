#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>

/*
 * ImageLoader
 * Responsabilidade: carregar um arquivo de imagem do disco usando SDL_image.
 * Trata erros de arquivo não encontrado e formato inválido.
 */
class ImageLoader {
public:
    // Retorna SDL_Surface* em caso de sucesso, nullptr em caso de erro.
    // O chamador é responsável por liberar a superfície com SDL_DestroySurface().
    static SDL_Surface* load(const std::string& path);
};
