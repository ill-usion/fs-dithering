#ifndef PALETTES_H
#define PALETTES_H

struct rgb_t {
    uint8_t r, g, b;
};

struct palette_t {
    const struct rgb_t* ptr;
    size_t len;
    const char* name;
};

// https://en.wikipedia.org/wiki/List_of_16-bit_computer_color_palettes
static const struct rgb_t VGA16[] = {
    {   0,   0,   0 }, // 0  black
    {   0,   0, 170 }, // 1  blue
    {   0, 170,   0 }, // 2  green
    {   0, 170, 170 }, // 3  cyan
    { 170,   0,   0 }, // 4  red
    { 170,   0, 170 }, // 5  magenta
    { 170,  85,   0 }, // 6  brown
    { 170, 170, 170 }, // 7  light gray

    {  85,  85,  85 }, // 8  dark gray
    {  85,  85, 255 }, // 9  bright blue
    {  85, 255,  85 }, // 10 bright green
    {  85, 255, 255 }, // 11 bright cyan
    { 255,  85,  85 }, // 12 bright red
    { 255,  85, 255 }, // 13 bright magenta
    { 255, 255,  85 }, // 14 yellow
    { 255, 255, 255 }  // 15 white
};


static const struct palette_t PALETTES_MAP[] = {
    { VGA16, 16, "VGA16"},
};

const size_t PALETTES_LEN = 1;

#endif
