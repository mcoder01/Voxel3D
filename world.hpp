#ifndef WORLD_HPP
#define WORLD_HPP

#include <variant>
#include "geom.hpp"
#include "texture.hpp"
#include <stdio.h>

typedef struct {
    Vec pos, vel, view, dv;
    float speed;
} Camera;

enum BlockType {
    SOLID=0, TRANSLUCENT=1
};

typedef struct {
    int type;
    Image texture;
} Block;

struct {
    Block AIR = {BlockType::TRANSLUCENT, {1, 1, new int[1] {0}}};
    Block STONE = {BlockType::SOLID, LoadImage("textures/stone.png")};
} Blocks;

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
        {{0, 0, 3}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, 0.3}};

    for (int i = 0; i < width; i++) {
        world.blocks[i] = new Block*[height];
        for (int j = 0; j < height; j++) {
            world.blocks[i][j] = new Block[depth];
            for (int k = 0; k < depth; k++)
                world.blocks[i][j][k] = Blocks.AIR;
        }
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

bool StoreMin(float& min, float value) {
    if (value < min) {
        min = value;
        return true;
    }

    return false;
}

Vec FindTextureUV(Vec pos) {
    Vec uv = {ceil(pos.y)-pos.y, ceil(pos.z)-pos.z};
    float dist = pos.x-floor(pos.x);
    if (StoreMin(dist, ceil(pos.x)-pos.x))
        uv = {pos.y-floor(pos.y), pos.z-floor(pos.z)};
    
    if (StoreMin(dist, pos.y-floor(pos.y)))
        uv = {pos.x-floor(pos.x), ceil(pos.z)-pos.z};
    else if (StoreMin(dist, ceil(pos.y)-pos.y))
        uv = {ceil(pos.x)-pos.x, ceil(pos.z)-pos.z};
    
    if (StoreMin(dist, pos.z-floor(pos.z)))
        uv = {ceil(pos.y)-pos.y, pos.x-floor(pos.x)};
    else if (StoreMin(dist, ceil(pos.z)-pos.z))
        uv = {ceil(pos.y)-pos.y, ceil(pos.x)-pos.x};
    return uv;
}

int GetColorAt(World world, Vec point) {
    if (!IsInWorld(world, point))
        return 0;

    Block* block = GetBlockAt(world, point);
    Vec uv = FindTextureUV(point);
    int color = GetImageRGB(block->texture, uv);
    if (world.selected && world.selected == block)
        color = std::min(color+0x222222, 0xffffff);
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