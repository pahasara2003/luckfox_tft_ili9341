#pragma once

// Change these values for your board and wiring. GPIO numbers are chip-local offsets.
namespace board {
constexpr const char* spi = "/dev/spidev0.0";
constexpr const char* gpiochip = "/dev/gpiochip1";
constexpr unsigned dc = 20;
constexpr int reset = -1; // GPIO1_C6 is held high outside the example.
constexpr int cs = 25;
constexpr unsigned hz = 40000000;
constexpr unsigned rotation = 2;
}
