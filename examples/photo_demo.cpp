#include "board_config.h"
#include "ILI9341.h"
#include "Sprite.h"
#include <exception>
#include <iostream>

static void circle(ILI9341& tft, int cx, int cy, int r, uint16_t color) {
    int x = r, y = 0, error = 1 - r;
    while (x >= y) {
        tft.drawPixel(cx + x, cy + y, color); tft.drawPixel(cx + y, cy + x, color);
        tft.drawPixel(cx - y, cy + x, color); tft.drawPixel(cx - x, cy + y, color);
        tft.drawPixel(cx - x, cy - y, color); tft.drawPixel(cx - y, cy - x, color);
        tft.drawPixel(cx + y, cy - x, color); tft.drawPixel(cx + x, cy - y, color);
        ++y;
        if (error < 0) error += 2 * y + 1;
        else { --x; error += 2 * (y - x) + 1; }
    }
}

int main() {
    try {
        ILI9341 tft(board::spi, board::gpiochip, board::dc, board::reset, board::hz, board::cs);
        tft.init();
        tft.setRotation(board::rotation);
        const uint16_t bg = 0x0842, white = 0xFFFF, cyan = 0x07FF;
        tft.fillScreen(bg);

        tft.fillRect(12, 10, 5, 44, cyan);
        tft.drawString(25, 11, "ILI9341", white, bg, 3);
        tft.drawString(25, 42, "LUCKFOX PICO MINI B", cyan, bg);
        tft.drawFastHLine(12, 63, 216, 0x39E7);

        tft.fillRect(19, 80, 45, 42, 0xF800);
        circle(tft, 118, 101, 23, 0x07E0);
        tft.drawLine(179, 122, 202, 78, 0xFFE0);
        tft.drawLine(202, 78, 225, 122, 0xFFE0);
        tft.drawLine(225, 122, 179, 122, 0xFFE0);
        tft.drawString(27, 131, "RECT", white, bg);
        tft.drawString(100, 131, "CIRCLE", white, bg);
        tft.drawString(191, 131, "TRI", white, bg);
        tft.drawFastHLine(12, 151, 216, 0x39E7);

        tft.drawString(16, 162, "Aa Bb Cc", white, bg, 2);
        tft.drawString(16, 187, "0123456789", cyan, bg, 2);
        tft.drawString(16, 212, "!@#$%&*?", 0xFFE0, bg, 2);
        tft.drawString(16, 240, "The quick fox 123", white, bg);

        Sprite footer(216, 50);
        footer.fillSprite(0x2104);
        footer.fillRect(0, 0, 6, 50, cyan);
        footer.pushSprite(tft, 12, 260);
        tft.drawString(28, 270, "RGB565  /  40 MHz", white, 0x2104);
        tft.drawString(28, 288, "SPI + GPIO  *  C++", 0x07E0, 0x2104);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
