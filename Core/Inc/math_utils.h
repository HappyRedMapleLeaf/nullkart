#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stdint.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE // fixes intellisense for M_PI. not actually necessary
#endif // _GNU_SOURCE

#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2

typedef struct {
   double x;
   double y;
   double z;
} Vec3;

typedef struct {
   double x;
   double y;
   double z;
   double w;
} Vec4;

typedef struct {
   Vec3 col1;
   Vec3 col2;
   Vec3 col3;
} Mat3;

typedef struct {
   Vec4 col1;
   Vec4 col2;
   Vec4 col3;
} Mat43;

double dot(Vec3 a, Vec3 b);
Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_scale(Vec3 a, double s);
double vec3_mag(Vec3 a);
Vec3 vec3_norm(Vec3 a);
Vec3 vec3_proj(Vec3 a, Vec3 b);
Vec3 vec3_proj_unit(Vec3 a, Vec3 b); // b must be a unit vector
Vec3 vec3_neg(Vec3 a);
Vec3 vec3_sub(Vec3 a, Vec3 b);
Vec3 vec3_cross(Vec3 a, Vec3 b);
Vec3 mat3_mul_vec3(Mat3 a, Vec3 b);

Mat3 mat3_mul(Mat3 a, Mat3 b);
Mat3 mat3_add(Mat3 a, Mat3 b);
Mat3 mat3_scale(Mat3 a, double s);
Mat3 mat3_transpose(Mat3 a);

Vec4 vec4_from_mat3(Mat3 a);
Vec4 vec4_scale(Vec4 q, float a);

Vec4 vec4_add(Vec4 a, Vec4 b);
Vec4 vec4_sub(Vec4 a, Vec4 b);
Vec4 vec4_mul(Vec4 a, Vec4 b); // hamilton product
Vec4 vec4_from_vec3(Vec3 a);
double vec4_mag(Vec4 a);

Vec4 mat43_mul_vec3(Mat43 a, Vec3 b);
Vec4 vec4_norm(Vec4 a);
Vec4 vec4_conj(Vec4 a);

Vec3 vec3_rot_by_vec4(Vec3 v, Vec4 q);
double tilt_angle(Vec4 q);

#endif // MATH_UTILS_H
