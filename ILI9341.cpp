#include "ILI9341.h"
#include "font5x7.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <fcntl.h>
#include <linux/gpio.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {
void check(bool ok, const char* action) {
    if (!ok) throw std::runtime_error(std::string(action) + ": " + std::strerror(errno));
}
void write_all(int fd, const uint8_t* bytes, size_t size) {
    while (size) {
        ssize_t n = ::write(fd, bytes, size);
        check(n > 0, "SPI write");
        bytes += n;
        size -= size_t(n);
    }
}
int request_line(int chip, unsigned line, uint8_t initial) {
    gpiohandle_request req{};
    req.lines = 1;
    req.lineoffsets[0] = line;
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    req.default_values[0] = initial;
    std::strncpy(req.consumer_label, "luckfox-ili9341", sizeof(req.consumer_label) - 1);
    check(ioctl(chip, GPIO_GET_LINEHANDLE_IOCTL, &req) == 0, "request GPIO line");
    return req.fd;
}
}

ILI9341::ILI9341(const std::string& spi, const std::string& gpiochip, unsigned dc,
                 int reset, uint32_t hz, int cs)
    : spi_path_(spi), gpiochip_path_(gpiochip), dc_line_(dc), reset_line_(reset),
      cs_line_(cs), hz_(hz) {}

ILI9341::~ILI9341() {
    if (cs_ >= 0) close(cs_);
    if (reset_ >= 0) close(reset_);
    if (dc_ >= 0) close(dc_);
    if (spi_ >= 0) close(spi_);
}

void ILI9341::gpio(int fd, uint8_t value) {
    gpiohandle_data values{};
    values.values[0] = value;
    check(ioctl(fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &values) == 0, "set GPIO line");
}

void ILI9341::data(const uint8_t* bytes, unsigned size) {
    gpio(dc_, 1);
    write_all(spi_, bytes, size);
}

void ILI9341::command(uint8_t value, const uint8_t* bytes, unsigned size) {
    if (cs_ >= 0) gpio(cs_, 0);
    gpio(dc_, 0);
    write_all(spi_, &value, 1);
    if (size) data(bytes, size);
    if (cs_ >= 0) gpio(cs_, 1);
}

void ILI9341::init() {
    if (spi_ >= 0) return;
    int chip = open(gpiochip_path_.c_str(), O_RDWR | O_CLOEXEC);
    check(chip >= 0, "open gpiochip");
    try {
        dc_ = request_line(chip, dc_line_, 1);
        if (reset_line_ >= 0) reset_ = request_line(chip, unsigned(reset_line_), 1);
        if (cs_line_ >= 0) cs_ = request_line(chip, unsigned(cs_line_), 1);
    } catch (...) {
        close(chip);
        throw;
    }
    close(chip);
    spi_ = open(spi_path_.c_str(), O_RDWR | O_CLOEXEC);
    check(spi_ >= 0, "open spidev");
    uint8_t mode = SPI_MODE_0, bits = 8;
    check(ioctl(spi_, SPI_IOC_WR_MODE, &mode) == 0, "set SPI mode");
    check(ioctl(spi_, SPI_IOC_WR_BITS_PER_WORD, &bits) == 0, "set SPI word size");
    check(ioctl(spi_, SPI_IOC_WR_MAX_SPEED_HZ, &hz_) == 0, "set SPI speed");
    if (reset_ >= 0) {
        gpio(reset_, 0); usleep(20000);
        gpio(reset_, 1); usleep(120000);
    }
    const uint8_t cf[] = {0x00, 0xC1, 0x30}, ed[] = {0x64, 0x03, 0x12, 0x81};
    const uint8_t e8[] = {0x85, 0x00, 0x78}, cb[] = {0x39, 0x2C, 0x00, 0x34, 0x02};
    const uint8_t ea[] = {0x00, 0x00}, power[] = {0x10}, power2[] = {0x00};
    const uint8_t vcom[] = {0x30, 0x30}, vcom2[] = {0xB7}, frame[] = {0, 0x1A};
    const uint8_t display_function[] = {0x08, 0x82, 0x27};
    const uint8_t gamma_pos[] = {0x0F, 0x2A, 0x28, 0x08, 0x0E, 0x08, 0x54, 0xA9,
                                 0x43, 0x0A, 0x0F, 0, 0, 0, 0};
    const uint8_t gamma_neg[] = {0, 0x15, 0x17, 0x07, 0x11, 0x06, 0x2B, 0x56,
                                 0x3C, 0x05, 0x10, 0x0F, 0x3F, 0x3F, 0x0F};
    command(0x01); usleep(120000); // Software reset
    command(0xCF, cf, sizeof(cf));
    command(0xED, ed, sizeof(ed));
    command(0xE8, e8, sizeof(e8));
    command(0xCB, cb, sizeof(cb));
    const uint8_t f7 = 0x20;
    command(0xF7, &f7, 1);
    command(0xEA, ea, sizeof(ea));
    command(0xC0, power, sizeof(power));
    command(0xC1, power2, sizeof(power2));
    command(0xC5, vcom, sizeof(vcom));
    command(0xC7, vcom2, sizeof(vcom2));
    const uint8_t rgb565 = 0x55;
    command(0x3A, &rgb565, 1);
    command(0xB1, frame, sizeof(frame));
    command(0xB6, display_function, sizeof(display_function));
    const uint8_t gamma_control = 0, gamma_curve = 1;
    command(0xF2, &gamma_control, 1);
    command(0x26, &gamma_curve, 1);
    command(0xE0, gamma_pos, sizeof(gamma_pos));
    command(0xE1, gamma_neg, sizeof(gamma_neg));
    setRotation(0);
    command(0x11); usleep(120000); // Sleep out
    command(0x29); usleep(20000);  // Display on
}

void ILI9341::setRotation(unsigned rotation) {
    static const uint8_t madctl[] = {0x48, 0x28, 0x88, 0xE8};
    rotation %= 4;
    command(0x36, &madctl[rotation], 1);
    width_ = rotation & 1 ? 320 : 240;
    height_ = rotation & 1 ? 240 : 320;
}

void ILI9341::window(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    const uint16_t xe = x + w - 1, ye = y + h - 1;
    const uint8_t col[] = {uint8_t(x >> 8), uint8_t(x), uint8_t(xe >> 8), uint8_t(xe)};
    const uint8_t row[] = {uint8_t(y >> 8), uint8_t(y), uint8_t(ye >> 8), uint8_t(ye)};
    command(0x2A, col, 4);
    command(0x2B, row, 4);
    command(0x2C);
}

void ILI9341::drawPixel(int32_t x, int32_t y, uint16_t color) {
    fillRect(x, y, 1, 1, color);
}

void ILI9341::drawFastHLine(int32_t x, int32_t y, int32_t w, uint16_t color) {
    fillRect(x, y, w, 1, color);
}

void ILI9341::drawFastVLine(int32_t x, int32_t y, int32_t h, uint16_t color) {
    fillRect(x, y, 1, h, color);
}

void ILI9341::fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color) {
    if (w <= 0 || h <= 0) return;
    int64_t x2 = int64_t(x) + w, y2 = int64_t(y) + h;
    x = std::max<int32_t>(0, x); y = std::max<int32_t>(0, y);
    x2 = std::min<int64_t>(width_, x2); y2 = std::min<int64_t>(height_, y2);
    if (x >= x2 || y >= y2) return;
    w = int32_t(x2 - x); h = int32_t(y2 - y);
    window(x, y, w, h);
    uint8_t pixels[4096];
    for (unsigned i = 0; i < sizeof(pixels); i += 2) {
        pixels[i] = color >> 8; pixels[i + 1] = color;
    }
    gpio(dc_, 1);
    if (cs_ >= 0) gpio(cs_, 0);
    size_t remaining = size_t(w) * h * 2;
    while (remaining) {
        size_t n = std::min(remaining, sizeof(pixels));
        write_all(spi_, pixels, n);
        remaining -= n;
    }
    if (cs_ >= 0) gpio(cs_, 1);
}

void ILI9341::pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t* pixels) {
    if (!pixels || w <= 0 || h <= 0) return;
    int64_t x2 = std::min<int64_t>(width_, int64_t(x) + w);
    int64_t y2 = std::min<int64_t>(height_, int64_t(y) + h);
    int32_t left = std::max<int32_t>(0, x), top = std::max<int32_t>(0, y);
    if (left >= x2 || top >= y2) return;
    window(left, top, uint16_t(x2 - left), uint16_t(y2 - top));
    gpio(dc_, 1);
    if (cs_ >= 0) gpio(cs_, 0);
    uint8_t buffer[4096];
    for (int32_t row = top; row < y2; ++row) {
        int32_t col = left;
        while (col < x2) {
            int32_t n = std::min<int64_t>(x2 - col, sizeof(buffer) / 2);
            for (int32_t i = 0; i < n; ++i) {
                uint16_t c = pixels[size_t(row - y) * w + (col + i - x)];
                buffer[2 * i] = c >> 8; buffer[2 * i + 1] = c;
            }
            write_all(spi_, buffer, size_t(n) * 2);
            col += n;
        }
    }
    if (cs_ >= 0) gpio(cs_, 1);
}

uint16_t ILI9341::readPixel(int32_t x, int32_t y) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return 0;
    if (cs_ < 0) throw std::runtime_error("readPixel requires a GPIO CS line and SPI MISO");
    window(x, y, 1, 1);
    const uint8_t cmd = 0x2E;
    uint8_t received[4]{}; // First byte is the ILI9341 dummy byte.
    spi_ioc_transfer transfer{};
    transfer.rx_buf = reinterpret_cast<uintptr_t>(received);
    transfer.len = sizeof(received);
    transfer.speed_hz = hz_;
    transfer.bits_per_word = 8;
    gpio(cs_, 0);
    gpio(dc_, 0);
    write_all(spi_, &cmd, 1);
    gpio(dc_, 1);
    check(ioctl(spi_, SPI_IOC_MESSAGE(1), &transfer) >= 0, "SPI pixel read");
    gpio(cs_, 1);
    return color565(received[1], received[2], received[3]);
}

void ILI9341::drawChar(int32_t x, int32_t y, char c, uint16_t color,
                       uint16_t background, unsigned scale) {
    if (c < 32 || c > 126 || scale == 0 || scale > 8) return;
    std::vector<uint16_t> image(6 * scale * 8 * scale, background);
    const uint8_t* glyph = font5x7 + (c - 32) * 5;
    const unsigned stride = 6 * scale;
    for (unsigned col = 0; col < 5; ++col)
        for (unsigned row = 0; row < 7; ++row)
            if (glyph[col] & (1u << row))
                for (unsigned sy = 0; sy < scale; ++sy)
                    for (unsigned sx = 0; sx < scale; ++sx)
                        image[(row * scale + sy) * stride + col * scale + sx] = color;
    pushImage(x, y, stride, 8 * scale, image.data());
}

void ILI9341::drawString(int32_t x, int32_t y, const std::string& text,
                         uint16_t color, uint16_t background, unsigned scale) {
    if (scale == 0 || scale > 8) return;
    const unsigned cell_width = 6 * scale, cell_height = 8 * scale;
    size_t start = 0;
    while (start <= text.size()) {
        size_t end = text.find('\n', start);
        if (end == std::string::npos) end = text.size();
        size_t count = end - start;
        if (count && count <= size_t(width_) / cell_width + 1) {
            const size_t stride = count * cell_width;
            std::vector<uint16_t> image(stride * cell_height, background);
            for (size_t i = 0; i < count; ++i) {
                unsigned char c = text[start + i];
                if (c < 32 || c > 126) continue;
                const uint8_t* glyph = font5x7 + (c - 32) * 5;
                for (unsigned col = 0; col < 5; ++col)
                    for (unsigned row = 0; row < 7; ++row)
                        if (glyph[col] & (1u << row))
                            for (unsigned sy = 0; sy < scale; ++sy)
                                for (unsigned sx = 0; sx < scale; ++sx)
                                    image[(row * scale + sy) * stride + i * cell_width + col * scale + sx] = color;
            }
            pushImage(x, y, stride, cell_height, image.data());
        } else if (count) {
            for (size_t i = start; i < end; ++i)
                drawChar(x + int32_t(i - start) * cell_width, y, text[i], color, background, scale);
        }
        if (end == text.size()) break;
        start = end + 1;
        y += cell_height;
    }
}

void ILI9341::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint16_t color) {
    int64_t dx = std::llabs(int64_t(x1) - x0), dy = -std::llabs(int64_t(y1) - y0);
    int32_t sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
    int64_t err = dx + dy;
    const bool steep = dx < -dy;
    int32_t run_x = x0, run_y = y0, last_x = x0, last_y = y0;
    const auto flush = [&](int32_t end_x, int32_t end_y) {
        if (steep) {
            int64_t top = std::max<int64_t>(0, std::min(run_y, end_y));
            int64_t bottom = std::min<int64_t>(height_ - 1, std::max(run_y, end_y));
            if (top <= bottom) fillRect(run_x, top, 1, bottom - top + 1, color);
        } else {
            int64_t left = std::max<int64_t>(0, std::min(run_x, end_x));
            int64_t right = std::min<int64_t>(width_ - 1, std::max(run_x, end_x));
            if (left <= right) fillRect(left, run_y, right - left + 1, 1, color);
        }
    };
    for (;;) {
        if ((steep && x0 != run_x) || (!steep && y0 != run_y)) {
            flush(last_x, last_y);
            run_x = x0; run_y = y0;
        }
        if (x0 == x1 && y0 == y1) break;
        last_x = x0; last_y = y0;
        int64_t e = 2 * err;
        if (e >= dy) { err += dy; x0 += sx; }
        if (e <= dx) { err += dx; y0 += sy; }
    }
    flush(x0, y0);
}
