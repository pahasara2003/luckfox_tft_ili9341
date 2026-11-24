#include "ILI9341.h"
#include "Sprite.h"
#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 6 && argc != 7) {
        std::cerr << "usage: " << argv[0] << " /dev/spidev0.0 /dev/gpiochip1 DC_LINE RESET_LINE CS_LINE [SPI_HZ]\n";
        return 2;
    }
    try {
        ILI9341 tft(argv[1], argv[2], std::stoul(argv[3]), std::stoi(argv[4]),
                    argc == 7 ? std::stoul(argv[6]) : 16000000, std::stoi(argv[5]));
        tft.init();
        tft.setRotation(2);
        tft.fillScreen(ILI9341::color565(0, 0, 0));
        tft.fillRect(20, 20, 200, 60, ILI9341::color565(255, 0, 0));
        tft.drawLine(0, 0, tft.width() - 1, tft.height() - 1,
                     ILI9341::color565(255, 255, 255));
        tft.drawString(30, 100, "Luckfox Pico Mini B", 0xFFFF, 0, 2);
        Sprite sprite(40, 40);
        sprite.fillSprite(0x001F);
        sprite.fillRect(10, 10, 20, 20, 0xFFE0);
        sprite.pushSprite(tft, 100, 170);
        if (std::stoi(argv[5]) >= 0)
            std::cout << "pixel(105,175): 0x" << std::hex << tft.readPixel(105, 175) << '\n';
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
