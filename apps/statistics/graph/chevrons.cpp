#include "chevrons.h"

namespace Statistics {

const uint8_t Chevrons::DownChevronMask
    [Chevrons::k_chevronHeight][Chevrons::k_chevronWidth] = {
        {0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C},
        {0xE1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE1},
        {0xFF, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x45, 0xFF},
        {0xFF, 0xFF, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x0C, 0xFF, 0xFF},
        {0xFF, 0xFF, 0xE1, 0x00, 0x00, 0x00, 0x00, 0xE1, 0xFF, 0xFF},
        {0xFF, 0xFF, 0xFF, 0x45, 0x00, 0x00, 0x45, 0xFF, 0xFF, 0xFF},
        {0xFF, 0xFF, 0xFF, 0xFF, 0x0C, 0x0C, 0xFF, 0xFF, 0xFF, 0xFF},
        {0xFF, 0xFF, 0xFF, 0xFF, 0x45, 0x45, 0xFF, 0xFF, 0xFF, 0xFF},
};

const uint8_t Chevrons::UpChevronMask
    [Chevrons::k_chevronHeight][Chevrons::k_chevronWidth] = {
        {0xFF, 0xFF, 0xFF, 0xFF, 0x45, 0x45, 0xFF, 0xFF, 0xFF, 0xFF},
        {0xFF, 0xFF, 0xFF, 0xFF, 0x0C, 0x0C, 0xFF, 0xFF, 0xFF, 0xFF},
        {0xFF, 0xFF, 0xFF, 0x45, 0x00, 0x00, 0x45, 0xFF, 0xFF, 0xFF},
        {0xFF, 0xFF, 0xE1, 0x00, 0x00, 0x00, 0x00, 0xE1, 0xFF, 0xFF},
        {0xFF, 0xFF, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x0C, 0xFF, 0xFF},
        {0xFF, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x45, 0xFF},
        {0xE1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE1},
        {0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C},
};

}  // namespace Statistics