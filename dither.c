#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define XY2IDX(x, y, w, n) (((y) * (w) + (x)) * (n))
#define clamp(v, l, u) ((v < l) ? l : (v > u) ? u : v)

struct rgb_t {
    uint8_t r, g, b;
};
// https://en.wikipedia.org/wiki/List_of_16-bit_computer_color_palettes
static const struct rgb_t PALETTE[] = {
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

void rgb_mul(struct rgb_t* col, float coeff, struct rgb_t* dest_col);
void rgb_diff(struct rgb_t* lcol, struct rgb_t* rcol, struct rgb_t* dest_col);
void rgb_add(struct rgb_t* lcol, struct rgb_t* rcol, struct rgb_t* dest_col);
void fs_find_closest_color(struct rgb_t* col, struct rgb_t* dest_col);
void fs_dither_rgb(uint8_t* img, int w, int h);

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("%s filename\n", argv[0]);
        return 1;
    }

    int x, y, n;
    const char* fn = argv[1];
    uint8_t* img = stbi_load(fn, &x, &y, &n, 0);
    if (!img) {
        printf("Invalid image\n");
        return 1;
    }

    if (n != 3) {
        printf("Unsupported number of channels %d\n", n);
        return 1;
    }

    printf("x=%d, y=%d, n=%d\n", x, y, n);
    fs_dither_rgb(img, x, y);

    stbi_write_jpg("output.jpg", x, y, n, img, 100);

    stbi_image_free(img);
    return 0;
}

inline void rgb_mul(struct rgb_t* col, float coeff, struct rgb_t* dest_col) {
    dest_col->r = col->r * coeff;
    dest_col->g = col->g * coeff;
    dest_col->b = col->b * coeff;
}

inline void rgb_diff(struct rgb_t* lcol, struct rgb_t* rcol, struct rgb_t* dest_col) {
    dest_col->r = clamp(lcol->r - rcol->r, 0, 255);
    dest_col->g = clamp(lcol->g - rcol->g, 0, 255);
    dest_col->b = clamp(lcol->b - rcol->b, 0, 255);
}

inline void rgb_add(struct rgb_t* lcol, struct rgb_t* rcol, struct rgb_t* dest_col) {
    dest_col->r = clamp(lcol->r + rcol->r, 0, 255);
    dest_col->g = clamp(lcol->g + rcol->g, 0, 255);
    dest_col->b = clamp(lcol->b + rcol->b, 0, 255);
}

void fs_find_closest_color(struct rgb_t* col, struct rgb_t* dest_col) {
    int min_idx;
    float dist, min_dist = FLT_MAX;
    struct rgb_t pal;
    for (int i = 0; i < sizeof(PALETTE) / sizeof(struct rgb_t); i++) {
        pal = PALETTE[i];
        dist = sqrt((col->r - pal.r) * (col->r - pal.r) + 
                    (col->g - pal.g) * (col->g - pal.g) + 
                    (col->b - pal.b) * (col->b - pal.b));

        if (dist < min_dist) {
            min_dist = dist;
            min_idx = i;
        }
    }

    pal = PALETTE[min_idx];
    dest_col->r = pal.r;
    dest_col->g = pal.g;
    dest_col->b = pal.b;
}

void fs_dither_rgb(uint8_t* img, int w, int h) {
    const int N = 3;
    struct rgb_t* pix, *next, new, err, temp;
    for (int y = 0; y < h - 2; y++) {
        for (int x = 0; x < w - 2; x++) {
            pix = (struct rgb_t*)(img + XY2IDX(x, y, w, N));
            fs_find_closest_color(pix, &new);
            rgb_diff(pix, &new, &err); 
            pix->r = new.r;
            pix->g = new.g;
            pix->b = new.b;
            
            next = (struct rgb_t*)(img + XY2IDX(x + 1, y, w, N));
            rgb_mul(&err, 7.0f / 16.0f, &temp);
            rgb_add(next, &temp, next);

            next = (struct rgb_t*)(img + XY2IDX(x - 1, y + 1, w, N));
            rgb_mul(&err, 3.0f / 16.0f, &temp);
            rgb_add(next, &temp, next);

            next = (struct rgb_t*)(img + XY2IDX(x, y + 1, w, N));
            rgb_mul(&err, 5.0f / 16.0f, &temp);
            rgb_add(next, &temp, next);

            next = (struct rgb_t*)(img + XY2IDX(x + 1, y + 1, w, N));
            rgb_mul(&err, 1.0f / 16.0f, &temp);
            rgb_add(next, &temp, next);
        }
    }
}
