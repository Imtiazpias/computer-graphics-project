#include <GL/glut.h>
#include <cmath>
#include "dda.h"

void ddaLine(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;

    int steps = (int)(fabs(dx) > fabs(dy) ? fabs(dx)*400 : fabs(dy)*400);
    if(steps == 0) return;

    float xInc = dx / steps;
    float yInc = dy / steps;

    float x = x1;
    float y = y1;

    glBegin(GL_POINTS);
    for(int i = 0; i <= steps; i++){
        glVertex2f(x, y);
        x += xInc;
        y += yInc;
    }
    glEnd();
}