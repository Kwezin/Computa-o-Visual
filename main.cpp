/*
 * Projeto 1 - Processamento de Imagens
 * Universidade Presbiteriana Mackenzie - Computação Visual
 *
 * Integrantes:
 *   - [Nome 1] - RA: [XXXXXXX]
 *   - [Nome 2] - RA: [XXXXXXX]
 *   - [Nome 3] - RA: [XXXXXXX]
 *   - [Nome 4] - RA: [XXXXXXX]
 */

#include "app.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <caminho_da_imagem>" << std::endl;
        return 1;
    }

    App app;
    if (!app.init(argv[1])) {
        return 1;
    }

    app.run();
    app.shutdown();

    return 0;
}
