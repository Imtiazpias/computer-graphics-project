// nature.cpp - Clouds, birds, butterfly, flowers
// ============================================================

#include <GL/glut.h>
#include <cmath>

extern float waveTime;

void drawCloud(float x,float y){
    glColor3f(1,1,1);

    for(int i=0;i<4;i++){
        float ox = x + i*0.04f;
        float oy = y + 0.01f*sin(waveTime);

        glBegin(GL_POLYGON);
            for(int j=0;j<100;j++){
                float a = 2*3.1416*j/100;
                glVertex2f(ox + 0.03f*cos(a),
                           oy + 0.02f*sin(a));
            }
        glEnd();
    }
}

void drawBird(float x,float y){
    glColor3f(0,0,0);

    glBegin(GL_LINES);
        glVertex2f(x-0.02f,y);
        glVertex2f(x,y+0.01f);

        glVertex2f(x,y+0.01f);
        glVertex2f(x+0.02f,y);
    glEnd();
}

void drawButterfly(float x,float y){
    glColor3f(1,0.3f,0.6f);

    glBegin(GL_POLYGON);
        for(int i=0;i<100;i++){
            float a = 2*3.1416*i/100;
            glVertex2f(x+0.01f*cos(a), y+0.01f*sin(a));
        }
    glEnd();
}
