// Copyright (C) 2024 Benjamin Froelich
// This file is part of https://github.com/bbeni/earth_bender_game
// For conditions of distribution and use, see copyright notice in project root.
#ifndef MMA_H_
#define MMA_H_

#include "math.h"
#include "stdbool.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846  /* pi from math.h */
#endif

#define DEG_TO_RAD M_PI / 180

struct Vec4f {
	float x, y, z, w;
};

struct Vec3f {
	float x, y, z;
};

struct Vec2f {
	float x, y;
};

struct Mat4f {
	// column major format (looks flipped here)
	float u11, u21, u31, u41;
	float u12, u22, u32, u42;
	float u13, u23, u33, u43;
	float u14, u24, u34, u44;
};

extern const struct Mat4f mat4f_unit;

void mma_clampf(float* v, float lower, float upper);
void mma_clampi(int* v, int lower, int upper);
float mma_lerpf(float lower, float upper, float t);
void mma_move_towardsf(float* x, float target, float speed, float dt);
void mma_move_towardsv3f(struct Vec3f* vec, struct Vec3f target, float speed, float dt);
float mma_next_multiple_of(float start, float step);
// move an angle to a range [-pi, pi) closer to the mapped target angle the shorter way around the circle
// @Bug somehow there is and edge case where the target is PI and we go to -PI and never set to PI but it should flip it here
// for now just assume the user of this function is not dependent on the wrap around behaviour
void mma_move_towards_on_circlef(float* angle, float target,  float speed, float dt);
float mma_angle_betweenv2f(struct Vec2f a, struct Vec2f b);
void mma_normalize_or_y_axisv2f(struct Vec2f* v);
void mma_normalize_or_zerov2f(struct Vec2f* v);
float mma_dotv3f(struct Vec3f a, struct Vec3f b);
float mma_dotv2f(struct Vec2f a, struct Vec2f b);
float mma_normv3f(struct Vec3f vec);
float mma_normv2f(struct Vec2f vec);
void mma_normalize_or_z_axisv3f(struct Vec3f* v);
struct Vec3f mma_crossv3f(struct Vec3f a, struct Vec3f b);
struct Vec3f mma_addv3f(struct Vec3f a, struct Vec3f b);
struct Vec3f mma_subtractv3f(struct Vec3f a, struct Vec3f b);
struct Vec3f mma_multiplyfv3f(struct Vec3f a, float f);
bool mma_not_equalsv3f(struct Vec3f a, struct Vec3f b);
bool mma_equalsv3f(struct Vec3f a, struct Vec3f b);
void mma_add_inplacev3f(struct Vec3f a, struct Vec3f b);
void mma_subtract_inplacev3f(struct Vec3f a, struct Vec3f b);
struct Vec2f mma_addv2f(struct Vec2f a, struct Vec2f b);
struct Vec2f mma_subtractv2f(struct Vec2f a, struct Vec2f b);
struct Vec2f mma_mutliplyv2ff(struct Vec2f a, float f);
void mma_add_inplacev2f(struct Vec2f a, struct Vec2f b);
void mma_subtract_inplacev2f(struct Vec2f a, struct Vec2f b);
bool mma_not_equalsv2f(struct Vec2f a, struct Vec2f b);
bool mma_equalsv2f(struct Vec2f a, struct Vec2f b);
struct Mat4f mma_matrix_multiplyv4f(struct Mat4f *a, struct Mat4f *b);
struct Vec4f mma_matrix_multiply_rightmat4fv4f(struct Mat4f *m, struct Vec4f v);
struct Vec3f mma_matrix_multiply_rightmat4fv3f(struct Mat4f *m, struct Vec3f v);
float mma_matrix_detmat4f(struct Mat4f *m);


#endif // MMA_H_