#ifndef RAYTRACE_HPP
#define RAYTRACE_HPP

#include <SDL3\SDL.h>
#include "texture.hpp"
#include "geom.hpp"
#include "world.hpp"

typedef struct : Image {
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

float DistanceToNext(float pos, float dir, float eps) {
    if (abs(dir) < eps) return INFINITY;
    float voxel = floor(pos);
    if (dir > 0) voxel++;
    return (voxel-pos)/dir;
}

float VoxelDistance(Vec pos, Vec dir, float eps) {
    float dist = DistanceToNext(pos.x, dir.x, eps);
    dist = min(DistanceToNext(pos.y, dir.y, eps), dist);
    return min(DistanceToNext(pos.z, dir.z, eps), dist);
}

Vec GetDirection(RaytraceData data, int x, int y, View screen) {
    Vec dir = vec_add(data.top_left, vec_scale(data.to_right, (float) x/(screen.width-1)));
    return vec_add(dir, vec_scale(data.to_bottom, (float) y/(screen.height-1)));
}

Vec Raytrace(World world, Vec dir) {
    const float eps = 0.01;
    Vec center = {(float) world.width/2, (float) world.height/2, (float) world.depth/2};
    Vec point = vec_add(center, world.camera.pos);
    while(IsInWorld(world, point)) {
        Block* block = GetBlockAt(world, point);
        if (block->type != BlockType::TRANSLUCENT)
            return point;

        float dist = VoxelDistance(point, dir, eps);
        point = vec_add(point, vec_scale(dir, dist+eps));
    }

    return point;
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

    float halfHeight = tan(screen.fov/2);
    float halfWidth = halfHeight*screen.aspect_ratio;

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