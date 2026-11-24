#include "ILI9341.h"
#include "Sprite.h"
#include <cassert>

static_assert(ILI9341::color565(255, 0, 0) == 0xF800, "red RGB565");
static_assert(ILI9341::color565(0, 255, 0) == 0x07E0, "green RGB565");
static_assert(ILI9341::color565(0, 0, 255) == 0x001F, "blue RGB565");

int main() {
    ILI9341 tft("/dev/spidev0.0", "/dev/gpiochip0", 0);
    assert(tft.width() == 240 && tft.height() == 320);
    Sprite s(4, 3);
    s.fillSprite(0x1234);
    s.fillRect(-1, 1, 3, 2, 0xFFFF);
    assert(s.readPixel(0, 0) == 0x1234);
    assert(s.readPixel(0, 1) == 0xFFFF);
    assert(s.readPixel(2, 1) == 0x1234);
}
