// https://gist.github.com/gcr/1075131

/*
To run this code:
gcc -o donut donut.c -lncurses -lm && ./donut
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <time.h>

#define PI 3.141593

void start()
{
    initscr();
    cbreak();
    curs_set(0);
    noecho();
    nodelay(stdscr, TRUE);
}

void end()
{
    endwin();
}

char getInput()

{
    char input = getch();

    char c;

    if (input == ERR)
    {
        // no actual input
        c = '$';
    }
    else
    {
        c = input;
    }

    return c;
}

double clamp(double a, double min, double max)
{
    if (a > max)
    {
        return max;
    }
    else if (a < min)
    {
        return min;
    }
    else
    {
        return a;
    };
}

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

typedef struct Vec2Int
{
    int x;
    int y;
} Vec2Int;

typedef struct Matrix4x4
{
    double xx, xy, xz, xw;
    double yx, yy, yz, yw;
    double zx, zy, zz, zw;
    double wx, wy, wz, ww;
} Matrix4x4;

Vec3 add(Vec3 a, Vec3 b)
{
    Vec3 v =
        {
            a.x + b.x,
            a.y + b.y,
            a.z + b.z};
    return v;
}

Vec3 mult(double n, Vec3 a)
{
    Vec3 v =
        {
            a.x * n,
            a.y * n,
            a.z * n};

    return v;
}

Vec3 neg(Vec3 a)
{
    return mult(-1, a);
}

double mag(Vec3 a)
{
    return sqrt((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
}

double dot(Vec3 a, Vec3 b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

Vec3 cross(Vec3 a, Vec3 b)
{
    // (a2 * b3 – a3 * b2) * i + (a3 * b1 – a1 * b3) * j + (a1 * b2 – a2 * b1) * k

    Vec3 i = {1, 0, 0};
    Vec3 iDir = mult((a.y * b.z) - (a.z * b.y), i);

    Vec3 j = {0, 1, 0};
    Vec3 jDir = mult((a.z * b.x) - (a.x * b.z), j);

    Vec3 k = {0, 0, 1};
    Vec3 kDir = mult((a.x * b.y) - (a.y * b.x), k);

    return add(add(iDir, jDir), kDir);
}

Vec3 norm(Vec3 v)
{
    double m = mag(v);
    Vec3 n = {v.x / m, v.y / m, v.z / m};
    return n;
}

Vec4 multM(Matrix4x4 m, Vec4 v)
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

/**
 * Converts a vector in projection space to a coordinate on the screen
 * Projection space is normalized from (-1, -1, -1) to (1, 1, 1).  Z is away from the camera, x is left, y is up.
 * But this doesn't use the z because that should be stored in the z buffer
 */
Vec2Int worldToScreen(int width, int height, Vec3 projVec)
{
    // Screen origin is top left
    double xScale = (projVec.x + 1) / 2.0;
    double yScale = (projVec.y + 1) / 2.0;

    int x = (int)round(xScale * (double)width);
    int y = (int)round(yScale * (double)height);

    Vec2Int v = {x, y};

    return v;
}

const int CHAR_MAP_SIZE = 6;
char CHAR_MAP[] = {' ', '.', '-', '*', '$', '#'};

void render(int width, int height, double arr[width][height])
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            double dot = arr[i][j];

            dot = clamp(dot, 0.0, 1.0);

            int charMapIndex = (int)(round(dot * (double)(CHAR_MAP_SIZE - 1)));
            char c = CHAR_MAP[charMapIndex];

            mvaddch(j + 1, (i * 2), c);
            mvaddch(j + 1, (i * 2) + 1, c);
        }
    }
    refresh();
}

Vec3 transformPoint(Vec3 worldPoint, Vec3 camPos, Vec3 cameraForward, double near, double halfWidth, double halfHeight, double width, double height)
{
    Vec3 plusZ = {0, 0, 1};

    Vec3 c = camPos;
    Vec3 f = neg(norm(cameraForward));
    Vec3 r = norm(cross(plusZ, f));
    Vec3 u = norm(cross(f, r));

    Matrix4x4 translateMatrix = {
        1, 0, 0, -c.x,
        0, 1, 0, -c.y,
        0, 0, 1, -c.z,
        0, 0, 0, 1};

    Matrix4x4 orientMatrix = {
        r.x, r.y, r.z, 0,
        u.x, u.y, u.z, 0,
        f.x, f.y, f.z, 0,
        0, 0, 0, 1};

    Matrix4x4 projectionMatrix = {
        near / halfWidth, 0, 0, 0,
        0, near / halfHeight, 0, 0,
        0, 0, -1, -2 * near,
        0, 0, -1, 0};

    Vec4 vWorld = {worldPoint.x, worldPoint.y, worldPoint.z, 1};

    Vec4 vClip = multM(projectionMatrix, multM(orientMatrix, multM(translateMatrix, vWorld)));

    Vec3 vScreen = {vClip.x / vClip.w, vClip.y / vClip.w, vClip.z};

    return vScreen;
}

Vec3 cameraPos = {0,
                  -10,
                  0};

double cameraYawRad = 0;
double cameraPitchRad = 0;

void update(double time, double dt, char input, int width, int height, double arr[width][height], double zBuff[width][height])
{
    // All the logic goes here

    double speed = 100 * dt;
    double rotSpeed = 1 * PI * 2.0 / 4.0 * dt;

    // We don't actually need the w component---I just don't want to make 3x3 matrices right now
    Vec4 initialCamDir = {0, 1, 0, 0};

    // Clamp the up and down angle so you can't flip over (make it so you can look almost straight up and almost straight down)
    cameraPitchRad = clamp(cameraPitchRad, -PI / 3 + 0.1, PI / 2 - 0.1);

    // Rotate around horizontally (z axis)
    // Then, rotate up and down (x axis)

    Vec4 camForward4 = multM(zRotationMatrix(cameraYawRad), multM(xRotationMatrix(cameraPitchRad), initialCamDir));

    Vec3 cameraForward = {camForward4.x, camForward4.y, camForward4.z};

    Vec4 moveForward4 = multM(zRotationMatrix(cameraYawRad), initialCamDir);
    Vec4 moveRight4 = multM(zRotationMatrix(cameraYawRad + PI / 2.0), initialCamDir);

    Vec3 forwardDir = {moveForward4.x, moveForward4.y, moveForward4.z};
    Vec3 rightDir = {moveRight4.x, moveRight4.y, moveRight4.z};

    Vec3 forwardMove = mult(speed, norm(forwardDir));
    Vec3 rightMove = mult(speed, norm(rightDir));

    // First Person camera controls
    if (input == 'w')
    {
        cameraPos = add(cameraPos, forwardMove);
    }
    else if (input == 's')
    {
        cameraPos = add(cameraPos, neg(forwardMove));
    }
    else if (input == 'a')
    {
        cameraPos = add(cameraPos, rightMove);
    }
    else if (input == 'd')
    {
        cameraPos = add(cameraPos, neg(rightMove));
    }
    else if (input == 'q')
    {
        cameraPos.z += speed;
    }
    else if (input == 'e')
    {
        cameraPos.z -= speed;
    }
    else if (input == 'l')
    {
        cameraYawRad -= rotSpeed;
    }
    else if (input == 'j')
    {
        cameraYawRad += rotSpeed;
    }
    else if (input == 'i')
    {
        cameraPitchRad -= rotSpeed;
    }
    else if (input == 'k')
    {
        cameraPitchRad += rotSpeed;
    }

    const double DONUT_HOLE_DIAM = 2;
    const double DONUT_THICKNESS = 1;

    const double dRadius = (DONUT_HOLE_DIAM + 2 * DONUT_THICKNESS) / 2.0;

    const double cRadius = DONUT_THICKNESS / 2.0;

    for (double dAngle = 0; dAngle < 2 * PI; dAngle += 0.05)
    {
        Vec3 center = {sin(dAngle) * dRadius, 0, cos(dAngle) * dRadius};

        Vec3 right = {0, 1, 0};
        right = mult(cRadius, right);
        Vec3 out = mult(cRadius, norm(center));

        for (double cAngle = 0; cAngle < 2 * PI; cAngle += 0.05)
        {

            Vec3 offset = add(mult(sin(cAngle), out), mult(cos(cAngle), right));

            Vec3 pModel = add(center, offset);

            // Spin around the whole donut

            Vec4 p4 = {pModel.x, pModel.y, pModel.z, 1};

            Vec4 o4 = {offset.x, offset.y, offset.z, 1};

            p4 = multM(xRotationMatrix(time * 2), multM(yRotationMatrix(time * 0.1), multM(zRotationMatrix(time * 1.3), p4)));

            o4 = multM(xRotationMatrix(time * 2), multM(yRotationMatrix(time * 0.1), multM(zRotationMatrix(time * 1.3), o4)));

            Vec3 p = {p4.x, p4.y, p4.z};

            Vec3 o = {o4.x, o4.y, o4.z};

            Vec3 vScreen = transformPoint(p, cameraPos, cameraForward, 1, 0.5, 0.5, width, height);

            Vec2Int screenPoint = worldToScreen(width, height, vScreen);

            double zIndex = vScreen.z;

            Vec3 lightSource = {-1, 0.5, 2};

            double lum = (dot(norm(o), neg(norm(lightSource)))) / 1;

            if (screenPoint.x >= 0 && screenPoint.x < width && screenPoint.y >= 0 && screenPoint.y < height)
            {
                // Check clipping
                if (zIndex > 0 && zIndex < zBuff[screenPoint.x][screenPoint.y])
                {
                    arr[screenPoint.x][screenPoint.y] = lum;
                    zBuff[screenPoint.x][screenPoint.y] = zIndex;
                }
            }
        }
    }
}

int main()
{
    int width = 80;
    int height = 80;
    double arr[width][height];
    double zBuff[width][height];

    start();

    clock_t start, current;

    start = clock();

    while (true)
    {
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {

                arr[i][j] = 0;
                zBuff[i][j] = 9999999;
            }
        }

        char c = getInput();

        double dt = (double)(clock() - current) / CLOCKS_PER_SEC;

        current = clock();

        update((double)(current - start) / CLOCKS_PER_SEC, dt, c, width, height, arr, zBuff);

        render(width, height, arr);
    }

    end();

    return 0;
}
