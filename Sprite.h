#pragma once

#include "ILI9341.h"
#include <vector>

class Sprite {
public:
    Sprite(int16_t width, int16_t height);
    int16_t width() const { return width_; }
    int16_t height() const { return height_; }
    void fillSprite(uint16_t color);
    void drawPixel(int32_t x, int32_t y, uint16_t color);
    void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color);
    uint16_t readPixel(int32_t x, int32_t y) const;
    void pushSprite(ILI9341& display, int32_t x, int32_t y) const;
private:
    int16_t width_, height_;
    std::vector<uint16_t> pixels_;
};
