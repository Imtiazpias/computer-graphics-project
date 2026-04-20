#include <GL/glut.h>
#include <cmath>

// External variables (from main)
extern float sunY;
extern float waveTime;

// Brightness function
float br(){
    float b = (sunY + 0.10f) / 0.85f;
    if(b < 0.07f) b = 0.07f;
    if(b > 1.0f) b = 1.0f;
    return b;
}

void col(float r, float g, float b){
    float f = br();
    glColor3f(r * f, g * f, b * f);
}

// You must already have these in another file
extern void fillCircle(float cx, float cy, float r);
extern void mpCircle(float cx, float cy, float r);
extern void ddaLine(float x1, float y1, float x2, float y2);

// Sky background
void drawSky(){
    float f = br();
    glBegin(GL_QUADS);
        glColor3f(0.01f+0.20f*f,0.01f+0.42f*f,0.06f+0.66f*f);
        glVertex2f(-1,1); glVertex2f(1,1);
        glColor3f(0.04f+0.48f*f,0.04f+0.58f*f,0.09f+0.68f*f);
        glVertex2f(1,0.35f); glVertex2f(-1,0.35f);
    glEnd();
}

// Stars (night only)
void drawStars(){
    float f = br();
    if(f > 0.65f) return;

    float sf = 1.0f - f/0.65f;
    glColor3f(sf, sf, sf);
    glPointSize(2.0f);

    float sx[] = {-0.9f,-0.7f,-0.5f,-0.3f,-0.1f,0.1f,0.3f,0.5f,0.7f,0.9f,
                  -0.8f,-0.6f,-0.4f,-0.2f,0.0f,0.2f,0.4f,0.6f,0.8f};

    float sy[] = {0.85f,0.78f,0.92f,0.75f,0.88f,0.80f,0.90f,0.76f,0.84f,0.79f,
                  0.70f,0.82f,0.74f,0.86f,0.77f,0.89f,0.73f,0.83f,0.71f};

    glBegin(GL_POINTS);
    for(int i=0;i<19;i++)
        glVertex2f(sx[i], sy[i]);
    glEnd();

    glPointSize(1.0f);
}

// Moon
void drawMoon(){
    float f = br();
    if(f > 0.65f) return;

    float mx = 0.75f;
    float my = 0.80f;

    glColor3f(1.0f, 1.0f, 1.0f);
    fillCircle(mx, my, 0.055f);

    float skyR = 0.01f + 0.20f*f;
    float skyG = 0.01f + 0.42f*f;
    float skyB = 0.06f + 0.66f*f;

    glColor3f(skyR, skyG, skyB);
    fillCircle(mx + 0.030f, my, 0.050f);
}

// Sun
void drawSun(){
    float f = br();
    float sunX = -0.25f;
    float sunVisibleY = sunY;

    for(float ri = 0.12f; ri > 0.05f; ri -= 0.01f){
        float t = (ri - 0.05f) / (0.12f - 0.05f);

        float r = 1.0f;
        float g = 0.3f + 0.6f * (1.0f - t);
        float b = 0.0f;

        glColor3f(r*f, g*f, b*f);
        mpCircle(sunX, sunVisibleY, ri);
    }

    glColor3f(1.0f*f, 0.9f*f, 0.1f*f);
    fillCircle(sunX, sunVisibleY, 0.07f);

    glColor3f(1.0f*f, 0.85f*f, 0.2f*f);
    glPointSize(1.5f);

    for(int i = 0; i < 10; i++){
        float a = i * 36 * 3.14159f / 180.0f;
        float len = 0.08f + 0.01f * sin(waveTime + i);

        ddaLine(sunX, sunVisibleY,
                sunX + len * cos(a),
                sunVisibleY + len * sin(a));
    }

    glPointSize(1.0f);
}

// Cloud
void drawCloud(float x, float y){
    float f = br();
    float w = 0.90f*f + 0.10f;

    glColor3f(w, w, w);
    fillCircle(x, y, 0.065f);
    fillCircle(x+0.078f,y+0.024f,0.050f);
    fillCircle(x-0.072f,y+0.016f,0.048f);
    fillCircle(x+0.140f,y-0.004f,0.038f);
    fillCircle(x-0.135f,y-0.006f,0.033f);
}