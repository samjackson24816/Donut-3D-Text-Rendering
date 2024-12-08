#ifndef LINEAR_MATH_H // Header guard to prevent multiple inclusions
#define LINEAR_MATH_H

// Structs
typedef struct Vec2Int
{
    int x;
    int y;
} Vec2Int;

typedef struct Vec3
{
    double x;
    double y;
    double z;
} Vec3;

typedef struct Vec4
{
    double x;
    double y;
    double z;
    double w;
} Vec4;

typedef struct Matrix4x4
{
    double xx, xy, xz, xw;
    double yx, yy, yz, yw;
    double zx, zy, zz, zw;
    double wx, wy, wz, ww;
} Matrix4x4;

// Function prototypes
Vec3 normVec3(Vec3 v);
Vec3 crossVec3(Vec3 a, Vec3 b);
double dotVec3(Vec3 a, Vec3 b);
double magVec3(Vec3 a);
Vec3 negVec3(Vec3 a);
Vec3 multVec3(double n, Vec3 a);
Vec3 addVec3(Vec3 a, Vec3 b);
Vec3 vec3fromVec4(Vec4 v);
Vec4 vec4fromVec3(Vec3 v);
Matrix4x4 multMatrix4x4(Matrix4x4 a, Matrix4x4 b);
Matrix4x4 xRotationMatrix(double angleRad);
Matrix4x4 yRotationMatrix(double angleRad);
Matrix4x4 zRotationMatrix(double angleRad);
Vec3 getDirection(double yawRad, double pitchRad);
Vec4 multVec4byMatrix4x4(Matrix4x4 m, Vec4 v);

#endif