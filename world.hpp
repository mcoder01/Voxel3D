#ifndef WORLD_HPP
#define WORLD_HPP

#include "geom.hpp"

enum BlockType {
    AIR=0, STONE=0xaaaaaa
};

typedef struct {
    Vec pos, view;
} Camera;

typedef struct {
    int width, height, depth;
    int*** blocks;
    Camera camera;
} World;

World create_world(int width, int height, int depth) {
    World world = {width, height, depth};
    world.blocks = new int**[width];
    world.camera = {{0, 0, 1.6}, {0, 0}};
    for (int i = 0; i < width; i++) {
        world.blocks[i] = new int*[height];
        for (int j = 0; j < height; j++)
            world.blocks[i][j] = new int[depth] {BlockType::AIR};
    }

    return world;
}

#endif