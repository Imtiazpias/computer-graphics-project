
#include <GL/glut.h>
#include <cmath>

// External variables from main.cpp
extern float waveTime;
extern float HILL_BASE;
extern float RIVER_BOT;

// brightness helper (same logic as main)
float riverBrightness(){
    float b = 1.0f;
    return b;
}

// safe color helper
void riverColor(float r, float g, float b){
    float f = riverBrightness();
    glColor3f(r * f, g * f, b * f);
}

// ============================================================
//  MAIN RIVER BODY
// ============================================================
void drawRiver(){
    float f = riverBrightness();

    float top = HILL_BASE - 0.02f;
    float bottom = RIVER_BOT;

    // =========================
    // WATER BODY
    // =========================
    glColor3f(0.08f * f, 0.28f * f, 0.55f * f);
    glBegin(GL_QUADS);
        glVertex2f(-1, bottom);
        glVertex2f(1, bottom);
        glVertex2f(1, top);
        glVertex2f(-1, top);
    glEnd();

    // =========================
    // TOP SHORE WAVE LINE
    // =========================
    riverColor(0.16f, 0.48f, 0.10f);

    glBegin(GL_POLYGON);
        glVertex2f(-1, HILL_BASE);

        for(int i = 0; i <= 150; i++){
            float x = -1.0f + i * (2.0f / 150.0f);

            float y = (HILL_BASE - 0.02f)
                    + 0.020f * sin(x * 8.0f + waveTime * 0.7f)
                    + 0.008f * sin(x * 16.0f - waveTime);

            glVertex2f(x, y);
        }

        glVertex2f(1, HILL_BASE);
    glEnd();

    // =========================
    // BOTTOM SHORE LINE
    // =========================
    riverColor(0.14f, 0.45f, 0.08f);

    glBegin(GL_POLYGON);
        glVertex2f(-1, bottom);

        for(int i = 0; i <= 150; i++){
            float x = -1.0f + i * (2.0f / 150.0f);

            float y = bottom
                    + 0.010f * sin(x * 7.0f + waveTime * 0.6f)
                    - 0.006f * sin(x * 15.0f - waveTime * 0.8f);

            glVertex2f(x, y);
        }

        glVertex2f(1, bottom);
    glEnd();

    // =========================
    // WATER WAVES (LINES)
    // =========================
    for(int i = 0; i < 10; i++){
        float y = top - i * 0.03f;

        float phase = waveTime * 1.3f + i * 0.5f;

        glColor3f(0.2f * f, 0.55f * f, 0.9f * f);

        glBegin(GL_LINE_STRIP);
            for(int j = 0; j <= 120; j++){
                float x = -1.0f + j * (2.0f / 120.0f);

                float wy = y
                         + 0.004f * sin(x * 20.0f + phase)
                         + 0.002f * sin(x * 35.0f - phase);

                glVertex2f(x, wy);
            }
        glEnd();
    }

    // =========================
    // SHIMMER EFFECT (LIGHT STRIPS)
    // =========================
    glColor3f(0.6f * f, 0.8f * f, 1.0f * f);
    glPointSize(1.5f);

    for(int i = 0; i < 6; i++){
        float x = -0.8f + i * 0.3f + 0.05f * sin(waveTime + i);

        float y = (HILL_BASE + RIVER_BOT) * 0.5f
                + 0.02f * sin(waveTime * 0.8f + i);

        glBegin(GL_LINES);
            glVertex2f(x, y);
            glVertex2f(x + 0.06f, y + 0.003f);
        glEnd();
    }

    glPointSize(1.0f);
}