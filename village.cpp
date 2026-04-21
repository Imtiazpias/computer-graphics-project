// village.cpp - Village objects (huts, well, fence, trees)

#include <GL/glut.h>

void drawHut(float x,float y,float w,float h){
    glColor3f(0.70f,0.52f,0.30f);

    glBegin(GL_QUADS);
        glVertex2f(x,y);
        glVertex2f(x+w,y);
        glVertex2f(x+w,y+h);
        glVertex2f(x,y+h);
    glEnd();

    // roof
    glColor3f(0.75f,0.65f,0.25f);
    glBegin(GL_TRIANGLES);
        glVertex2f(x-0.02f,y+h);
        glVertex2f(x+w+0.02f,y+h);
        glVertex2f(x+w/2,y+h+0.10f);
    glEnd();
}

void drawWell(float x,float y){
    glColor3f(0.4f,0.2f,0.1f);
    glBegin(GL_QUADS);
        glVertex2f(x-0.04f,y);
        glVertex2f(x+0.04f,y);
        glVertex2f(x+0.04f,y+0.06f);
        glVertex2f(x-0.04f,y+0.06f);
    glEnd();
}

void drawFence(float x,float y,float len){
    glColor3f(0.5f,0.3f,0.1f);

    for(float i=0;i<len;i+=0.04f){
        glBegin(GL_LINES);
            glVertex2f(x+i,y);
            glVertex2f(x+i,y+0.05f);
        glEnd();
    }
}