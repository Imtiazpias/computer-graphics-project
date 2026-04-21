#include <GL/glut.h>
#include <cmath>

// external brightness function from main
extern float br();

// ==========================
// STRUCT
// ==========================
struct Pt { float x, y; };

// ==========================
// COSINE INTERPOLATION
// ==========================
float cosInterp(float a, float b, float t) {
    float f = (1 - cos(t * 3.14159f)) * 0.5f;
    return a * (1 - f) + b * f;
}

float hillY(Pt* pk, int n, float x, float base) {
    for (int i = 0; i < n - 1; i++) {
        if (x >= pk[i].x && x <= pk[i + 1].x) {
            float t = (x - pk[i].x) / (pk[i + 1].x - pk[i].x);
            return cosInterp(pk[i].y, pk[i + 1].y, t);
        }
    }
    return base;
}

// ==========================
// DRAW HILL LAYER
// ==========================
void drawHillLayer(Pt* pk, int n, float base, float r, float g, float b) {
    float f = br();
    glColor3f(r * f, g * f, b * f);

    glBegin(GL_POLYGON);
    glVertex2f(-1.0f, base);

    for (int i = 0; i <= 400; i++) {
        float x = -1.0f + i * (2.0f / 400);
        glVertex2f(x, hillY(pk, n, x, base));
    }

    glVertex2f(1.0f, base);
    glEnd();
}

// ==========================
// HILL PEAK DATA
// ==========================
Pt farPk[] = {
    {-1.2f,0.60f},{-0.85f,0.78f},{-0.45f,0.82f},
    {0.00f,0.88f},{0.44f,0.80f},{0.84f,0.74f},{1.2f,0.60f}
};

Pt midPk[] = {
    {-1.2f,0.55f},{-0.80f,0.64f},{-0.36f,0.68f},
    {0.10f,0.72f},{0.52f,0.66f},{0.88f,0.60f},{1.2f,0.55f}
};

Pt nearPk[] = {
    {-1.2f,0.50f},{-0.75f,0.52f},{-0.30f,0.54f},
    {0.14f,0.55f},{0.56f,0.52f},{0.91f,0.48f},{1.2f,0.50f}
};

// ==========================
// MAIN HILLS FUNCTION
// ==========================
void drawHills(float HILL_BASE) {
    drawHillLayer(farPk, 7, HILL_BASE, 0.20f, 0.25f, 0.35f);
    drawHillLayer(midPk, 7, HILL_BASE, 0.10f, 0.25f, 0.09f);
    drawHillLayer(nearPk,7, HILL_BASE, 0.07f, 0.20f, 0.06f);
}