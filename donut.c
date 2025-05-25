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
#include "linearMath.h"
#include "mathUtils.h"

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

Vec3 cameraPos = {0,
                  -10,
                  0};

double cameraYawRad = 0;
double cameraPitchRad = 0;



Vec3 fpsControls(char input, double moveSpeed, double rotSpeed, double dt, Vec3 cameraPos)
{
    Vec3 camForward = getDirection(cameraYawRad, cameraPitchRad);

    Vec3 forwardDir = getDirection(cameraYawRad, 0);
    Vec3 rightDir = getDirection(cameraYawRad + PI / 2, 0);

    Vec3 forwardMove = multVec3(moveSpeed * dt, normVec3(forwardDir));
    Vec3 rightMove = multVec3(rotSpeed * dt, normVec3(rightDir));

    // First Person camera controls
    if (input == 'w')
    {
        cameraPos = addVec3(cameraPos, forwardMove);
    }
    else if (input == 's')
    {
        cameraPos = addVec3(cameraPos, negVec3(forwardMove));
    }
    else if (input == 'a')
    {
        cameraPos = addVec3(cameraPos, rightMove);
    }
    else if (input == 'd')
    {
        cameraPos = addVec3(cameraPos, negVec3(rightMove));
    }
    else if (input == 'q')
    {
        cameraPos.z += moveSpeed;
    }
    else if (input == 'e')
    {
        cameraPos.z -= moveSpeed;
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

    return cameraPos;
}

void update(double time, double dt, char input, int width, int height, double arr[width][height], double zBuff[width][height])
{
    cameraPos = fpsControls(input, 100, 2, dt, cameraPos);

    Vec3 cameraForward = getDirection(cameraYawRad, cameraPitchRad);

    const double DONUT_HOLE_DIAM = 2;
    const double DONUT_THICKNESS = 1;

    const double dRadius = (DONUT_HOLE_DIAM + 2 * DONUT_THICKNESS) / 2.0;

    const double cRadius = DONUT_THICKNESS / 2.0;

    for (double dAngle = 0; dAngle < 2 * PI; dAngle += 0.03)
    {
        Vec3 center = {sin(dAngle) * dRadius, 0, cos(dAngle) * dRadius};

        Vec3 right = {0, 1, 0};
        right = multVec3(cRadius, right);
        Vec3 out = multVec3(cRadius, normVec3(center));

        for (double cAngle = 0; cAngle < 2 * PI; cAngle += 0.03)
        {
            // Get the model vertex
            Vec3 oModel = addVec3(multVec3(sin(cAngle), out), multVec3(cos(cAngle), right));

            // Get the model normal vector
            Vec3 pModel = addVec3(center, oModel);

            // MODEL: Transform to get the world-relative vertex
            Vec3 oWorld = vec3fromVec4(multVec4byMatrix4x4(xRotationMatrix(time * 2), multVec4byMatrix4x4(yRotationMatrix(time * 0.1), multVec4byMatrix4x4(zRotationMatrix(time * 1.3), vec4fromVec3(oModel)))));

            // MODEL: Transform to get the world-relative normal vector
            Vec3 pWorld = vec3fromVec4(multVec4byMatrix4x4(xRotationMatrix(time * 2), multVec4byMatrix4x4(yRotationMatrix(time * 0.1), multVec4byMatrix4x4(zRotationMatrix(time * 1.3), vec4fromVec3(pModel)))));

            // Make a new vector basis
            Vec3 plusZ = {0, 0, 1};

            Vec3 c = cameraPos;
            Vec3 f = negVec3(normVec3(cameraForward));
            Vec3 r = normVec3(crossVec3(plusZ, f));
            Vec3 u = normVec3(crossVec3(f, r));

            // Make the inverse translation matrix of the camera
            Matrix4x4 translateMatrix = {
                1, 0, 0, -c.x,
                0, 1, 0, -c.y,
                0, 0, 1, -c.z,
                0, 0, 0, 1};

            // Make the inverse reorientation matrix of the camera
            Matrix4x4 orientMatrix = {
                r.x, r.y, r.z, 0,
                u.x, u.y, u.z, 0,
                f.x, f.y, f.z, 0,
                0, 0, 0, 1};

            // VIEW: Transform to get the camera-relative vertex
            Vec4 pCamera = multVec4byMatrix4x4(orientMatrix, multVec4byMatrix4x4(translateMatrix, vec4fromVec3(pWorld)));

            // Camera properties
            double near = 1;
            double halfWidth = 1;
            double halfHeight = 1;

            // Make the matrix that projects camera-relative space into clip space (what you see on the screen)
            Matrix4x4 projectionMatrix = {
                near / halfWidth, 0, 0, 0,
                0, near / halfHeight, 0, 0,
                0, 0, -1, -2 * near,
                0, 0, -1, 0};

            // PROJECTION: Transform to get the screen-relative vertex
            Vec4 pClipRaw = multVec4byMatrix4x4(projectionMatrix, pCamera);

            Vec3 pClip = {pClipRaw.x / pClipRaw.w, pClipRaw.y / pClipRaw.w, pClipRaw.z};

            // Convert to the coordinates of a pixel on the screen
            Vec2Int pScreen = worldToScreen(width, height, pClip);

            double zIndex = pClip.z;

            Vec3 lightSource = {-1, 0.5, 2};

            // The brightness of the point depends on how aligned its normal vector is with the light source
            double lum = (dotVec3(normVec3(oWorld), negVec3(normVec3(lightSource))));

            if (pScreen.x >= 0 && pScreen.x < width && pScreen.y >= 0 && pScreen.y < height)
            {
                // Check clipping
                if (zIndex > 0 && zIndex < zBuff[pScreen.x][pScreen.y])
                {
                    arr[pScreen.x][pScreen.y] = lum;
                    zBuff[pScreen.x][pScreen.y] = zIndex;
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
