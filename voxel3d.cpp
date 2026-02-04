#include <SDL3\SDL.h>
#include "world.hpp"
#include "raytrace.hpp"

#define WIDTH 1280
#define HEIGHT 720
#define RESOLUTION 10

void Update(World* world) {
    world->camera.view.x += 0.05;
}

void Draw(World* world, View* screen) {
    compute_directions(&world->camera, screen);
    for (int y = 0; y < screen->height; y++)
        for (int x = 0; x < screen->width; x++) {
            int idx = y*screen->width+x;
            screen->pixels[idx] = raytrace(world, screen->directions[idx]);
        }
}

int main() {
    SDL_Window* window = SDL_CreateWindow("Voxel3D", WIDTH, HEIGHT, 0);
    SDL_Surface* surface = SDL_GetWindowSurface(window);

    World world = create_world(9, 9, 9);
    for (int i = 3; i < 6; i++)
        for (int j = 3; j < 6; j++)
            for (int k = 3; k < 6; k++)
                world.blocks[i][j][k] = BlockType::STONE;

    View screen = init_screen(surface, M_PI/3);
    SDL_Rect window_rect = {0, 0, WIDTH, HEIGHT};

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event))
            if (event.type == SDL_EVENT_QUIT)
                running = false;

        Update(&world);
        SDL_FillSurfaceRect(surface, &window_rect, 0);
        Draw(&world, &screen);
        SDL_UpdateWindowSurface(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}