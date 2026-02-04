#ifndef RAYTRACE_HPP
#define RAYTRACE_HPP

#include <SDL3\SDL.h>
#include "geom.hpp"
#include "world.hpp"

typedef struct {
    int width, height;
    int* pixels;
    float aspect_ratio, fov;
    Vec* directions;
} View;

bool in_world(Vec pos, World* world) {
    return pos.x >= 0 && pos.x < world->width
        && pos.y >= 0 && pos.y < world->height
        && pos.z >= 0 && pos.z < world->depth;
}

float axisDistToNextVoxel(float pos, float dir, float eps) {
    if (abs(dir) < eps) return INFINITY;
    float voxel = floor(pos);
    if (dir > 0) voxel++;
    return (voxel-pos)/dir;
}

float min(float a, float b) {
    return a < b ? a : b;
}

int raytrace(World* world, Vec dir) {
    const float eps = 0.01;
    Vec center = {(float) world->width/2, (float) world->height/2, (float) world->depth/2};
    Vec ray = {0};
    while(true) {
        Vec pos = vec_sub(vec_add(ray, center), world->camera.pos);
        if (!in_world(pos, world)) break;

        int block = world->blocks[(int) pos.x][(int) pos.y][(int) pos.z];
        if (block != BlockType::AIR) 
            return block;

        float dist = min(axisDistToNextVoxel(ray.x, dir.x, eps), 2);
        dist = min(axisDistToNextVoxel(ray.y, dir.y, eps), dist);
        dist = min(axisDistToNextVoxel(ray.z, dir.z, eps), dist);
        ray = vec_add(ray, vec_scale(dir, dist+eps));
    }

    return BlockType::AIR;
}

Vec polar_to_cartesian(Vec angles) {
    Vec v;
    v.x = cos(angles.y)*cos(angles.x);
    v.y = cos(angles.y)*sin(angles.x);
    v.z = sin(angles.y);
    return v;
}

void compute_directions(Camera* camera, View* screen) {
    if (screen->directions == NULL)
        screen->directions = new Vec[screen->width*screen->height];

    float hfov = screen->fov*screen->aspect_ratio;
    Vec left = polar_to_cartesian({camera->view.x-screen->fov/2, 0, 0});
    Vec right = polar_to_cartesian({camera->view.x+screen->fov/2, 0, 0});
    Vec top = polar_to_cartesian({0, camera->view.y-hfov/2, 0});
    Vec bottom = polar_to_cartesian({0, camera->view.y+hfov/2, 0});

    Vec to_right = vec_sub(right, left);
    Vec to_bottom = vec_sub(bottom, top);

    Vec top_left = vec_sub(left, vec_scale(to_bottom, 0.5));
    for (int y = 0; y < screen->height; y++)
        for (int x = 0; x < screen->width; x++) {
            Vec dir = vec_add(top_left, vec_scale(to_right, (float) x/(screen->width-1)));
            dir = vec_add(dir, vec_scale(to_bottom, (float) y/(screen->height-1)));
            screen->directions[y*screen->width+x] = normalize(dir);
        }
}

View init_screen(SDL_Surface* surface, float fov) {
    return {
        surface->w, surface->h, 
        (int*) surface->pixels,
        (float) surface->h/surface->w,
        fov, NULL
    };
}

#endif