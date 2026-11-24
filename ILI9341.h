#pragma once

#include <cstdint>
#include <string>

class ILI9341 {
public:
    ILI9341(const std::string& spi, const std::string& gpiochip, unsigned dc,
            int reset = -1, uint32_t hz = 16000000, int cs = -1);
    ~ILI9341();
    ILI9341(const ILI9341&) = delete;
    ILI9341& operator=(const ILI9341&) = delete;

    void init();
    void setRotation(unsigned rotation);
    int16_t width() const { return width_; }
    int16_t height() const { return height_; }
    void drawPixel(int32_t x, int32_t y, uint16_t color);
    void drawFastHLine(int32_t x, int32_t y, int32_t w, uint16_t color);
    void drawFastVLine(int32_t x, int32_t y, int32_t h, uint16_t color);
    void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color);
    void fillScreen(uint16_t color) { fillRect(0, 0, width_, height_, color); }
    void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint16_t color);
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t* pixels);
    uint16_t readPixel(int32_t x, int32_t y);
    void drawChar(int32_t x, int32_t y, char c, uint16_t color, uint16_t background, unsigned scale = 1);
    void drawString(int32_t x, int32_t y, const std::string& text, uint16_t color,
                    uint16_t background, unsigned scale = 1);
    static constexpr uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
        return uint16_t((r & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3);
    }

private:
    void command(uint8_t value, const uint8_t* data = nullptr, unsigned size = 0);
    void data(const uint8_t* bytes, unsigned size);
    void window(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void gpio(int fd, uint8_t value);
    std::string spi_path_, gpiochip_path_;
    unsigned dc_line_;
    int reset_line_, cs_line_;
    uint32_t hz_;
    int spi_ = -1, dc_ = -1, reset_ = -1, cs_ = -1;
    int16_t width_ = 240, height_ = 320;
};
