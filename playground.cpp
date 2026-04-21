// playground.cpp - Football field + kids + ball + seesaw

#include <GL/glut.h>
#include <cmath>

extern float ballX, ballY;
extern float waveTime;

void drawBall(){
    glColor3f(1,1,1);

    glBegin(GL_POLYGON);
        for(int i=0;i<100;i++){
            float a = 2*3.1416*i/100;
            glVertex2f(ballX + 0.015f*cos(a),
                       ballY + 0.015f*sin(a));
        }
    glEnd();
}

void drawSimplePlayer(float x,float y){
    glColor3f(0.9f,0.2f,0.2f);

    glBegin(GL_QUADS);
        glVertex2f(x-0.015f,y);
        glVertex2f(x+0.015f,y);
        glVertex2f(x+0.015f,y+0.04f);
        glVertex2f(x-0.015f,y+0.04f);
    glEnd();
}

void drawSeesaw(float x,float y){
    float tilt = 0.03f * sin(waveTime);

    glColor3f(0.6f,0.3f,0.1f);

    glBegin(GL_LINES);
        glVertex2f(x-0.1f,y-tilt);
        glVertex2f(x+0.1f,y+tilt);
    glEnd();
}