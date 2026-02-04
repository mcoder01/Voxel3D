#ifndef GEOM_HPP
#define GEOM_HPP

#include <math.h>

typedef struct {
    float x, y, z;
} Vec;

Vec vec_add(Vec v1, Vec v2) {
    return {v1.x+v2.x, v1.y+v2.y, v1.z+v2.z};
}

Vec vec_scale(Vec v, float m) {
    return {v.x*m, v.y*m, v.z*m};
}

Vec vec_sub(Vec v1, Vec v2) {
    return vec_add(v1, vec_scale(v2, -1));
}

float vec_mag(Vec v) {
    return sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
}

Vec normalize(Vec v) {
    float mag = vec_mag(v);
    return {v.x/mag, v.y/mag, v.z/mag};
}

Vec vec_lerp(Vec v1, Vec v2, float t) {
    Vec diff = vec_sub(v2, v1);
    return vec_add(vec_scale(diff, t), v1);
}

#endif