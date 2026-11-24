#include "board_config.h"
#include "ILI9341.h"
#include <exception>
#include <iostream>

int main() {
    try {
        ILI9341 tft(board::spi, board::gpiochip, board::dc, board::reset, board::hz, board::cs);
        tft.init();
        tft.setRotation(board::rotation);
        tft.fillScreen(0x0000);
        tft.fillRect(20, 30, 200, 60, 0xF800);   // Red rectangle
        tft.fillRect(20, 110, 200, 60, 0x07E0);  // Green rectangle
        tft.drawLine(20, 190, 219, 270, 0xFFFF);
        tft.drawString(30, 280, "Hello, ILI9341!", 0xFFFF, 0x0000, 1);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
