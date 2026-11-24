#include "Sprite.h"
#include <algorithm>
#include <stdexcept>

Sprite::Sprite(int16_t width, int16_t height) : width_(width), height_(height) {
    if (width <= 0 || height <= 0) throw std::invalid_argument("sprite dimensions must be positive");
    pixels_.resize(size_t(width) * height);
}

void Sprite::fillSprite(uint16_t color) {
    std::fill(pixels_.begin(), pixels_.end(), color);
}

void Sprite::drawPixel(int32_t x, int32_t y, uint16_t color) {
    if (x >= 0 && y >= 0 && x < width_ && y < height_)
        pixels_[size_t(y) * width_ + x] = color;
}

void Sprite::fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color) {
    if (w <= 0 || h <= 0) return;
    int32_t left = std::max<int32_t>(0, x), top = std::max<int32_t>(0, y);
    int32_t right = std::min<int64_t>(width_, int64_t(x) + w);
    int32_t bottom = std::min<int64_t>(height_, int64_t(y) + h);
    if (left >= right || top >= bottom) return;
    for (int32_t row = top; row < bottom; ++row)
        std::fill(pixels_.begin() + size_t(row) * width_ + left,
                  pixels_.begin() + size_t(row) * width_ + right, color);
}

uint16_t Sprite::readPixel(int32_t x, int32_t y) const {
    return x >= 0 && y >= 0 && x < width_ && y < height_
        ? pixels_[size_t(y) * width_ + x] : 0;
}

void Sprite::pushSprite(ILI9341& display, int32_t x, int32_t y) const {
    display.pushImage(x, y, width_, height_, pixels_.data());
}
