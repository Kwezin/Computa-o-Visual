#include "image_data.h"
#include <cstring>
#include <iostream>

/*
 * STUB — esqueleto para compilar.
 *
 * Membro 2: substituir o construtor e isGrayscale()/getGrayscaleSurface()
 *           pela lógica real de detecção + conversão Y=0.2125R+0.7154G+0.0721B
 *
 * Membro 3: implementar getHistogram(), getMeanIntensity(), getStdDeviation()
 *
 * Membro 4: implementar equalize(), revertToOriginal(), getEqualizedSurface()
 */

ImageData::ImageData(SDL_Surface* rawSurface) {
    m_original = rawSurface;
    // TODO (Membro 2): detectar se é cinza e converter
    // Por enquanto, assume que a superfície já está convertida para RGBA
    m_gray = rawSurface; // substituir pela conversão real
    std::memset(m_histogram, 0, sizeof(m_histogram));
}

ImageData::~ImageData() {
    // Liberar superfícies (cuidado para não liberar duas vezes se m_gray == m_original)
    if (m_equalized) { SDL_DestroySurface(m_equalized); }
    if (m_gray && m_gray != m_original) { SDL_DestroySurface(m_gray); }
    if (m_original) { SDL_DestroySurface(m_original); }
}

bool ImageData::isGrayscale() const {
    // TODO (Membro 2)
    return m_wasGray;
}

SDL_Surface* ImageData::getGrayscaleSurface() const {
    return m_gray;
}

const int* ImageData::getHistogram() const {
    return m_histogram;
}

float ImageData::getMeanIntensity() const {
    return m_mean;
}

float ImageData::getStdDeviation() const {
    return m_stddev;
}

SDL_Surface* ImageData::getEqualizedSurface() const {
    return m_equalized;
}

bool ImageData::isEqualized() const {
    return m_isEq;
}

void ImageData::equalize() {
    // TODO (Membro 4)
    m_isEq = true;
}

void ImageData::revertToOriginal() {
    // TODO (Membro 4)
    m_isEq = false;
}

SDL_Surface* ImageData::getCurrentSurface() const {
    if (m_isEq && m_equalized) return m_equalized;
    return m_gray;
}
