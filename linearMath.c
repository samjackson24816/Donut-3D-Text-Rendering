
#include "mathUtils.h"
#include "linearMath.h"

Vec3 vec3fromVec2Int(Vec2Int v)
{
    return (Vec3){v.x, v.y, 0};
}

Vec2Int vec2IntfromVec3(Vec3 v)
{
    return (Vec2Int){(int)v.x, (int)v.y};
}

/**
 * Returns a 4d vector where w is 1
 */
Vec4 vec4fromVec3(Vec3 v)
{
    return (Vec4){v.x, v.y, v.z, 1};
}

Vec3 vec3fromVec4(Vec4 v)
{
    return (Vec3){v.x, v.y, v.z};
}

Vec3 addVec3(Vec3 a, Vec3 b)
{
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 multVec3(double n, Vec3 a)
{
    return (Vec3){
        a.x * n,
        a.y * n,
        a.z * n};
}

Vec3 negVec3(Vec3 a)
{
    return multVec3(-1, a);
}

double magVec3(Vec3 a)
{
    return sqrt((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
}

double dotVec3(Vec3 a, Vec3 b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

Vec3 crossVec3(Vec3 a, Vec3 b)
{
    Vec3 i = {1, 0, 0};
    Vec3 iDir = multVec3((a.y * b.z) - (a.z * b.y), i);

    Vec3 j = {0, 1, 0};
    Vec3 jDir = multVec3((a.z * b.x) - (a.x * b.z), j);

    Vec3 k = {0, 0, 1};
    Vec3 kDir = multVec3((a.x * b.y) - (a.y * b.x), k);

    return addVec3(addVec3(iDir, jDir), kDir);
}

Vec3 normVec3(Vec3 v)
{
    double m = magVec3(v);
    Vec3 n = {v.x / m, v.y / m, v.z / m};
    return n;
}

Vec4 multVec4byMatrix4x4(Matrix4x4 m, Vec4 v)
{
    Vec4 u =
        {
            (m.xx * v.x) + (m.xy * v.y) + (m.xz * v.z) + (m.xw * v.w),
            (m.yx * v.x) + (m.yy * v.y) + (m.yz * v.z) + (m.yw * v.w),
            (m.zx * v.x) + (m.zy * v.y) + (m.zz * v.z) + (m.zw * v.w),
            (m.wx * v.x) + (m.wy * v.y) + (m.wz * v.z) + (m.ww * v.w),
        };

    return u;
}

/**
 * Creates a matrix that rotates a vector "angleRad" degrees around the x axis
 */
Matrix4x4 xRotationMatrix(double angleRad)
{
    double a = angleRad;
    Matrix4x4 m = {
        1, 0, 0, 0,
        0, cos(a), -sin(a), 0,
        0, sin(a), cos(a), 0,
        0, 0, 0, 1};
    return m;
}

/**
 * Creates a matrix that rotates a vector "angleRad" degrees around the y axis
 */
Matrix4x4 yRotationMatrix(double angleRad)
{
    double a = angleRad;
    Matrix4x4 m = {
        cos(a), 0, sin(a), 0,
        0, 1, 0, 0,
        -sin(a), 0, cos(a), 0,
        0, 0, 0, 1};
    return m;
}

/**
 * Creates a matrix that rotates a vector "angleRad" degrees around the z axis
 */
Matrix4x4 zRotationMatrix(double angleRad)
{
    double a = angleRad;
    Matrix4x4 m = {
        cos(a), -sin(a), 0, 0,
        sin(a), cos(a), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1};
    return m;
}

Vec3 getDirection(double yawRad, double pitchRad)
{
    double clampedPitchRad = clamp(pitchRad, -PI / 3 + 0.01, PI / 2 - 0.01);

    Vec3 dir = vec3fromVec4(multVec4byMatrix4x4(zRotationMatrix(yawRad), multVec4byMatrix4x4(xRotationMatrix(pitchRad), (Vec4){0, 1, 0, 0})));

    return dir;
}
