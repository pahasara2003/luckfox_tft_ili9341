#include "ILI9341.h"
#include <chrono>
#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) return 2;
    try {
        ILI9341 tft("/dev/spidev0.0", "/dev/gpiochip1", 20, -1, std::stoul(argv[1]), 25);
        tft.init();
        tft.setRotation(2);
        const auto measure = [](const char* name, const std::chrono::steady_clock::time_point& start) {
            std::cout << name << "_us=" << std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start).count() << '\n';
        };
        auto start = std::chrono::steady_clock::now();
        tft.fillScreen(0x001F);
        measure("fill", start);
        std::cout << "pixel=" << std::hex << tft.readPixel(200, 300) << std::dec << '\n';
        start = std::chrono::steady_clock::now();
        for (int i = 0; i < 100; ++i) tft.drawPixel(i, 200, 0xFFFF);
        measure("pixels100", start);
        start = std::chrono::steady_clock::now();
        tft.drawString(0, 0, "Luckfox Pico Mini B", 0xFFFF, 0x001F, 2);
        measure("text", start);
        start = std::chrono::steady_clock::now();
        tft.drawLine(0, 0, 239, 319, 0xF800);
        measure("diagonal", start);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
