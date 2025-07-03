#include "math_utils.h"
#include <math.h>

double dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 vec3_add(Vec3 a, Vec3 b) {
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 vec3_scale(Vec3 a, double s) {
    return (Vec3){a.x * s, a.y * s, a.z * s};
}

double vec3_mag(Vec3 a) {
    return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

Vec3 vec3_norm(Vec3 a) {
    return vec3_scale(a, 1.0 / vec3_mag(a));
}

Vec3 vec3_proj(Vec3 a, Vec3 b) {
    return vec3_scale(b, dot(a, b) / dot(b, b));
}

// b must be a unit vector
Vec3 vec3_proj_unit(Vec3 a, Vec3 b) {
    return vec3_scale(b, dot(a, b));
}

Vec3 vec3_neg(Vec3 a) {
    return (Vec3){-a.x, -a.y, -a.z};
}

Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return (Vec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

Mat3 mat3_mul(Mat3 a, Mat3 b) {
    Vec3 col1 = {dot((Vec3){a.col1.x, a.col2.x, a.col3.x}, b.col1),
                 dot((Vec3){a.col1.y, a.col2.y, a.col3.y}, b.col1),
                 dot((Vec3){a.col1.z, a.col2.z, a.col3.z}, b.col1)};
    Vec3 col2 = {dot((Vec3){a.col1.x, a.col2.x, a.col3.x}, b.col2),
                 dot((Vec3){a.col1.y, a.col2.y, a.col3.y}, b.col2),
                 dot((Vec3){a.col1.z, a.col2.z, a.col3.z}, b.col2)};
    Vec3 col3 = {dot((Vec3){a.col1.x, a.col2.x, a.col3.x}, b.col3),
                 dot((Vec3){a.col1.y, a.col2.y, a.col3.y}, b.col3),
                 dot((Vec3){a.col1.z, a.col2.z, a.col3.z}, b.col3)};
    return (Mat3){col1, col2, col3};
}

Vec3 mat3_mul_vec3(Mat3 a, Vec3 b) {
    return (Vec3){dot((Vec3){a.col1.x, a.col2.x, a.col3.x}, b),
                  dot((Vec3){a.col1.y, a.col2.y, a.col3.y}, b),
                  dot((Vec3){a.col1.z, a.col2.z, a.col3.z}, b)};
}

Vec4 mat43_mul_vec3(Mat43 a, Vec3 b) {
    return (Vec4){dot((Vec3){a.col1.x, a.col2.x, a.col3.x}, b),
                   dot((Vec3){a.col1.y, a.col2.y, a.col3.y}, b),
                   dot((Vec3){a.col1.z, a.col2.z, a.col3.z}, b),
                   dot((Vec3){a.col1.w, a.col2.w, a.col3.w}, b)};
}

Mat3 mat3_add(Mat3 a, Mat3 b) {
    Vec3 col1 = {a.col1.x + b.col1.x, a.col1.y + b.col1.y, a.col1.z + b.col1.z};
    Vec3 col2 = {a.col2.x + b.col2.x, a.col2.y + b.col2.y, a.col2.z + b.col2.z};
    Vec3 col3 = {a.col3.x + b.col3.x, a.col3.y + b.col3.y, a.col3.z + b.col3.z};
    return (Mat3){col1, col2, col3};
}

Mat3 mat3_scale(Mat3 a, double s) {
    Vec3 col1 = {a.col1.x * s, a.col1.y * s, a.col1.z * s};
    Vec3 col2 = {a.col2.x * s, a.col2.y * s, a.col2.z * s};
    Vec3 col3 = {a.col3.x * s, a.col3.y * s, a.col3.z * s};
    return (Mat3){col1, col2, col3};
}

Mat3 mat3_transpose(Mat3 a) {
    Vec3 col1 = {a.col1.x, a.col2.x, a.col3.x};
    Vec3 col2 = {a.col1.y, a.col2.y, a.col3.y};
    Vec3 col3 = {a.col1.z, a.col2.z, a.col3.z};
    return (Mat3){col1, col2, col3};
}

Vec4 vec4_scale(Vec4 q, float a) {
    return (Vec4){q.x * a, q.y * a, q.z * a, q.w * a};
}

Vec4 vec4_from_mat3(Mat3 a) {
    // https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
    // they tell us that their formula uses transposed matrix. I'm too lazy to do it manually for now...
    a = mat3_transpose(a);

    float t;
    Vec4 q;

    if (a.col2.y < 0) {
        if (a.col1.x > a.col2.y) {
            t = 1 + a.col1.x - a.col2.y - a.col3.z;
            q = (Vec4){t, a.col2.x + a.col1.y, a.col1.z + a.col3.x, a.col3.y - a.col2.z};
        } else {
            t = 1 - a.col1.x + a.col2.y - a.col3.z;
            q = (Vec4){a.col2.x + a.col1.y, t, a.col3.y + a.col2.z, a.col1.z - a.col3.x};
        }
    } else {
        if (a.col1.x < -a.col2.y) {
            t = 1 - a.col1.x - a.col2.y + a.col3.z;
            q = (Vec4){a.col1.z + a.col3.x, a.col3.y + a.col2.z, t, a.col2.x - a.col1.y};
        } else {
            t = 1 + a.col1.x + a.col2.y + a.col3.z;
            q = (Vec4){a.col3.y - a.col2.z, a.col1.z - a.col3.x, a.col2.x - a.col1.y, t};
        }
    }
    q = vec4_scale(q, 0.5 / sqrt(t));

    return q;
}

Vec4 vec4_add(Vec4 a, Vec4 b) {
    return (Vec4){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

Vec4 vec4_sub(Vec4 a, Vec4 b) {
    return (Vec4){a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

Vec4 vec4_mul(Vec4 a, Vec4 b) {
    // https://en.wikipedia.org/wiki/Quaternion#Hamilton_product
    return (Vec4){a.x * b.x - a.y * b.y - a.z * b.z - a.w * b.w,
                  a.x * b.y + a.y * b.x + a.z * b.w - a.w * b.z,
                  a.x * b.z - a.y * b.w + a.z * b.x + a.w * b.y,
                  a.x * b.w + a.y * b.z - a.z * b.y + a.w * b.x};
}

Vec4 vec4_from_vec3(Vec3 a) {
    return (Vec4){0, a.x, a.y, a.z};
}

double vec4_mag(Vec4 a) {
    return sqrt(a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w);
}

Vec4 vec4_norm(Vec4 a) {
    return vec4_scale(a, 1.0 / vec4_mag(a));
}

Vec4 vec4_conj(Vec4 a) {
    return (Vec4){a.x, -a.y, -a.z, -a.w};
}

Vec3 vec3_rot_by_vec4(Vec3 v, Vec4 q) {
    Vec4 q_conj = vec4_conj(q);
    Vec4 temp = vec4_mul(q, vec4_from_vec3(v));
    Vec4 rotated = vec4_mul(temp, q_conj);

    Vec3 result = {rotated.y, rotated.z, rotated.w};
    return result;
}

// gpt got me frfr (im actually the best prompt engineer of this generation)
// WILL NEED TO CHANGE once we do wheelie stuff because atan2 only works if x axis is approx horizontal
double tilt_angle(Vec4 q) {
    // Neutral axes
    Vec3 z_neutral = {0.0, 0.0, -1.0};  // gravity down
    Vec3 x_neutral = {1.0, 0.0, 0.0};   // axis of rotation

    // Rotate z_neutral and x_neutral
    Vec3 z_current = vec3_rot_by_vec4(z_neutral, q);
    Vec3 x_current = vec3_rot_by_vec4(x_neutral, q);

    // Extract y and z components of the rotated z vector (project onto YZ plane)
    double y = z_current.y;
    double z = z_current.z;

    // Use atan2 for signed angle in YZ plane (around x)
    double angle = atan2(y, -z);  // compare against (0, 0, -1)

    return angle;
}
