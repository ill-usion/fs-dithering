#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include "palettes.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define xy2idx(x, y, w, n) (((y) * (w) + (x)) * (n))
#define clamp(v, l, u) ((v < l) ? l : (v > u) ? u : v)

void rgb_mul(struct rgb_t* col, float coeff, struct rgb_t* dest_col);
void rgb_diff(struct rgb_t* lcol, struct rgb_t* rcol, struct rgb_t* dest_col);
void rgb_add(struct rgb_t* lcol, struct rgb_t* rcol, struct rgb_t* dest_col);
void fs_find_closest_color(struct rgb_t* col, const struct rgb_t* palette, const size_t palette_len, struct rgb_t* dest_col);
void fs_dither_rgb(uint8_t* img, int w, int h, const struct palette_t* palette);

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("%s [filename] [palette=0] [--list-palettes]\n", argv[0]);
        return 1;
    }

    const char* fn = argv[1];
    if (!strcmp(fn, "--list-palettes")) {
        printf("Available color palettes:\n");
        for (int i = 0; i < PALETTES_LEN; i++) {
            printf("\t- %d %s\n", i, PALETTES_MAP[i].name);
        }
        return 0;
    }

    int x, y, n;
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
    const struct palette_t palette = PALETTES_MAP[0];
    fs_dither_rgb(img, x, y, &palette);

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

void fs_find_closest_color(struct rgb_t* col, const struct rgb_t* palette, const size_t palette_len, struct rgb_t* dest_col) {
    int min_idx;
    float dist, min_dist = FLT_MAX;
    struct rgb_t pal;
    for (int i = 0; i < palette_len; i++) {
        pal = palette[i];
        dist = sqrt((col->r - pal.r) * (col->r - pal.r) + 
                    (col->g - pal.g) * (col->g - pal.g) + 
                    (col->b - pal.b) * (col->b - pal.b));

        if (dist < min_dist) {
            min_dist = dist;
            min_idx = i;
        }
    }

    pal = palette[min_idx];
    dest_col->r = pal.r;
    dest_col->g = pal.g;
    dest_col->b = pal.b;
}

void fs_dither_rgb(uint8_t* img, int w, int h, const struct palette_t* palette) {
    const int N = 3;
    struct rgb_t* pix, *next, new, err, temp;

    for (int y = 0; y < h - 1; y++) {
        for (int x = 0; x < w - 1; x++) {
            pix = (struct rgb_t*)(img + xy2idx(x, y, w, N));
            fs_find_closest_color(pix, palette->ptr, palette->len, &new);
            rgb_diff(pix, &new, &err); 
            pix->r = new.r;
            pix->g = new.g;
            pix->b = new.b;
            
            next = (struct rgb_t*)(img + xy2idx(x + 1, y, w, N));
            rgb_mul(&err, 7.0f / 16.0f, &temp);
            rgb_add(next, &temp, next);

            next = (struct rgb_t*)(img + xy2idx(x - 1, y + 1, w, N));
            rgb_mul(&err, 3.0f / 16.0f, &temp);
            rgb_add(next, &temp, next);

            next = (struct rgb_t*)(img + xy2idx(x, y + 1, w, N));
            rgb_mul(&err, 5.0f / 16.0f, &temp);
            rgb_add(next, &temp, next);

            next = (struct rgb_t*)(img + xy2idx(x + 1, y + 1, w, N));
            rgb_mul(&err, 1.0f / 16.0f, &temp);
            rgb_add(next, &temp, next);
        }
    }
}
