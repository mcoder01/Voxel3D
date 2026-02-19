#ifndef RAYTRACE_HPP
#define RAYTRACE_HPP

#include <SDL3\SDL.h>
#include "geom.hpp"
#include "world.hpp"

typedef struct {
    int width, height;
    int* pixels;
    float aspect_ratio, fov;
} View;

typedef struct {
    Vec top_left;
    Vec to_right;
    Vec to_bottom;
} RaytraceData;

float min(float a, float b) {
    return a < b ? a : b;
}

float VoxelDistance(float pos, float dir, float eps) {
    if (abs(dir) < eps) return INFINITY;
    float voxel = floor(pos);
    if (dir > 0) voxel++;
    return (voxel-pos)/dir;
}

Vec GetDirection(RaytraceData data, int x, int y, View screen) {
    Vec dir = vec_add(data.top_left, vec_scale(data.to_right, (float) x/(screen.width-1)));
    return vec_add(dir, vec_scale(data.to_bottom, (float) y/(screen.height-1)));
}

Vec Raytrace(World world, Vec dir) {
    const float eps = 0.01;
    Vec center = {(float) world.width/2, (float) world.height/2, (float) world.depth/2};
    Vec pos = vec_add(center, world.camera.pos);
    while(IsInWorld(world, pos)) {
        Block* block = GetBlockAt(world, pos);
        if (block->type != BlockType::AIR)
            return pos;

        float dist = VoxelDistance(pos.x, dir.x, eps);
        dist = min(VoxelDistance(pos.y, dir.y, eps), dist);
        dist = min(VoxelDistance(pos.z, dir.z, eps), dist);
        pos = vec_add(pos, vec_scale(dir, dist+eps));
    }

    return pos;
}

Vec PolarToCartesian(Vec angles) {
    return {
        cos(angles.y)*cos(angles.x),
        cos(angles.y)*sin(angles.x),
        sin(angles.y)
    };
}

RaytraceData BeginRaytrace(Camera camera, View screen) {
    Vec forward = PolarToCartesian(camera.view);
    Vec right = normalize(vec_cross(forward, {0, 0, 1}));
    Vec up = normalize(vec_cross(right, forward));

    float halfVFov = screen.fov/2;
    float halfHFov = atan(tan(halfVFov)*screen.aspect_ratio);
    float halfWidth = tan(halfHFov);
    float halfHeight = tan(halfVFov);

    RaytraceData data;
    data.top_left = vec_add(forward, vec_scale(right, -halfWidth));
    data.top_left = vec_add(data.top_left, vec_scale(up, halfHeight));
    data.to_right = vec_scale(right, halfWidth*2);
    data.to_bottom = vec_scale(up, -halfHeight*2);
    return data;
}

View MakeView(SDL_Surface* surface, float fov) {
    return {
        surface->w, surface->h, 
        (int*) surface->pixels,
        (float) surface->w/surface->h,
        fov
    };
}

#endif