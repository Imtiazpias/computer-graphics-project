// ============================================================
//  Interactive Village Environment Simulation
//  Scene Layout (y-axis):
//    Top    (+1.0 to +0.35) → Sky + Hills
//    Middle (+0.35 to  0.0) → River
//    Bottom ( 0.0 to -1.0)  → Green Field (Village + Playground)
// ============================================================

#include <GL/glut.h>
#include <ctime>
#include <iostream>
#include <cmath>
using namespace std;

// ── Zone boundaries ──────────────────────────────────────────
const float HILL_BASE  =  0.35f;  // sky ends / river starts here
const float RIVER_BOT  =  0.00f;  // river ends / green field starts here

// ── Animation state variables ────────────────────────────────
float sunY      = -0.10f;   // sun Y position (rises upward over time)
bool  isDay     = true;     // true = day mode, false = night mode

float waveTime  =  0.00f;   // global time counter for wave/animation

float boatX     = -0.88f;   // boat moves left → right in river
float birdX     = -1.20f;   // birds fly left → right in sky
float cloudX    =  0.15f;   // cloud 1 drifts right
float cloud2X   = -0.55f;   // cloud 2 drifts right (different speed)

float manX      =  0.0f;    // man walks up the center road
float manY      = -0.90f;

float cowX1     = -0.90f;   // cow 1 grazes slowly rightward
float cowX2     = -0.75f;   // cow 2 grazes at different speed
float cowY1     = -0.70f;
float cowY2     = -0.78f;

float butterflyX =  0.20f;  // butterfly moves across playground
float butterflyT =  0.00f;  // butterfly wing-flap timer

float childT    =  0.00f;   // animation timer for children running

// 6 football players; playerDir = movement direction (+1 or -1)
float playerX[6]   = {0.25f, 0.35f, 0.45f, 0.55f, 0.70f, 0.85f};
float playerDir[6] = {1, -1, 1, -1, 1, -1};

float ballX      = 0.50f;   // football position
float ballY      = -0.55f;
int   targetPlayer = 0;     // which player the ball moves toward

float fishT      =  0.00f;  // fish jump timer
float duckX1     = -0.78f;  // duck 1 in river
float duckX2     = -0.50f;  // duck 2 in river

// ── 2D / 3D Transformation variables ────────────────────────
float transX = 0, transY = 0;   // 2D translation (arrow keys)
float scaleS = 1.0f;            // 2D scale (+/- keys)
float rotY   = 0, rotX = 0;     // 3D rotation angles (R/T keys)


// ============================================================
//  BRIGHTNESS HELPER
//  Returns a value 0.07 (night) to 1.0 (day)
//  based on sun's Y position.
//  Multiply all colors by br() for realistic day/night effect.
// ============================================================
float br() {
    float b = (sunY + 0.10f) / 0.85f;
    if (b < 0.07f) b = 0.07f;   // minimum brightness at night
    if (b > 1.0f)  b = 1.0f;
    return b;
}

// Shorthand: set color with brightness applied
void col(float r, float g, float b) {
    float f = br();
    glColor3f(r * f, g * f, b * f);
}


// ============================================================
//  ALGORITHM 1 – DDA LINE DRAWING
//
//  DDA = Digital Differential Analyzer
//  Idea: Find the larger of dx or dy → use it as the number
//        of steps. Each step, increment x and y by small
//        fractions (xi = dx/steps, yi = dy/steps).
//  Used for: sun rays, man's legs, butterfly antenna, waves
// ============================================================
void ddaLine(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;

    // Number of steps = max of |dx| or |dy|, scaled to screen pixels
    int steps = (int)((fabs(dx) > fabs(dy) ? fabs(dx) : fabs(dy)) * 900);
    if (!steps) return;

    float xi = dx / steps;  // x increment per step
    float yi = dy / steps;  // y increment per step

    float x = x1, y = y1;

    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; i++) {
        glVertex2f(x, y);
        x += xi;
        y += yi;
    }
    glEnd();
}


// ============================================================
//  ALGORITHM 2 – BRESENHAM'S LINE DRAWING
// ============================================================
void bLine(float fx1, float fy1, float fx2, float fy2) {
    // Convert float coords → integer pixel coords
    int x1 = (int)(fx1 * 900), y1 = (int)(fy1 * 900);
    int x2 = (int)(fx2 * 900), y2 = (int)(fy2 * 900);

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;   // step direction for x
    int sy = (y1 < y2) ? 1 : -1;   // step direction for y
    int e  = dx - dy;               // initial error

    glBegin(GL_POINTS);
    for (;;) {
        glVertex2f(x1 / 900.0f, y1 / 900.0f);
        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * e;
        if (e2 > -dy) { e -= dy; x1 += sx; }
        if (e2 <  dx) { e += dx; y1 += sy; }
    }
    glEnd();
}


// ============================================================
//  ALGORITHM 3 – MID-POINT CIRCLE DRAWING
// ============================================================
void plotPts(float cx, float cy, int px, int py, float sc) {
    // Mirror one point to all 8 symmetric octants
    glVertex2f(cx + px*sc, cy + py*sc);
    glVertex2f(cx - px*sc, cy + py*sc);
    glVertex2f(cx + px*sc, cy - py*sc);
    glVertex2f(cx - px*sc, cy - py*sc);
    glVertex2f(cx + py*sc, cy + px*sc);
    glVertex2f(cx - py*sc, cy + px*sc);
    glVertex2f(cx + py*sc, cy - px*sc);
    glVertex2f(cx - py*sc, cy - px*sc);
}

void mpCircle(float cx, float cy, float r) {
    float sc = r / 100.0f;  // scale factor to convert pixel coords → GL coords
    int x = 0, y = 100;
    int d = 1 - 100;        // initial decision parameter d = 1 - R

    glBegin(GL_POINTS);
    plotPts(cx, cy, x, y, sc);

    while (x < y) {
        if (d < 0)
            d += 2*x + 3;          // move east
        else {
            d += 2*(x - y) + 5;    // move southeast
            y--;
        }
        x++;
        plotPts(cx, cy, x, y, sc);
    }
    glEnd();
}

// Fill a circle by drawing many concentric circles (radius shrinks by 0.002 each time)
void fillCircle(float cx, float cy, float r) {
    for (float ri = r; ri > 0.002f; ri -= 0.002f)
        mpCircle(cx, cy, ri);
}


// ============================================================
//  COSINE INTERPOLATION FOR SMOOTH HILLS
// ============================================================
struct Pt { float x, y; };

float cosInterp(float a, float b, float t) {
    float f = (1 - cos(t * 3.14159f)) * 0.5f;
    return a * (1 - f) + b * f;
}

// Given a set of hill peak points, find the y-value at any x
float hillY(Pt* pk, int n, float x) {
    for (int i = 0; i < n - 1; i++) {
        if (x >= pk[i].x && x <= pk[i+1].x) {
            float t = (x - pk[i].x) / (pk[i+1].x - pk[i].x);
            return cosInterp(pk[i].y, pk[i+1].y, t);
        }
    }
    return HILL_BASE;
}

// ── Hill peak data (3 layers: far, mid, near) ────────────────
// Each layer is a series of (x, y) control points
// Far layer is highest / smallest, near layer is lowest / biggest

Pt farPk[] = {
    {-1.2f,0.60f},{-1.0f,0.65f},{-0.85f,0.78f},{-0.65f,0.55f},
    {-0.45f,0.82f},{-0.22f,0.60f},{0.00f,0.88f},{0.22f,0.62f},
    {0.44f,0.80f},{0.65f,0.55f},{0.84f,0.74f},{1.0f,0.65f},{1.2f,0.60f}
};
int FN = 13;

Pt midPk[] = {
    {-1.2f,0.55f},{-1.0f,0.58f},{-0.80f,0.64f},{-0.58f,0.44f},
    {-0.36f,0.68f},{-0.12f,0.46f},{0.10f,0.72f},{0.30f,0.48f},
    {0.52f,0.66f},{0.72f,0.42f},{0.88f,0.60f},{1.0f,0.58f},{1.2f,0.55f}
};
int MN = 13;

Pt nearPk[] = {
    {-1.2f,0.50f},{-1.0f,0.52f},{-0.75f,0.52f},{-0.52f,0.37f},
    {-0.30f,0.54f},{-0.08f,0.38f},{0.14f,0.55f},{0.35f,0.36f},
    {0.56f,0.52f},{0.76f,0.35f},{0.91f,0.48f},{1.0f,0.52f},{1.2f,0.50f}
};
int NN = 13;


// Draw one hill layer as a filled polygon, then add forest on top
void drawHillLayer(Pt* pk, int n, float r, float g, float b2) {
    float f = br();
    glColor3f(r*f, g*f, b2*f);

    glBegin(GL_POLYGON);
    glVertex2f(-1.0f, HILL_BASE);           // left base corner
    for (int i = 0; i <= 400; i++) {        // 400 steps across width
        float x = -1.0f + i * (2.0f / 400);
        glVertex2f(x, hillY(pk, n, x));     // top follows hill curve
    }
    glVertex2f(1.0f, HILL_BASE);            // right base corner
    glEnd();
}

// Plant trees on a hill surface at given x positions
void hillForest(Pt* pk, int n, float* xs, int cnt, float sz,
                float r, float g, float b2) {
    float f = br();
    for (int i = 0; i < cnt; i++) {
        float tx = xs[i];
        float ty = hillY(pk, n, tx);   // tree base sits ON the hill

        // Trunk
        glColor3f(0.15f*f, 0.08f*f, 0.02f*f);
        glPointSize(3.0f);
        bLine(tx, ty, tx, ty + sz * 0.5f);
        glPointSize(1.0f);

        // 3 overlapping circles = tree canopy
        glColor3f(r*f, g*f, b2*f);
        fillCircle(tx,          ty + sz*0.9f,  sz*0.85f);
        fillCircle(tx - sz*0.5f, ty + sz*0.6f, sz*0.70f);
        fillCircle(tx + sz*0.5f, ty + sz*0.65f,sz*0.70f);

        // Slightly lighter top circle
        glColor3f((r+0.04f)*f, (g+0.05f)*f, (b2+0.03f)*f);
        fillCircle(tx, ty + sz*1.2f, sz*0.50f);
    }
}


// ============================================================
//  ZONE 1: SKY  (y = HILL_BASE to +1.0)
// ============================================================

// Draw gradient sky: dark at top → lighter near horizon
void drawSky() {
    float f = br();
    glBegin(GL_QUADS);
        // Top of sky (darker blue at night, bright blue in day)
        glColor3f(0.01f + 0.20f*f, 0.01f + 0.42f*f, 0.06f + 0.66f*f);
        glVertex2f(-1, 1); glVertex2f(1, 1);

        // Near horizon (slightly lighter)
        glColor3f(0.04f + 0.48f*f, 0.04f + 0.58f*f, 0.09f + 0.68f*f);
        glVertex2f(1, HILL_BASE); glVertex2f(-1, HILL_BASE);
    glEnd();
}

// Stars appear only at night (when brightness < 0.65)
void drawStars() {
    float f = br();
    if (f > 0.65f) return;  // don't draw stars in daytime

    float starFade = 1.0f - f / 0.65f;  // stars fade as day comes
    glColor3f(starFade, starFade, starFade);
    glPointSize(2.0f);

    // Fixed star positions
    float sx[] = {-0.9f,-0.7f,-0.5f,-0.3f,-0.1f,0.1f,0.3f,0.5f,0.7f,0.9f,
                  -0.8f,-0.6f,-0.4f,-0.2f,0.0f,0.2f,0.4f,0.6f,0.8f};
    float sy[] = {0.85f,0.78f,0.92f,0.75f,0.88f,0.80f,0.90f,0.76f,0.84f,0.79f,
                  0.70f,0.82f,0.74f,0.86f,0.77f,0.89f,0.73f,0.83f,0.71f};

    glBegin(GL_POINTS);
    for (int i = 0; i < 19; i++)
        glVertex2f(sx[i], sy[i]);
    glEnd();
    glPointSize(1.0f);
}

// Moon: visible only at night, crescent shape = two overlapping circles
void drawMoon() {
    float f = br();
    if (f > 0.65f) return;  // hide moon in daytime

    float mx = 0.75f, my = 0.80f;  // moon position

    // Full white circle
    glColor3f(1.0f, 1.0f, 1.0f);
    fillCircle(mx, my, 0.055f);

    // Cover part of it with sky color → creates crescent
    float skyR = 0.01f + 0.20f*f;
    float skyG = 0.01f + 0.42f*f;
    float skyB = 0.06f + 0.66f*f;
    glColor3f(skyR, skyG, skyB);
    fillCircle(mx + 0.030f, my, 0.050f);  // shifted right to make crescent
}

// Sun: rises from behind far hills; glowing yellow-red core with rays
void drawSun() {
    float f = br();
    float sunX = -0.25f;

    // Don't let sun go below the hill it rises behind
    float visY = sunY;
    float hillTop = hillY(farPk, FN, sunX);
    if (visY < hillTop - 0.08f)
        visY = hillTop - 0.08f;

    // Glow rings: red on outside → yellow toward center
    for (float ri = 0.12f; ri > 0.05f; ri -= 0.01f) {
        float t = (ri - 0.05f) / (0.12f - 0.05f);
        float g = 0.3f + 0.6f * (1.0f - t);
        glColor3f(1.0f*f, g*f, 0.0f);
        mpCircle(sunX, visY, ri);
    }

    // Bright yellow core
    glColor3f(1.0f*f, 0.9f*f, 0.1f*f);
    fillCircle(sunX, visY, 0.07f);

    // Animated rays (using DDA line, slight wobble with sin)
    glColor3f(1.0f*f, 0.85f*f, 0.2f*f);
    glPointSize(1.5f);
    for (int i = 0; i < 15; i++) {
        float angle = i * 36 * 3.14159f / 180.0f;  // 10 rays, 36° apart
        float len = 0.08f + 0.01f * sin(waveTime + i);  // slight wobble
        ddaLine(sunX, visY,
                sunX + len * cos(angle),
                visY  + len * sin(angle));
    }
    glPointSize(1.0f);
}

// Cloud: made of 5 overlapping circles
void drawCloud(float x, float y) {
    float f = br();
    float w = 0.90f*f + 0.10f;  // clouds are bright in day, dim at night
    glColor3f(w, w, w);
    fillCircle(x,          y,          0.065f);
    fillCircle(x + 0.078f, y + 0.024f, 0.050f);
    fillCircle(x - 0.072f, y + 0.016f, 0.048f);
    fillCircle(x + 0.140f, y - 0.004f, 0.038f);
    fillCircle(x - 0.135f, y - 0.006f, 0.033f);
}

// Birds: 5 simple V-shapes using DDA lines
void drawBirds(float bx, float by) {
    col(0.04f, 0.04f, 0.04f);
    glPointSize(2.0f);
    for (int i = 0; i < 5; i++) {
        float ox = bx + i * 0.082f;
        float oy = by + (i % 2) * 0.024f;
        ddaLine(ox - 0.022f, oy + 0.014f, ox, oy);       // left wing
        ddaLine(ox, oy, ox + 0.022f, oy + 0.014f);       // right wing
    }
    glPointSize(1.0f);
}

// Draw hills: 3 layers (far → near), each with forest on top
void drawHills() {
    // Layer 1 – farthest: blue-grey (atmospheric perspective)
    drawHillLayer(farPk, FN, 0.20f, 0.25f, 0.35f);
    float ft[] = {-0.95f,-0.75f,-0.50f,-0.25f,0.0f,0.25f,0.50f,0.75f,0.95f};
    hillForest(farPk, FN, ft, 9, 0.030f, 0.08f, 0.16f, 0.06f);

    // Layer 2 – middle: dark forest green
    drawHillLayer(midPk, MN, 0.10f, 0.25f, 0.09f);
    float mt[] = {-0.72f,-0.46f,-0.20f,0.06f,0.28f,0.54f,0.76f};
    hillForest(midPk, MN, mt, 7, 0.038f, 0.06f, 0.18f, 0.05f);

    // Layer 3 – nearest: darkest green (appears largest, closest)
    drawHillLayer(nearPk, NN, 0.07f, 0.20f, 0.06f);
    float nt[] = {-0.68f,-0.44f,-0.18f,0.06f,0.26f,0.48f,0.70f,0.88f};
    hillForest(nearPk, NN, nt, 8, 0.044f, 0.04f, 0.14f, 0.03f);
}


// ============================================================
//  ZONE 2: RIVER  (y = RIVER_BOT to HILL_BASE = 0.0 to 0.35)
// ============================================================

void drawRiver() {
    float f  = br();
    float rMid = (HILL_BASE + RIVER_BOT) * 0.5f;  // middle of river = 0.175

    // Top grass bank (thin strip just below hills)
    col(0.16f, 0.48f, 0.09f);
    glBegin(GL_QUADS);
        glVertex2f(-1, HILL_BASE);
        glVertex2f( 1, HILL_BASE);
        glVertex2f( 1, HILL_BASE - 0.02f);
        glVertex2f(-1, HILL_BASE - 0.02f);
    glEnd();

    // Water body (blue)
    glColor3f(0.06f + 0.12f*f, 0.25f + 0.32f*f, 0.54f + 0.24f*f);
    glBegin(GL_QUADS);
        glVertex2f(-1, RIVER_BOT);
        glVertex2f( 1, RIVER_BOT);
        glVertex2f( 1, HILL_BASE - 0.02f);
        glVertex2f(-1, HILL_BASE - 0.02f);
    glEnd();

    // Top shoreline: wavy grass edge (sin wave)
    col(0.16f, 0.48f, 0.09f);
    glBegin(GL_POLYGON);
    glVertex2f(-1, HILL_BASE);
    for (int j = 0; j <= 130; j++) {
        float wx = -1.0f + j * (2.0f / 130);
        float wy = HILL_BASE - 0.02f
                 + 0.022f * sin(wx * 8.0f + waveTime * 0.7f)
                 + 0.009f * sin(wx * 16.0f - waveTime);
        glVertex2f(wx, wy);
    }
    glVertex2f(1, HILL_BASE);
    glEnd();

    // Bottom shoreline: wavy grass edge
    col(0.18f, 0.50f, 0.10f);
    glBegin(GL_POLYGON);
    glVertex2f(-1, RIVER_BOT);
    for (int j = 0; j <= 130; j++) {
        float wx = -1.0f + j * (2.0f / 130);
        float wy = RIVER_BOT + 0.002f
                 - 0.018f * sin(wx * 7.5f + waveTime * 0.6f + 2.0f)
                 - 0.008f * sin(wx * 15.0f - waveTime * 0.8f);
        glVertex2f(wx, wy);
    }
    glVertex2f(1, RIVER_BOT);
    glEnd();

    // Wave rows across the water (sine wave strips)
    for (int i = 0; i < 12; i++) {
        float wy  = HILL_BASE - 0.04f - i * ((HILL_BASE - RIVER_BOT - 0.05f) / 12.0f);
        float ph  = waveTime * 1.4f + i * 0.65f;
        float amp = 0.004f + 0.003f * sin(i * 1.3f);
        float dep = 1.0f - i * 0.06f;
        if (dep < 0.28f) dep = 0.28f;

        glColor3f(0.20f*f*dep, 0.55f*f*dep, 0.88f*f*dep);
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j <= 120; j++) {
            float wx  = -1.0f + j * (2.0f / 120);
            float wvy = wy + amp * sin(wx * 20.0f + ph)
                           + amp * 0.5f * sin(wx * 32.0f - ph * 0.7f);
            glVertex2f(wx, wvy);
        }
        glEnd();
    }

    // Shimmer: short DDA lines that glisten on the water
    glColor3f(0.60f*f, 0.78f*f, 0.95f*f);
    glPointSize(1.5f);
    for (int i = 0; i < 8; i++) {
        float rx = -0.85f + i * 0.22f + 0.05f * sin(waveTime + i * 1.6f);
        float ry = rMid + 0.03f * sin(waveTime * 0.8f + i);
        ddaLine(rx, ry, rx + 0.07f, ry + 0.003f);
    }
    glPointSize(1.0f);
}

// Boat: wooden hull + mast + sail; bobs up and down in river
void drawBoat(float bx) {
    float f    = br();
    float rMid = (HILL_BASE + RIVER_BOT) * 0.5f;

    // Bob the boat with sin wave
    float by = rMid + 0.006f * sin(waveTime * 2.8f + bx * 5.0f);

    // Clamp boat strictly inside river (no overlap with banks)
    float topLim = HILL_BASE - 0.07f;
    float botLim = RIVER_BOT + 0.05f;
    if (by > topLim) by = topLim;
    if (by < botLim) by = botLim;

    // Hull (filled polygon)
    col(0.46f, 0.22f, 0.06f);
    glBegin(GL_POLYGON);
        glVertex2f(bx - 0.09f, by + 0.010f);
        glVertex2f(bx + 0.09f, by + 0.010f);
        glVertex2f(bx + 0.07f, by - 0.030f);
        glVertex2f(bx - 0.07f, by - 0.030f);
    glEnd();

    // Hull outline (Bresenham lines)
    col(0.26f, 0.12f, 0.03f);
    glPointSize(2.0f);
    bLine(bx-0.09f, by+0.010f, bx+0.09f, by+0.010f);
    bLine(bx+0.09f, by+0.010f, bx+0.07f, by-0.030f);
    bLine(bx+0.07f, by-0.030f, bx-0.07f, by-0.030f);
    bLine(bx-0.07f, by-0.030f, bx-0.09f, by+0.010f);
    glPointSize(1.0f);

    // Mast (DDA line)
    col(0.30f, 0.15f, 0.04f);
    glPointSize(2.5f);
    ddaLine(bx, by + 0.010f, bx, by + 0.125f);
    glPointSize(1.0f);

    // Sail (triangle)
    col(0.94f, 0.90f, 0.80f);
    glBegin(GL_TRIANGLES);
        glVertex2f(bx,          by + 0.125f);
        glVertex2f(bx,          by + 0.016f);
        glVertex2f(bx + 0.08f,  by + 0.072f);
    glEnd();

    // Sail edges (Bresenham lines)
    col(0.58f, 0.54f, 0.46f);
    glPointSize(1.5f);
    bLine(bx, by+0.125f, bx+0.08f, by+0.072f);
    bLine(bx+0.08f, by+0.072f, bx, by+0.016f);
    glPointSize(1.0f);

    // Boatman (small head + body)
    col(0.10f, 0.06f, 0.02f);
    fillCircle(bx - 0.015f, by + 0.024f, 0.011f);
    bLine(bx-0.015f, by+0.013f, bx-0.015f, by-0.003f);
}

// Duck: white body circle + head circle + orange beak
void drawDuck(float x) {
    float f  = br();
    float dy = RIVER_BOT + 0.07f + 0.005f * sin(waveTime * 2.0f + x * 6.0f);
    if (dy > HILL_BASE - 0.06f) dy = HILL_BASE - 0.06f;

    glColor3f(0.92f*f, 0.92f*f, 0.92f*f);
    fillCircle(x, dy, 0.022f);               // body
    fillCircle(x + 0.027f, dy + 0.017f, 0.013f);  // head

    // Beak (orange triangle)
    glColor3f(1.0f*f, 0.58f*f, 0.0f);
    glBegin(GL_TRIANGLES);
        glVertex2f(x + 0.039f, dy + 0.017f);
        glVertex2f(x + 0.052f, dy + 0.015f);
        glVertex2f(x + 0.039f, dy + 0.010f);
    glEnd();
}

// Fish: jumps out of water with arc motion (sin curve)
void drawFish(float ft) {
    float f  = br();
    float ph = fmod(ft, 3.0f);
    if (ph > 1.2f) return;  // only show fish during the jump phase

    float fx = 0.38f + 0.09f * ph;
    float fy = HILL_BASE - 0.04f + 0.04f * sin(ph * 3.14159f / 1.2f);

    // Clamp fish inside river zone
    if (fy > HILL_BASE - 0.02f) fy = HILL_BASE - 0.02f;
    if (fy < RIVER_BOT + 0.02f) fy = RIVER_BOT + 0.02f;

    glColor3f(0.14f*f, 0.62f*f, 0.72f*f);
    fillCircle(fx, fy, 0.020f);  // fish body

    // Tail (triangle)
    glBegin(GL_TRIANGLES);
        glVertex2f(fx - 0.017f, fy);
        glVertex2f(fx - 0.038f, fy + 0.013f);
        glVertex2f(fx - 0.038f, fy - 0.013f);
    glEnd();
}

// Small tree on the river bank edge
void drawBankTree(float x, float sz) {
    float y = RIVER_BOT + 0.01f;  // base sits right on river bank

    col(0.24f, 0.12f, 0.04f);
    glPointSize(5.0f);
    bLine(x, y, x, y + sz * 0.45f);  // trunk
    glPointSize(1.0f);

    col(0.05f, 0.35f, 0.07f);
    fillCircle(x, y + sz * 0.52f, sz * 0.20f);

    col(0.08f, 0.44f, 0.09f);
    fillCircle(x - sz * 0.08f, y + sz * 0.58f, sz * 0.16f);
    fillCircle(x + sz * 0.08f, y + sz * 0.59f, sz * 0.16f);
}


// ============================================================
//  ZONE 3: GREEN FIELD  (y < 0.0)
//
//  LEFT  side (x < -0.08) → Village (huts, well, cows, trees)
//  CENTER (x -0.08 to 0.12) → Dirt road / path
//  RIGHT side (x > 0.12)  → Playground (football, seesaw)
// ============================================================

void drawField() {
    // Main green field
    col(0.17f, 0.50f, 0.10f);
    glBegin(GL_QUADS);
        glVertex2f(-1, -1);
        glVertex2f( 1, -1);
        glVertex2f( 1,  RIVER_BOT);
        glVertex2f(-1,  RIVER_BOT);
    glEnd();

    // Lighter strip at top of field (near river)
    col(0.21f, 0.56f, 0.12f);
    glBegin(GL_QUADS);
        glVertex2f(-1, RIVER_BOT);
        glVertex2f( 1, RIVER_BOT);
        glVertex2f( 1, RIVER_BOT - 0.04f);
        glVertex2f(-1, RIVER_BOT - 0.04f);
    glEnd();

    // Center dirt path (tapers toward bottom = perspective effect)
    col(0.50f, 0.36f, 0.12f);
    glBegin(GL_POLYGON);
        glVertex2f(-0.07f,  RIVER_BOT);
        glVertex2f( 0.07f,  RIVER_BOT);
        glVertex2f( 0.14f, -1.0f);
        glVertex2f(-0.14f, -1.0f);
    glEnd();

    // Road edges (Bresenham lines)
    col(0.34f, 0.22f, 0.07f);
    glPointSize(2.0f);
    bLine(-0.07f, RIVER_BOT, -0.14f, -1.0f);
    bLine( 0.07f, RIVER_BOT,  0.14f, -1.0f);
    glPointSize(1.0f);
}

// Walking man on the road
void drawMan(float x, float y) {
    // Body
    col(0.2f, 0.3f, 0.8f);
    glBegin(GL_QUADS);
        glVertex2f(x-0.01f, y);
        glVertex2f(x+0.01f, y);
        glVertex2f(x+0.01f, y+0.03f);
        glVertex2f(x-0.01f, y+0.03f);
    glEnd();

    // Head
    col(1, 0.8f, 0.6f);
    fillCircle(x, y + 0.045f, 0.012f);

    // Legs (swing using sin for walking animation)
    float step = 0.01f * sin(waveTime * 5);
    ddaLine(x, y, x - 0.01f + step, y - 0.02f);
    ddaLine(x, y, x + 0.01f - step, y - 0.02f);
}

// Hut: wall + door + window + thatched roof
void drawHut(float x, float y, float w, float h) {
    // Wall (filled rectangle)
    col(0.68f, 0.50f, 0.28f);
    glBegin(GL_QUADS);
        glVertex2f(x,   y);
        glVertex2f(x+w, y);
        glVertex2f(x+w, y+h);
        glVertex2f(x,   y+h);
    glEnd();

    // Wall outline (Bresenham)
    col(0.36f, 0.22f, 0.08f);
    glPointSize(2.0f);
    bLine(x, y, x+w, y);
    bLine(x+w, y, x+w, y+h);
    bLine(x+w, y+h, x, y+h);
    bLine(x, y+h, x, y);
    glPointSize(1.0f);

    // Door
    col(0.22f, 0.11f, 0.04f);
    float dw = w * 0.22f, dh = h * 0.44f, dx = x + w * 0.38f;
    glBegin(GL_QUADS);
        glVertex2f(dx,    y);
        glVertex2f(dx+dw, y);
        glVertex2f(dx+dw, y+dh);
        glVertex2f(dx,    y+dh);
    glEnd();

    // Window (small blue square)
    col(0.42f, 0.68f, 0.84f);
    float wx = x + w*0.65f, wy2 = y + h*0.52f, ws = w*0.17f;
    glBegin(GL_QUADS);
        glVertex2f(wx,    wy2);
        glVertex2f(wx+ws, wy2);
        glVertex2f(wx+ws, wy2+ws);
        glVertex2f(wx,    wy2+ws);
    glEnd();

    // Window cross (X pattern)
    col(0.24f, 0.13f, 0.05f);
    glPointSize(1.5f);
    bLine(wx, wy2, wx+ws, wy2+ws);
    bLine(wx+ws, wy2, wx, wy2+ws);
    glPointSize(1.0f);

    // Thatched roof (triangle)
    float apex = y + h + h*0.58f, cx = x + w*0.5f;
    col(0.74f, 0.67f, 0.29f);
    glBegin(GL_TRIANGLES);
        glVertex2f(x - w*0.07f,   y+h);
        glVertex2f(x + w + w*0.07f, y+h);
        glVertex2f(cx, apex);
    glEnd();

    // Thatch lines (DDA lines fanning from base to apex)
    col(0.51f, 0.45f, 0.17f);
    glPointSize(1.5f);
    for (int i = 0; i <= 9; i++)
        ddaLine(x - w*0.07f + i*(w*1.14f/9), y+h, cx, apex);
    glPointSize(1.0f);

    // Roof edges
    col(0.37f, 0.30f, 0.11f);
    glPointSize(2.0f);
    bLine(x - w*0.07f, y+h, cx, apex);
    bLine(x + w + w*0.07f, y+h, cx, apex);
    glPointSize(1.0f);
}

// Well: cylindrical base + wooden frame + rope + bucket
void drawWell(float x, float y) {
    // Stone base (rectangle)
    col(0.48f, 0.24f, 0.09f);
    glBegin(GL_QUADS);
        glVertex2f(x-0.042f, y-0.076f);
        glVertex2f(x+0.042f, y-0.076f);
        glVertex2f(x+0.042f, y);
        glVertex2f(x-0.042f, y);
    glEnd();

    // Rim circle + frame
    col(0.28f, 0.13f, 0.04f);
    glPointSize(3.0f);
    mpCircle(x, y, 0.042f);
    bLine(x-0.033f, y, x-0.033f, y+0.062f);   // left post
    bLine(x+0.033f, y, x+0.033f, y+0.062f);   // right post
    bLine(x-0.042f, y+0.062f, x+0.042f, y+0.062f);  // top bar

    // Rope + bucket
    col(0.48f, 0.32f, 0.12f);
    ddaLine(x, y+0.062f, x, y+0.015f);  // rope
    glPointSize(1.0f);
    col(0.26f, 0.26f, 0.70f);
    fillCircle(x, y+0.011f, 0.008f);    // bucket (small blue circle)
}

// Fence: horizontal rails + vertical posts

// Village tree: trunk + 3 circle canopy layers
void drawVillageTree(float x, float y, float sz) {
    col(0.24f, 0.12f, 0.04f);
    glPointSize(6.0f);
    bLine(x, y, x, y + sz*0.50f);  // trunk
    glPointSize(1.0f);

    col(0.05f, 0.37f, 0.07f);
    fillCircle(x, y + sz*0.55f, sz*0.22f);

    col(0.08f, 0.46f, 0.10f);
    fillCircle(x - sz*0.09f, y + sz*0.61f, sz*0.17f);
    fillCircle(x + sz*0.09f, y + sz*0.62f, sz*0.17f);

    col(0.12f, 0.55f, 0.12f);
    fillCircle(x, y + sz*0.69f, sz*0.12f);  // bright top
}

// Cow: white body + spots + head + 4 animated legs
void drawCow(float x, float y) {
    float f = br();

    if (isDay)
        col(0.88f, 0.88f, 0.88f);
    else
        col(0.03f,  0.03f,  0.3f);

    glBegin(GL_QUADS);
        glVertex2f(x-0.062f, y);
        glVertex2f(x+0.062f, y);
        glVertex2f(x+0.062f, y+0.046f);
        glVertex2f(x-0.062f, y+0.046f);
    glEnd();

    // Black spots
    glColor3f(0.08f*f, 0.08f*f, 0.08f*f);
    fillCircle(x-0.016f, y+0.028f, 0.013f);
    fillCircle(x+0.034f, y+0.010f, 0.010f);

    // Head (lighter circle)
    glColor3f(0.82f*f, 0.82f*f, 0.82f*f);
    fillCircle(x+0.080f, y+0.036f, 0.020f);
    glColor3f(0, 0, 0);
    fillCircle(x+0.088f, y+0.042f, 0.004f);  // eye

    // Animated legs (slight swing using sin)
    col(0.64f, 0.64f, 0.64f);
    glPointSize(3.0f);
    float loff = 0.004f * sin(waveTime * 3.0f);
    bLine(x-0.046f, y, x-0.046f+loff,  y-0.042f);
    bLine(x-0.014f, y, x-0.014f-loff,  y-0.042f);
    bLine(x+0.018f, y, x+0.018f+loff,  y-0.042f);
    bLine(x+0.050f, y, x+0.050f-loff,  y-0.042f);
    glPointSize(1.0f);

    // Tail
    col(0.62f, 0.62f, 0.62f);
    ddaLine(x-0.062f, y+0.025f, x-0.090f, y+0.052f);
}

// Flower: stem + 2 leaves + 8 petals + yellow center
void drawFlower(float x, float y, float r, float g, float b) {
    float f = br();

    // Curved stem (dots in a slight arc)
    col(0.10f, 0.45f, 0.05f);
    glPointSize(2.0f);
    for (int i = 0; i < 10; i++) {
        float t  = i / 10.0f;
        float cx = x + 0.01f * sin(t * 2.0f);
        float cy = y - 0.05f * t;
        glBegin(GL_POINTS);
        glVertex2f(cx, cy);
        glEnd();
    }
    glPointSize(1.0f);

    // Leaves
    col(0.12f, 0.55f, 0.08f);
    fillCircle(x - 0.015f, y - 0.03f, 0.01f);
    fillCircle(x + 0.015f, y - 0.02f, 0.01f);

    // 8 petals arranged in a ring
    for (int i = 0; i < 8; i++) {
        float angle = (2 * 3.1416f * i) / 8;
        float px = x + cos(angle) * 0.02f;
        float py = y + sin(angle) * 0.02f;
        float shade = 0.8f + 0.2f * sin(i);
        glColor3f(r*shade*f, g*shade*f, b*shade*f);
        fillCircle(px, py, 0.012f);
    }

    // Yellow center
    col(1.0f, 0.85f, 0.1f);
    fillCircle(x, y, 0.010f);
    col(0.8f, 0.5f, 0.0f);
    mpCircle(x, y, 0.010f);  // darker ring outline
}

// Butterfly: 4 wing circles + body + antennae; wings flap with sin
void drawButterfly(float x, float y, float t) {
    float flap = 0.02f * sin(t);  // wing flap up/down animation

    // Left wings
    col(1.0f, 0.3f, 0.6f);
    fillCircle(x - 0.02f, y + flap, 0.02f);
    col(1.0f, 0.5f, 0.8f);
    fillCircle(x - 0.03f, y - 0.01f + flap, 0.018f);

    // Right wings
    col(1.0f, 0.3f, 0.6f);
    fillCircle(x + 0.02f, y + flap, 0.02f);
    col(1.0f, 0.5f, 0.8f);
    fillCircle(x + 0.03f, y - 0.01f + flap, 0.018f);

    // Body
    col(0.1f, 0.1f, 0.1f);
    fillCircle(x, y, 0.008f);

    // Antennae (DDA lines)
    glPointSize(1.5f);
    ddaLine(x, y+0.01f, x-0.01f, y+0.02f);
    ddaLine(x, y+0.01f, x+0.01f, y+0.02f);
    glPointSize(1.0f);
}

// Football field: green rectangle + boundary lines + goals + center circle
void drawFootballField() {
    float left   = 0.20f;
    float right  = 0.95f;
    float top    = RIVER_BOT - 0.02f;
    float bottom = -0.85f;

    // Field grass (darker green)
    col(0.15f, 0.55f, 0.15f);
    glBegin(GL_QUADS);
        glVertex2f(left,  bottom);
        glVertex2f(right, bottom);
        glVertex2f(right, top);
        glVertex2f(left,  top);
    glEnd();

    // Field markings (Bresenham lines + mid-point circle)
    col(0.12f, 0.45f, 0.12f);
    glPointSize(2);
    bLine(left,  top,    right, top);       // top boundary
    bLine(left,  bottom, right, bottom);    // bottom boundary
    bLine(left,  top,    left,  bottom);    // left boundary
    bLine(right, top,    right, bottom);    // right boundary

    float mid = (left + right) / 2.0f;
    bLine(mid, top, mid, bottom);           // center line
    mpCircle(mid, (top+bottom)/2.0f, 0.10f); // center circle

    // Left goal box
    bLine(left,        -0.30f, left+0.08f, -0.30f);
    bLine(left+0.08f,  -0.30f, left+0.08f, -0.45f);
    bLine(left+0.08f,  -0.45f, left,       -0.45f);

    // Right goal box
    bLine(right,       -0.30f, right-0.08f, -0.30f);
    bLine(right-0.08f, -0.30f, right-0.08f, -0.45f);
    bLine(right-0.08f, -0.45f, right,       -0.45f);

    glPointSize(1);
}

// Child running: body + head + animated legs + arms
void drawChild(float x, float y, float ct, float r, float g, float b2) {
    float f    = br();
    float leg  = 0.015f * sin(ct);
    float arm  = 0.015f * sin(ct + 1.5f);
    float tilt = 0.005f * sin(ct);   // body sways slightly

    // Body (slightly tilted rectangle)
    if (isDay)
        col(r, g, b2);
    else
        col(r*0.5f, g*0.5f, b2*0.5f);  // dimmer at night

    glBegin(GL_QUADS);
        glVertex2f(x-0.015f+tilt, y+0.009f);
        glVertex2f(x+0.015f+tilt, y+0.009f);
        glVertex2f(x+0.015f-tilt, y+0.036f);
        glVertex2f(x-0.015f-tilt, y+0.036f);
    glEnd();

    // Head
    glColor3f(0.7f*f, 0.5f*f, 0.3f*f);
    fillCircle(x, y + 0.048f, 0.013f);

    // Legs (Bresenham with swing)
    col(0.26f, 0.16f, 0.07f);
    glPointSize(2.5f);
    bLine(x, y+0.009f, x-0.013f+leg, y-0.020f);
    bLine(x, y+0.009f, x+0.013f-leg, y-0.020f);
    bLine(x-0.013f+leg, y-0.020f, x-0.021f+leg, y-0.027f);  // feet
    bLine(x+0.013f-leg, y-0.020f, x+0.021f-leg, y-0.027f);

    // Arms (DDA with swing)
    col(0.85f, 0.64f, 0.38f);
    glPointSize(2.0f);
    ddaLine(x-0.015f, y+0.028f, x-0.030f+arm, y+0.014f);
    ddaLine(x+0.015f, y+0.028f, x+0.030f-arm, y+0.014f);
    glPointSize(1.0f);
}

// Football: white circle with black outline
void drawBall() {
    float f = br();

    if (f < 0.65f) return;

    col(1, 1, 1);
    fillCircle(ballX, ballY, 0.015f);

    col(0, 0, 0);
    mpCircle(ballX, ballY, 0.015f);
}
// Seesaw: vertical post + tilting plank + two children
void drawSeesaw(float x, float y) {
    float tilt = 0.042f * sin(waveTime * 1.2f);  // plank tilts up/down

    // Central post
    col(0.62f, 0.30f, 0.08f);
    glPointSize(3.0f);
    bLine(x, y, x, y + 0.052f);
    glPointSize(1.0f);

    // Tilting plank
    col(0.60f, 0.36f, 0.12f);
    glPointSize(2.0f);
    bLine(x - 0.10f, y+0.052f - tilt,
          x + 0.10f, y+0.052f + tilt);
    glPointSize(1.0f);

    // Two children sit at each end of the seesaw
    drawChild(x-0.09f, y+0.052f - tilt,  waveTime*3.0f, 0.80f, 0.18f, 0.18f);
    drawChild(x+0.09f, y+0.052f + tilt, -waveTime*3.0f, 0.18f, 0.38f, 0.90f);
}


// ============================================================
//  DISPLAY  – main rendering function called every frame
// ============================================================
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Apply 3D and 2D transforms (from keyboard input)
    glRotatef(rotY, 0, 1, 0);    // 3D rotate around Y axis
    glRotatef(rotX, 1, 0, 0);    // 3D rotate (tilt) around X axis
    glTranslatef(transX, transY, 0);  // 2D translation
    glScalef(scaleS, scaleS, 1.0f);   // 2D scale

    // ── ZONE 1: SKY + HILLS ──────────────────────────────────
    drawSky();
    drawStars();
    drawSun();
    drawMoon();
    drawHills();
    drawCloud(cloudX,  0.80f);
    drawCloud(cloud2X, 0.88f);
    drawBirds(birdX, 0.68f);

    // ── ZONE 2: RIVER ────────────────────────────────────────
    drawRiver();
    drawBoat(boatX);
    drawDuck(duckX1);
    drawDuck(duckX2);
    drawFish(fishT);
    drawBankTree(-0.84f, 0.24f);
    drawBankTree(-0.60f, 0.20f);
    drawBankTree( 0.70f, 0.22f);
    drawBankTree( 0.88f, 0.19f);

    // ── ZONE 3: FIELD (GREEN GROUND) ────────────────────────
    drawField();
    drawMan(manX, manY);  // man walking up the road

    // Village side (left half)
    float HY = RIVER_BOT - 0.20f;  // hut base Y (well below river)

    drawVillageTree(-0.94f, RIVER_BOT - 0.02f, 0.36f);
    drawVillageTree(-0.60f, RIVER_BOT - 0.02f, 0.32f);

    drawHut(-0.98f, HY, 0.26f, 0.21f);
    drawHut(-0.75f, HY, 0.22f, 0.18f);
    drawHut(-0.55f, HY, 0.18f, 0.15f);

    drawWell(-0.18f, RIVER_BOT - 0.14f);

    drawCow(cowX1, cowY1);
    drawCow(cowX2, cowY2);

    // Flowers on left side
    float flx[] = {-0.92f, -0.80f, -0.68f, -0.56f, -0.44f};
    float flr[][3] = {
        {1,1,1}, {1,0.2f,0.2f}, {1,0.8f,0},
        {0.4f,0.5f,1}, {0.8f,0.1f,0.8f}
    };
    for (int i = 0; i < 5; i++)
        drawFlower(flx[i], -0.92f, flr[i][0], flr[i][1], flr[i][2]);

    // Playground side (right half)
    drawFootballField();

    float baseY = -0.60f;
    // 6 players: 2 red, 2 blue, 2 yellow
    drawChild(playerX[0], baseY,       childT, 1,0,0);
    drawChild(playerX[1], baseY+0.05f, childT, 1,0,0);
    drawChild(playerX[2], baseY,       childT, 0,0,1);
    drawChild(playerX[3], baseY+0.05f, childT, 0,0,1);
    drawChild(playerX[4], baseY,       childT, 1,1,0);
    drawChild(playerX[5], baseY+0.05f, childT, 1,1,0);

    drawVillageTree(1.00f, RIVER_BOT - 0.02f, 0.30f);

    float cy = RIVER_BOT - 0.22f;
    drawChild(0.50f, cy, childT*3.5f,   0.9f,0.2f,0.2f);
    drawChild(0.62f, cy, childT*3.5f+1, 0.2f,0.5f,0.9f);

    drawBall();

    drawSeesaw(-0.55f, RIVER_BOT - 0.45f);

    // Flowers on right side
    float frx[] = {0.68f, 0.80f, 0.94f};
    float frr[][3] = {{1,1,0}, {1,.4f,.7f}, {.3f,1,.5f}};
    for (int i = 0; i < 3; i++)
        drawFlower(frx[i], -0.92f, frr[i][0], frr[i][1], frr[i][2]);

    // Two butterflies flying across playground
    drawButterfly(butterflyX,
                  -0.90f + 0.03f * sin(butterflyT),
                  butterflyT);
    drawButterfly(butterflyX + 0.15f,
                  -0.85f + 0.04f * cos(butterflyT + 2),
                  butterflyT + 2);

    glutSwapBuffers();  // swap front/back buffer (double buffering)
}


// ============================================================
//  TIMER – called every 16ms (~60 FPS), updates all animations
// ============================================================
void timer(int) {
    // Sun rises during day
    if (isDay) {
        if (sunY < 0.70f) sunY += 0.0007f;
    }

    // Clouds drift right, wrap around
    cloudX  += 0.00035f; if (cloudX  > 1.6f) cloudX  = -1.6f;
    cloud2X += 0.00023f; if (cloud2X > 1.6f) cloud2X = -1.6f;

    // Birds fly right, wrap around
    birdX += 0.00165f; if (birdX > 1.6f) birdX = -1.6f;

    // Man walks up the road, resets at bottom when reaching river
    manY += 0.0006f;
    if (manY > -0.05f) manY = -0.90f;
    if (manX > 0.07f)  manX = -0.07f;

    // Wave/time counter (drives most animations)
    waveTime += 0.038f;

    // Boat moves right across river
    boatX += 0.00055f; if (boatX > 0.90f) boatX = -0.90f;

    // Fish jump cycle
    fishT += 0.013f;

    // Ducks move slowly right
    duckX1 += 0.00016f; if (duckX1 > -0.12f) duckX1 = -0.88f;
    duckX2 += 0.00012f; if (duckX2 >  0.08f) duckX2 = -0.68f;

    // Butterfly floats across playground
    butterflyT += 0.035f;
    butterflyX += 0.0005f;
    if (butterflyX > 0.95f) butterflyX = 0.20f;

    // Cows graze slowly, stop before road
    cowX1 += 0.00020f;
    cowX2 += 0.00025f;
    cowY1 = -0.70f + 0.01f * sin(waveTime);       // head bobs (eating)
    cowY2 = -0.78f + 0.01f * sin(waveTime + 1);
    if (cowX1 > -0.12f) cowX1 = -0.12f;
    if (cowX2 > -0.12f) cowX2 = -0.12f;

    // Football players bounce left/right within field boundaries
    for (int i = 0; i < 6; i++) {
        playerX[i] += 0.0008f * playerDir[i];
        if (playerX[i] > 0.90f || playerX[i] < 0.22f)
            playerDir[i] *= -1;  // reverse direction at edges
    }

    // Ball moves toward a random target player
    float targetX = playerX[targetPlayer];
    float targetY = -0.55f;
    ballX += (targetX - ballX) * 0.02f;
    ballY += (targetY - ballY) * 0.02f;
    if (fabs(ballX - targetX) < 0.01f)
        targetPlayer = rand() % 6;  // pick new target when ball arrives

    // Children running animation
    childT += 0.023f;

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);  // re-register timer (16ms = ~60 FPS)
}


// ============================================================
//  KEYBOARD INPUT
// ============================================================
void keyboard(unsigned char key, int, int) {
    switch (key) {
        // 2D Scale (zoom)
        case '+': scaleS += 0.05f; break;
        case '-': scaleS -= 0.05f; if (scaleS < 0.1f) scaleS = 0.1f; break;

        // 3D Rotation
        case 'R': rotY += 3.0f; break;   // rotate right around Y
        case 'r': rotY -= 3.0f; break;   // rotate left around Y
        case 'T': rotX += 3.0f; break;   // tilt forward around X
        case 't': rotX -= 3.0f; break;   // tilt backward around X

        // Day/Night toggle
        case 'd': isDay = true;  sunY = 0.70f;  break;  // force daytime
        case 'n': isDay = false; sunY = -0.10f; break;  // force night

        // Reset all transforms
        case '0': transX=0; transY=0; scaleS=1; rotY=0; rotX=0; break;

        case 27: exit(0);  // ESC to quit
    }
    glutPostRedisplay();
}

// Arrow keys for 2D translation
void specialKeys(int key, int, int) {
    switch (key) {
        case GLUT_KEY_UP:    transY += 0.05f; break;
        case GLUT_KEY_DOWN:  transY -= 0.05f; break;
        case GLUT_KEY_LEFT:  transX -= 0.05f; break;
        case GLUT_KEY_RIGHT: transX += 0.05f; break;
    }
    glutPostRedisplay();
}


// ============================================================
//  INIT – OpenGL setup
// ============================================================
void init() {
    glClearColor(0, 0, 0, 1);              // black background
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);             // 2D coordinate system: -1 to +1
    glMatrixMode(GL_MODELVIEW);
    glPointSize(1.5f);
}


// ============================================================
//  MAIN ENTRY POINT
// ============================================================
int main(int argc, char** argv) {
    cout << "========================================\n";
    cout << "  Interactive Village Environment Simulation\n";
    cout << "========================================\n";
    cout << "  Arrow Keys -> 2D Translate (Pan)\n";
    cout << "  +  /  -    -> Zoom In/Out (Scale)\n";
    cout << "  R  /  r    -> 3D Rotate Y-axis\n";
    cout << "  T  /  t    -> 3D Tilt X-axis\n";
    cout << "  d          -> Switch to Day\n";
    cout << "  n          -> Switch to Night\n";
    cout << "  0          -> Reset All Transforms\n";
    cout << "  ESC        -> Exit\n";
    cout << "========================================\n";

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1100, 720);
    glutInitWindowPosition(50, 20);
    glutCreateWindow("Interactive Village Environment Simulation");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    return 0;
}
