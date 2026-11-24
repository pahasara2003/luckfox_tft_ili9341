#include "board_config.h"
#include "ILI9341.h"
#include "Sprite.h"
#include <exception>
#include <iostream>

int main() {
    try {
        ILI9341 tft(board::spi, board::gpiochip, board::dc, board::reset, board::hz, board::cs);
        tft.init();
        tft.setRotation(board::rotation);
        tft.fillScreen(0x0000);

        Sprite tile(64, 64);
        tile.fillSprite(0x001F);
        tile.fillRect(8, 8, 48, 48, 0xFFE0);
        tile.fillRect(20, 20, 24, 24, 0xF800);
        tile.pushSprite(tft, 20, 70);
        tile.pushSprite(tft, 88, 70);
        tile.pushSprite(tft, 156, 70);
        tft.drawString(20, 30, "RGB565 SPRITES", 0xFFFF, 0x0000, 2);
        tft.drawString(20, 160, "Draw once in RAM", 0x07E0, 0x0000, 1);
        tft.drawString(20, 180, "Push many times", 0xFFFF, 0x0000, 1);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
