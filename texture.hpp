#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <SDL3_image\SDL_image.h>
#include "geom.hpp"

typedef struct {
    int width, height;
    int* pixels;
} Image;

Image LoadImage(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) return {0, 0, NULL};

    int pixels_len = surface->w*surface->h;
    Image image = {
        surface->w, surface->h,
        new int[pixels_len]
    };

    SDL_Surface* converted = SDL_ConvertSurface(surface, SDL_PixelFormat::SDL_PIXELFORMAT_RGBA32);
    memcpy(image.pixels, converted->pixels, pixels_len*sizeof(int));
    SDL_DestroySurface(surface);
    SDL_DestroySurface(converted);
    return image;
}

int GetImageRGB(Image image, Vec uv) {
    int x = (int) (uv.x*image.width);
    int y = (int) (uv.y*image.height);
    return image.pixels[x+y*image.width];
}

#endif