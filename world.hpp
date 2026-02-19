#ifndef WORLD_HPP
#define WORLD_HPP

#include "geom.hpp"
#include <stdio.h>

enum BlockType {
    AIR=0, STONE=0xaaaaaa
};

typedef struct {
    Vec pos, vel, view, dv;
    float speed;
} Camera;

typedef struct {
    int type;
    float borderWidth;
} Block;

typedef struct {
    int width, height, depth;
    Block*** blocks;
    Camera camera;
    Block* selected;
} World;

World CreateWorld(int width, int height, int depth) {
    World world = {
        width, height, depth,
        new Block**[width],
        {{0, 0, 2.3}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, 0.3}};

    for (int i = 0; i < width; i++) {
        world.blocks[i] = new Block*[height];
        for (int j = 0; j < height; j++)
            world.blocks[i][j] = new Block[depth] {BlockType::AIR, false};
    }

    return world;
}

void UpdateWorld(World* world) {
    world->camera.pos = vec_add(world->camera.pos, world->camera.vel);
}

void MoveCamera(Camera* camera) {
    camera->vel.x = camera->dv.x*cos(camera->view.x)-camera->dv.y*sin(camera->view.x);
    camera->vel.y = camera->dv.y*cos(camera->view.x)+camera->dv.x*sin(camera->view.x);
    camera->vel.z = camera->dv.z;
    camera->vel = vec_scale(camera->vel, camera->speed);
}

bool IsInWorld(World world, Vec pos) {
    return pos.x >= 0 && pos.x < world.width
        && pos.y >= 0 && pos.y < world.height
        && pos.z >= 0 && pos.z < world.depth;
}

Block* GetBlockAt(World world, Vec pos) {
    return &world.blocks[(int) pos.x][(int) pos.y][(int) pos.z];
}

bool IsOnBorder(Block block, Vec point) {
    if (point.x-floor(point.x) <= block.borderWidth || ceil(point.x)-point.x <= block.borderWidth)
        return point.y-floor(point.y) <= block.borderWidth || point.z-floor(point.z) <= block.borderWidth 
            || ceil(point.y)-point.y <= block.borderWidth || ceil(point.z)-point.z <= block.borderWidth;
    if (point.y-floor(point.y) <= block.borderWidth || ceil(point.y)-point.y <= block.borderWidth)
        return point.x-floor(point.x) <= block.borderWidth || point.z-floor(point.z) <= block.borderWidth
            || ceil(point.x)-point.x <= block.borderWidth || ceil(point.z)-point.z <= block.borderWidth;
    if (point.z-floor(point.z) <= block.borderWidth || ceil(point.z)-point.z <= block.borderWidth)
        return point.x-floor(point.x) <= block.borderWidth || point.y-floor(point.y) <= block.borderWidth
            || ceil(point.x)-point.x <= block.borderWidth || ceil(point.y)-point.y <= block.borderWidth;
    return false;
}

int GetColorAt(World world, Vec point) {
    if (!IsInWorld(world, point))
        return 0;

    Block* block = GetBlockAt(world, point);
    int color = IsOnBorder(*block, point) ? 0x555555 : block->type;
    if (world.selected && world.selected == block)
        color += 0x333333;
    return color;
}

Vec GetFaceNormal(Vec point, float eps) {
    if (point.x-floor(point.x) <= eps) return {-1, 0, 0};
    if (ceil(point.x)-point.x <= eps) return {1, 0, 0};
    if (point.y-floor(point.y) <= eps) return {0, -1, 0};
    if (ceil(point.y)-point.y <= eps) return {0, 1, 0};
    if (point.z-floor(point.z) <= eps) return {0, 0, -1};
    if (ceil(point.z)-point.z <= eps) return {0, 0, 1};
    return {0, 0, 0};
}

#endif