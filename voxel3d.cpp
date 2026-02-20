#include <SDL3\SDL.h>
#include "world.hpp"
#include "raytrace.hpp"
#include "thread_pool.hpp"

#define WIDTH 1280
#define HEIGHT 720
#define NUM_THREADS 11

using namespace std;

void Update(World* world) {
    UpdateWorld(world);

    Vec forward = PolarToCartesian(world->camera.view);
    Vec point = Raytrace(*world, forward);
    if (IsInWorld(*world, point)) 
        world->selected = GetBlockAt(*world, point);
}

void Draw(Pool* pool, World world, View screen) {
    RaytraceData data = BeginRaytrace(world.camera, screen);
    PoolSubmit(pool, screen.width, [=] (int start, int size) {
        for (int x = start; x < start+size; x++)
            for (int y = 0; y < screen.height; y++) {
                Vec dir = GetDirection(data, x, y, screen);
                Vec ray = Raytrace(world, dir);
                screen.pixels[x+y*screen.width] = GetColorAt(world, ray);
            }
    });
    WaitForJobFinish(pool);
}

void ShowPointer(View screen) {
    int cx = screen.width/2, cy = screen.height/2;
    for (int i = 0; i <= 10; i++) {
        int x = cx-5+i, y = cy-5+i;
        screen.pixels[x+cy*screen.width] = 0x00AACC;
        screen.pixels[cx+y*screen.width] = 0x00AACC;
    }
}

void KeyPressed(SDL_Keycode key, World* world) {
    world->camera.dv.x = key == SDLK_W ? 1 : key == SDLK_S ? -1 : 0;
    world->camera.dv.y = key == SDLK_A ? 1 : key == SDLK_D ? -1 : 0;
    world->camera.dv.z = key == SDLK_SPACE ? 1 : key == SDLK_LSHIFT ? -1 : 0;
    MoveCamera(&world->camera);
}

void KeyReleased(SDL_Keycode key, World* world) {
    if (key == SDLK_W || key == SDLK_S) world->camera.dv.x = 0;
    else if (key == SDLK_A || key == SDLK_D) world->camera.dv.y = 0;
    else if (key == SDLK_LSHIFT || key == SDLK_SPACE) world->camera.dv.z = 0;
    MoveCamera(&world->camera);
}

void MouseMoved(int dx, int dy, World* world) {
    Vec delta_view = {atanf(-dx/200.0), atanf(-dy/300.0)};
    world->camera.view = vec_add(world->camera.view, delta_view);
}

void MousePressed(int button, int x, int y, World* world) {
    Vec forward = PolarToCartesian(world->camera.view);
    Vec ray = Raytrace(*world, forward);
    if (!IsInWorld(*world, ray)) return;

    if (button == SDL_BUTTON_RIGHT) {
        Vec normal = GetFaceNormal(ray, 0.01);
        Vec pos = vec_add(ray, normal);
        if (IsInWorld(*world, pos))
            *GetBlockAt(*world, pos) = Blocks.STONE;
    } else if (button == SDL_BUTTON_LEFT)
        *GetBlockAt(*world, ray) = Blocks.AIR;
}

int main() {
    SDL_Window* window = SDL_CreateWindow("Voxel3D", WIDTH, HEIGHT, 0);
    SDL_Surface* surface = SDL_GetWindowSurface(window);

    World world = CreateWorld(9, 9, 9);
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            for (int k = 3; k < 6; k++)
                world.blocks[i][j][k] = Blocks.STONE;

    View screen = MakeView(surface, M_PI/3);
    Pool* pool = CreatePool(NUM_THREADS);
    
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event))
            switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                KeyPressed(event.key.key, &world);
                break;
            case SDL_EVENT_KEY_UP:
                KeyReleased(event.key.key, &world);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                MouseMoved(event.motion.xrel, event.motion.yrel, &world);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                MousePressed(event.button.button, event.button.x, event.button.y, &world);
                break;
            }

        Update(&world);
        Draw(pool, world, screen);
        ShowPointer(screen);
        SDL_UpdateWindowSurface(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}