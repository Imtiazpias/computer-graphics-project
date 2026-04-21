// ============================================================
//  Bangladeshi Village Scene - CG Project
// ============================================================
#include <GL/glut.h>
#include <ctime>
#include <iostream>
#include <cmath>
using namespace std;

const float HILL_BASE  =  0.35f;  // bottom of sky/hill zone
const float RIVER_BOT  =  0.00f;  // bottom of river = top of green field
// So green field is from y=-1.0 to y=0.0
// River is from y=0.0 to y=0.35
// Hills are from y=0.35 to y=1.0

float sunY      = -0.10f;   // sun starts below hills
bool isDay = true;
float waveTime  =  0.00f;
float boatX     = -0.88f;   // boat in river
float birdX     = -1.20f;
float cloudX    =  0.15f;
float cloud2X   = -0.55f;
float manX = 0.0f;     // keep centered on road
float manY = -0.90f;   // start from bottom
float cowX1 = -0.90f;
float cowX2 = -0.75f;
float cowY1 = -0.70f;
float cowY2 = -0.78f;
float butterflyX = 0.20f;
float butterflyT=  0.00f;
float childT    =  0.00f;
float playerX[6] = {0.25f,0.35f,0.45f,0.55f,0.70f,0.85f};
float playerDir[6] = {1,-1,1,-1,1,-1};
float ballX = 0.50f;
float ballY = -0.55f;

int targetPlayer = 0;
float fishT     =  0.00f;
float duckX1    = -0.78f;
float duckX2    = -0.50f;

// 2D/3D transform
float transX=0,transY=0,scaleS=1.0f,rotY=0,rotX=0;

// ══════════════════════════════════════
//  BRIGHTNESS (0=night → 1=day)
// ══════════════════════════════════════
float br(){
    float b=(sunY+0.10f)/0.85f;
    if(b<0.07f)b=0.07f; if(b>1.0f)b=1.0f; return b;
}
void col(float r,float g,float b){ float f=br(); glColor3f(r*f,g*f,b*f); }

// ══════════════════════════════════════
//  ALGORITHM 1 – DDA LINE
// ══════════════════════════════════════
void ddaLine(float x1,float y1,float x2,float y2){
    float dx=x2-x1,dy=y2-y1;
    int s=(int)((fabs(dx)>fabs(dy)?fabs(dx):fabs(dy))*900);
    if(!s)return;
    float xi=dx/s,yi=dy/s,x=x1,y=y1;
    glBegin(GL_POINTS);
    for(int i=0;i<=s;i++){glVertex2f(x,y);x+=xi;y+=yi;}
    glEnd();
}

// ══════════════════════════════════════
//  ALGORITHM 2 – BRESENHAM LINE
// ══════════════════════════════════════
void bLine(float fx1,float fy1,float fx2,float fy2){
    int x1=(int)(fx1*900),y1=(int)(fy1*900);
    int x2=(int)(fx2*900),y2=(int)(fy2*900);
    int dx=abs(x2-x1),dy=abs(y2-y1);
    int sx=x1<x2?1:-1,sy=y1<y2?1:-1,e=dx-dy;
    glBegin(GL_POINTS);
    for(;;){
        glVertex2f(x1/900.0f,y1/900.0f);
        if(x1==x2&&y1==y2)break;
        int e2=2*e;
        if(e2>-dy){e-=dy;x1+=sx;}
        if(e2< dx){e+=dx;y1+=sy;}
    }
    glEnd();
}

// ══════════════════════════════════════
//  ALGORITHM 3 – MID-POINT CIRCLE
// ══════════════════════════════════════
void plotPts(float cx,float cy,int px,int py,float sc){
    glVertex2f(cx+px*sc,cy+py*sc); glVertex2f(cx-px*sc,cy+py*sc);
    glVertex2f(cx+px*sc,cy-py*sc); glVertex2f(cx-px*sc,cy-py*sc);
    glVertex2f(cx+py*sc,cy+px*sc); glVertex2f(cx-py*sc,cy+px*sc);
    glVertex2f(cx+py*sc,cy-px*sc); glVertex2f(cx-py*sc,cy-px*sc);
}
void mpCircle(float cx,float cy,float r){
    float sc=r/100.0f; int x=0,y=100,d=1-100;
    glBegin(GL_POINTS); plotPts(cx,cy,x,y,sc);
    while(x<y){ if(d<0)d+=2*x+3; else{d+=2*(x-y)+5;y--;} x++; plotPts(cx,cy,x,y,sc); }
    glEnd();
}
void fillCircle(float cx,float cy,float r){ for(float ri=r;ri>0.002f;ri-=0.002f) mpCircle(cx,cy,ri); }

// ══════════════════════════════════════
//  COSINE SMOOTH HILLS
// ══════════════════════════════════════
struct Pt{ float x,y; };
float cosInterp(float a,float b,float t){ float f=(1-cos(t*3.14159f))*0.5f; return a*(1-f)+b*f; }
float hillY(Pt* pk,int n,float x){
    for(int i=0;i<n-1;i++) if(x>=pk[i].x&&x<=pk[i+1].x){
        float t=(x-pk[i].x)/(pk[i+1].x-pk[i].x); return cosInterp(pk[i].y,pk[i+1].y,t);
    }
    return HILL_BASE;
}

// Hill peaks – all y values are ABOVE HILL_BASE (0.35)
Pt farPk[]={
    {-1.2f,0.60f},
    {-1.0f,0.65f},
    {-0.85f,0.78f},
    {-0.65f,0.55f},
    {-0.45f,0.82f},
    {-0.22f,0.60f},
    {0.00f,0.88f},
    {0.22f,0.62f},
    {0.44f,0.80f},
    {0.65f,0.55f},
    {0.84f,0.74f},
    {1.0f,0.65f},
    {1.2f,0.60f}
};
int FN = 13;
Pt midPk[]={
    {-1.2f,0.55f},
    {-1.0f,0.58f},
    {-0.80f,0.64f},
    {-0.58f,0.44f},
    {-0.36f,0.68f},
    {-0.12f,0.46f},
    {0.10f,0.72f},
    {0.30f,0.48f},
    {0.52f,0.66f},
    {0.72f,0.42f},
    {0.88f,0.60f},
    {1.0f,0.58f},
    {1.2f,0.55f}
};
int MN = 13;

Pt nearPk[]={
    {-1.2f,0.50f},
    {-1.0f,0.52f},
    {-0.75f,0.52f},
    {-0.52f,0.37f},
    {-0.30f,0.54f},
    {-0.08f,0.38f},
    {0.14f,0.55f},
    {0.35f,0.36f},
    {0.56f,0.52f},
    {0.76f,0.35f},
    {0.91f,0.48f},
    {1.0f,0.52f},
    {1.2f,0.50f}
};
int NN = 13;


void drawHillLayer(Pt* pk,int n,float r,float g,float b2){
    float f=br(); glColor3f(r*f,g*f,b2*f);
    glBegin(GL_POLYGON);
    glVertex2f(-1.0f,HILL_BASE);
    for(int i=0;i<=400;i++){ float x=-1.0f+i*(2.0f/400); glVertex2f(x,hillY(pk,n,x)); }
    glVertex2f(1.0f,HILL_BASE);
    glEnd();
}

// Trees growing ON the hill surface (dark, realistic forest)
void hillForest(Pt* pk,int n,float* xs,int cnt,float sz,float r,float g,float b2){
    float f=br();
    for(int i=0;i<cnt;i++){
        float tx=xs[i], ty=hillY(pk,n,tx);
        // trunk
        glColor3f(0.15f*f,0.08f*f,0.02f*f);
        glPointSize(3.0f);
        bLine(tx,ty,tx,ty+sz*0.5f);
        glPointSize(1.0f);
        // canopy – 3 overlapping dark circles
        glColor3f(r*f,g*f,b2*f);
        fillCircle(tx,       ty+sz*0.9f, sz*0.85f);
        fillCircle(tx-sz*0.5f,ty+sz*0.6f,sz*0.70f);
        fillCircle(tx+sz*0.5f,ty+sz*0.65f,sz*0.70f);
        // slightly lighter top
        glColor3f((r+0.04f)*f,(g+0.05f)*f,(b2+0.03f)*f);
        fillCircle(tx,ty+sz*1.2f,sz*0.50f);
    }
}

// ══════════════════════════════════════
//  ZONE 1: SKY
// ══════════════════════════════════════
void drawSky(){
    float f=br();
    glBegin(GL_QUADS);
        glColor3f(0.01f+0.20f*f,0.01f+0.42f*f,0.06f+0.66f*f);
        glVertex2f(-1,1); glVertex2f(1,1);
        glColor3f(0.04f+0.48f*f,0.04f+0.58f*f,0.09f+0.68f*f);
        glVertex2f(1,HILL_BASE); glVertex2f(-1,HILL_BASE);
    glEnd();
}

void drawStars(){
    float f = br();
    if(f > 0.65f) return;   // only night

    float sf = 1.0f - f/0.65f;
    glColor3f(sf, sf, sf);
    glPointSize(2.0f);

    glBegin(GL_POINTS);

    // fixed star positions (no flicker)
    float sx[] = {-0.9f,-0.7f,-0.5f,-0.3f,-0.1f,0.1f,0.3f,0.5f,0.7f,0.9f,
                  -0.8f,-0.6f,-0.4f,-0.2f,0.0f,0.2f,0.4f,0.6f,0.8f};

    float sy[] = {0.85f,0.78f,0.92f,0.75f,0.88f,0.80f,0.90f,0.76f,0.84f,0.79f,
                  0.70f,0.82f,0.74f,0.86f,0.77f,0.89f,0.73f,0.83f,0.71f};

    for(int i=0;i<19;i++)
        glVertex2f(sx[i], sy[i]);

    glEnd();
    glPointSize(1.0f);

}
void drawMan(float x, float y){
    float f = br();


    // body
    col(0.2f,0.3f,0.8f);
    glBegin(GL_QUADS);
        glVertex2f(x-0.01f,y);
        glVertex2f(x+0.01f,y);
        glVertex2f(x+0.01f,y+0.03f);
        glVertex2f(x-0.01f,y+0.03f);
    glEnd();

    // head
    col(1,0.8f,0.6f);
    fillCircle(x, y+0.045f, 0.012f);

    // legs
    float step = 0.01f * sin(waveTime * 5);
    ddaLine(x,y,x-0.01f+step,y-0.02f);
    ddaLine(x,y,x+0.01f-step,y-0.02f);
}
void drawMoon(){
    float f = br();
    if(f > 0.65f) return;   // only night

    float mx = 0.75f;   // right side
    float my = 0.80f;

    // 🌕 main circle (white)
    glColor3f(1.0f, 1.0f, 1.0f);
    fillCircle(mx, my, 0.055f);

    // 🌑 cut circle (same vertical level → no tilt)
    float skyR = 0.01f + 0.20f*f;
    float skyG = 0.01f + 0.42f*f;
    float skyB = 0.06f + 0.66f*f;

    glColor3f(skyR, skyG, skyB);

    // 👉 key fix: same Y, only X shifted
    fillCircle(mx + 0.030f, my, 0.050f);
}
void drawSun(){
    float f = br();

    float sunX = -0.25f;

    // 🌄 real animated position
    float sunVisibleY = sunY;

    // get hill height at this X
    float hillTop = hillY(farPk, FN, sunX);

    // 🌄 limit so sun starts behind hill but can rise above
    if(sunVisibleY < hillTop - 0.08f)
        sunVisibleY = hillTop - 0.08f;

    // 🔥 GLOW (RED → YELLOW GRADIENT)
    for(float ri = 0.12f; ri > 0.05f; ri -= 0.01f){
        float t = (ri - 0.05f) / (0.12f - 0.05f); // gradient factor

        // outer = red, inner = yellow
        float r = 1.0f;
        float g = 0.3f + 0.6f * (1.0f - t);
        float b = 0.0f;

        glColor3f(r*f, g*f, b*f);
        mpCircle(sunX, sunVisibleY, ri);
    }

    // ☀️ CORE (bright yellow)
    glColor3f(1.0f*f, 0.9f*f, 0.1f*f);
    fillCircle(sunX, sunVisibleY, 0.07f);

    // ☀️ RAYS (slightly animated feel)
    glColor3f(1.0f*f, 0.85f*f, 0.2f*f);
    glPointSize(1.5f);

    for(int i = 0; i < 10; i++){
        float a = i * 36 * 3.14159f / 180.0f;

        float len = 0.08f + 0.01f * sin(waveTime + i); // subtle motion

        ddaLine(sunX, sunVisibleY,
                sunX + len * cos(a),
                sunVisibleY + len * sin(a));
    }

    glPointSize(1.0f);
}
void drawCloud(float x,float y){
    float f=br(); float w=0.90f*f+0.10f; glColor3f(w,w,w);
    fillCircle(x,       y,       0.065f);
    fillCircle(x+0.078f,y+0.024f,0.050f);
    fillCircle(x-0.072f,y+0.016f,0.048f);
    fillCircle(x+0.140f,y-0.004f,0.038f);
    fillCircle(x-0.135f,y-0.006f,0.033f);
}

// Hills: 3 layers, each darker toward front, dense forest on each
void drawHills(){
    // Layer 1 – farthest, blue-grey
   drawHillLayer(farPk,FN, 0.20f,0.25f,0.35f);
 float ft[]={-0.95f,-0.75f,-0.50f,-0.25f,0.0f,0.25f,0.50f,0.75f,0.95f};
   hillForest(farPk,FN,ft,9,0.030f, 0.08f,0.16f,0.06f);

    // Layer 2 – mid, dark forest green
    drawHillLayer(midPk,MN, 0.10f,0.25f,0.09f);
    float mt[]={-0.72f,-0.46f,-0.20f,0.06f,0.28f,0.54f,0.76f};
    hillForest(midPk,MN,mt,7,0.038f, 0.06f,0.18f,0.05f);

    // Layer 3 – nearest, darkest green (dense)
    drawHillLayer(nearPk,NN, 0.07f,0.20f,0.06f);
    float nt[]={-0.68f,-0.44f,-0.18f,0.06f,0.26f,0.48f,0.70f,0.88f};
    hillForest(nearPk,NN,nt,8,0.044f, 0.04f,0.14f,0.03f);

}

void drawBirds(float bx,float by){
    col(0.04f,0.04f,0.04f); glPointSize(2.0f);
    for(int i=0;i<5;i++){
        float ox=bx+i*0.082f,oy=by+(i%2)*0.024f;
        ddaLine(ox-0.022f,oy+0.014f,ox,oy);
        ddaLine(ox,oy,ox+0.022f,oy+0.014f);
    }
    glPointSize(1.0f);
}

// ══════════════════════════════════════
//  ZONE 2: RIVER  (y = 0.00 to 0.35)
//  NOTHING from village goes here
// ══════════════════════════════════════
void drawRiver(){
    float f=br();
    float rMid=(HILL_BASE+RIVER_BOT)*0.5f;  // 0.175

    // top grass bank strip
    col(0.16f,0.48f,0.09f);
    glBegin(GL_QUADS);
        glVertex2f(-1,HILL_BASE); glVertex2f(1,HILL_BASE);
        glVertex2f(1,HILL_BASE-0.02f); glVertex2f(-1,HILL_BASE-0.02f);
    glEnd();

    // water body
    glColor3f(0.06f+0.12f*f,0.25f+0.32f*f,0.54f+0.24f*f);
    glBegin(GL_QUADS);
        glVertex2f(-1,RIVER_BOT); glVertex2f(1,RIVER_BOT);
        glVertex2f(1,HILL_BASE-0.02f); glVertex2f(-1,HILL_BASE-0.02f);
    glEnd();

    // wavy top shoreline
    col(0.16f,0.48f,0.09f);
    glBegin(GL_POLYGON);
    glVertex2f(-1,HILL_BASE);
    for(int j=0;j<=130;j++){
        float wx=-1.0f+j*(2.0f/130);
        float wy=HILL_BASE-0.02f+0.022f*sin(wx*8.0f+waveTime*0.7f)
                                +0.009f*sin(wx*16.0f-waveTime);
        glVertex2f(wx,wy);
    }
    glVertex2f(1,HILL_BASE); glEnd();

    // wavy bottom shoreline
    col(0.18f,0.50f,0.10f);
    glBegin(GL_POLYGON);
    glVertex2f(-1,RIVER_BOT);
    for(int j=0;j<=130;j++){
        float wx=-1.0f+j*(2.0f/130);
        float wy=RIVER_BOT+0.002f-0.018f*sin(wx*7.5f+waveTime*0.6f+2.0f)
                                  -0.008f*sin(wx*15.0f-waveTime*0.8f);
        glVertex2f(wx,wy);
    }
    glVertex2f(1,RIVER_BOT); glEnd();

    // sine wave rows on water
    for(int i=0;i<12;i++){
        float wy=HILL_BASE-0.04f-i*((HILL_BASE-RIVER_BOT-0.05f)/12.0f);
        float ph=waveTime*1.4f+i*0.65f;
        float amp=0.004f+0.003f*sin(i*1.3f);
        float dep=1.0f-i*0.06f; if(dep<0.28f)dep=0.28f;
        glColor3f(0.20f*f*dep,0.55f*f*dep,0.88f*f*dep);
        glBegin(GL_LINE_STRIP);
        for(int j=0;j<=120;j++){
            float wx=-1.0f+j*(2.0f/120);
            float wvy=wy+amp*sin(wx*20.0f+ph)+amp*0.5f*sin(wx*32.0f-ph*0.7f);
            glVertex2f(wx,wvy);
        }
        glEnd();
    }
    // shimmer – DDA
    glColor3f(0.60f*f,0.78f*f,0.95f*f); glPointSize(1.5f);
    for(int i=0;i<8;i++){
        float rx=-0.85f+i*0.22f+0.05f*sin(waveTime+i*1.6f);
        float ry=rMid+0.03f*sin(waveTime*0.8f+i);
        ddaLine(rx,ry,rx+0.07f,ry+0.003f);
    }
    glPointSize(1.0f);
}

// Boat: y clamped strictly inside river
void drawBoat(float bx){
    float f=br();
    float rMid=(HILL_BASE+RIVER_BOT)*0.5f;
    float by=rMid+0.006f*sin(waveTime*2.8f+bx*5.0f);
    // strict clamp inside river
    float topLim=HILL_BASE-0.07f, botLim=RIVER_BOT+0.05f;
    if(by>topLim)by=topLim; if(by<botLim)by=botLim;

    col(0.46f,0.22f,0.06f);
    glBegin(GL_POLYGON);
        glVertex2f(bx-0.09f,by+0.010f); glVertex2f(bx+0.09f,by+0.010f);
        glVertex2f(bx+0.07f,by-0.030f); glVertex2f(bx-0.07f,by-0.030f);
    glEnd();
    col(0.26f,0.12f,0.03f); glPointSize(2.0f);
    bLine(bx-0.09f,by+0.010f,bx+0.09f,by+0.010f);
    bLine(bx+0.09f,by+0.010f,bx+0.07f,by-0.030f);
    bLine(bx+0.07f,by-0.030f,bx-0.07f,by-0.030f);
    bLine(bx-0.07f,by-0.030f,bx-0.09f,by+0.010f);
    glPointSize(1.0f);
    col(0.30f,0.15f,0.04f); glPointSize(2.5f);
    ddaLine(bx,by+0.010f,bx,by+0.125f); glPointSize(1.0f);
    col(0.94f,0.90f,0.80f);
    glBegin(GL_TRIANGLES);
        glVertex2f(bx,by+0.125f); glVertex2f(bx,by+0.016f); glVertex2f(bx+0.08f,by+0.072f);
    glEnd();
    col(0.58f,0.54f,0.46f); glPointSize(1.5f);
    bLine(bx,by+0.125f,bx+0.08f,by+0.072f);
    bLine(bx+0.08f,by+0.072f,bx,by+0.016f); glPointSize(1.0f);
    // boatman
    col(0.10f,0.06f,0.02f);
    fillCircle(bx-0.015f,by+0.024f,0.011f);
    bLine(bx-0.015f,by+0.013f,bx-0.015f,by-0.003f);
}

void drawDuck(float x){
    float f=br();
    float dy=RIVER_BOT+0.07f+0.005f*sin(waveTime*2.0f+x*6.0f);
    if(dy>HILL_BASE-0.06f)dy=HILL_BASE-0.06f;
    glColor3f(0.92f*f,0.92f*f,0.92f*f);
    fillCircle(x,dy,0.022f);
    fillCircle(x+0.027f,dy+0.017f,0.013f);
    glColor3f(1.0f*f,0.58f*f,0.0f);
    glBegin(GL_TRIANGLES);
        glVertex2f(x+0.039f,dy+0.017f);
        glVertex2f(x+0.052f,dy+0.015f);
        glVertex2f(x+0.039f,dy+0.010f);
    glEnd();
}

void drawFish(float ft){
    float f=br();
    float ph=fmod(ft,4.0f); if(ph>1.2f)return;
    float fx=0.38f+0.09f*ph;
    float fy=HILL_BASE-0.04f+0.04f*sin(ph*3.14159f/1.2f);
    if(fy>HILL_BASE-0.02f)fy=HILL_BASE-0.02f;
    if(fy<RIVER_BOT+0.02f)fy=RIVER_BOT+0.02f;
    glColor3f(0.14f*f,0.62f*f,0.72f*f);
    fillCircle(fx,fy,0.017f);
    glBegin(GL_TRIANGLES);
        glVertex2f(fx-0.017f,fy); glVertex2f(fx-0.038f,fy+0.013f); glVertex2f(fx-0.038f,fy-0.013f);
    glEnd();
}

// Small tree on river bank (base at RIVER_BOT)
void drawBankTree(float x,float sz){
    float y=RIVER_BOT+0.01f;
    col(0.24f,0.12f,0.04f); glPointSize(5.0f);
    bLine(x,y,x,y+sz*0.45f); glPointSize(1.0f);
    col(0.05f,0.35f,0.07f);  fillCircle(x,y+sz*0.52f,sz*0.20f);
    col(0.08f,0.44f,0.09f);
    fillCircle(x-sz*0.08f,y+sz*0.58f,sz*0.16f);
    fillCircle(x+sz*0.08f,y+sz*0.59f,sz*0.16f);
}

// ══════════════════════════════════════
//  ZONE 3: GREEN FIELD (y < 0.0)
//
//  Village LEFT side:  x < -0.08
//  Playground RIGHT:   x >  0.12
//
//  ALL huts placed at:
//    base y = RIVER_BOT - 0.12  (well below river)
//    which = -0.12 in GL coords (safely on green field)
// ══════════════════════════════════════
void drawField(){
    // base grass
    col(0.17f,0.50f,0.10f);
    glBegin(GL_QUADS);
        glVertex2f(-1,-1); glVertex2f(1,-1);
        glVertex2f(1,RIVER_BOT); glVertex2f(-1,RIVER_BOT);
    glEnd();
    // lighter strip at top of field
    col(0.21f,0.56f,0.12f);
    glBegin(GL_QUADS);
        glVertex2f(-1,RIVER_BOT);     glVertex2f(1,RIVER_BOT);
        glVertex2f(1,RIVER_BOT-0.04f);glVertex2f(-1,RIVER_BOT-0.04f);
    glEnd();
    // centre dirt path
    col(0.50f,0.36f,0.12f);
    glBegin(GL_POLYGON);
        glVertex2f(-0.07f,RIVER_BOT); glVertex2f(0.07f,RIVER_BOT);
        glVertex2f(0.14f,-1.0f);      glVertex2f(-0.14f,-1.0f);
    glEnd();
    col(0.34f,0.22f,0.07f); glPointSize(2.0f);
    bLine(-0.07f,RIVER_BOT,-0.14f,-1.0f);
    bLine( 0.07f,RIVER_BOT, 0.14f,-1.0f);
    glPointSize(1.0f);
}

// ── VILLAGE objects – all y < RIVER_BOT (0.0) ──

// Hut: base at y, height h.  Call with y well below 0.0
void drawHut(float x,float y,float w,float h){
    // wall
    col(0.68f,0.50f,0.28f);
    glBegin(GL_QUADS);
        glVertex2f(x,y); glVertex2f(x+w,y);
        glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
    col(0.36f,0.22f,0.08f); glPointSize(2.0f);
    bLine(x,y,x+w,y); bLine(x+w,y,x+w,y+h);
    bLine(x+w,y+h,x,y+h); bLine(x,y+h,x,y);
    glPointSize(1.0f);
    // door
    col(0.22f,0.11f,0.04f);
    float dw=w*0.22f,dh=h*0.44f,dx=x+w*0.38f;
    glBegin(GL_QUADS);
        glVertex2f(dx,y); glVertex2f(dx+dw,y);
        glVertex2f(dx+dw,y+dh); glVertex2f(dx,y+dh);
    glEnd();
    // window
    col(0.42f,0.68f,0.84f);
    float wx=x+w*0.65f,wy2=y+h*0.52f,ws=w*0.17f;
    glBegin(GL_QUADS);
        glVertex2f(wx,wy2); glVertex2f(wx+ws,wy2);
        glVertex2f(wx+ws,wy2+ws); glVertex2f(wx,wy2+ws);
    glEnd();
    col(0.24f,0.13f,0.05f); glPointSize(1.5f);
    bLine(wx,wy2,wx+ws,wy2+ws); bLine(wx+ws,wy2,wx,wy2+ws); glPointSize(1.0f);
    // thatched roof
    float apex=y+h+h*0.58f,cx=x+w*0.5f;
    col(0.74f,0.67f,0.29f);
    glBegin(GL_TRIANGLES);
        glVertex2f(x-w*0.07f,y+h); glVertex2f(x+w+w*0.07f,y+h); glVertex2f(cx,apex);
    glEnd();
    col(0.51f,0.45f,0.17f); glPointSize(1.5f);
    for(int i=0;i<=9;i++) ddaLine(x-w*0.07f+i*(w*1.14f/9),y+h,cx,apex);
    glPointSize(1.0f);
    col(0.37f,0.30f,0.11f); glPointSize(2.0f);
    bLine(x-w*0.07f,y+h,cx,apex); bLine(x+w+w*0.07f,y+h,cx,apex);
    glPointSize(1.0f);
}

void drawWell(float x,float y){
    col(0.48f,0.24f,0.09f);
    glBegin(GL_QUADS);
        glVertex2f(x-0.042f,y-0.076f); glVertex2f(x+0.042f,y-0.076f);
        glVertex2f(x+0.042f,y);        glVertex2f(x-0.042f,y);
    glEnd();
    col(0.28f,0.13f,0.04f); glPointSize(3.0f);
    mpCircle(x,y,0.042f);
    bLine(x-0.033f,y,x-0.033f,y+0.062f);
    bLine(x+0.033f,y,x+0.033f,y+0.062f);
    bLine(x-0.042f,y+0.062f,x+0.042f,y+0.062f);
    col(0.48f,0.32f,0.12f); ddaLine(x,y+0.062f,x,y+0.015f);
    glPointSize(1.0f);
    col(0.26f,0.26f,0.70f); fillCircle(x,y+0.011f,0.008f);
}

void drawFence(float x,float y,float len){
    col(0.44f,0.27f,0.09f); glPointSize(2.5f);
    ddaLine(x,y+0.025f,x+len,y+0.025f);
    ddaLine(x,y+0.012f,x+len,y+0.012f);
    int n=(int)(len/0.034f);
    for(int i=0;i<=n;i++) bLine(x+i*(len/n),y,x+i*(len/n),y+0.034f);
    glPointSize(1.0f);
}

void drawVillageTree(float x,float y,float sz){
    col(0.24f,0.12f,0.04f); glPointSize(6.0f);
    bLine(x,y,x,y+sz*0.50f); glPointSize(1.0f);
    col(0.05f,0.37f,0.07f); fillCircle(x,y+sz*0.55f,sz*0.22f);
    col(0.08f,0.46f,0.10f);
    fillCircle(x-sz*0.09f,y+sz*0.61f,sz*0.17f);
    fillCircle(x+sz*0.09f,y+sz*0.62f,sz*0.17f);
    col(0.12f,0.55f,0.12f); fillCircle(x,y+sz*0.69f,sz*0.12f);
}

void drawCow(float x,float y){
    float f=br();
    // body
    if(isDay)
    col(0.88f,0.88f,0.88f);
else
    col(0.6f,0.6f,0.6f);   // darker cow at night
    glBegin(GL_QUADS);
        glVertex2f(x-0.062f,y);      glVertex2f(x+0.062f,y);
        glVertex2f(x+0.062f,y+0.046f);glVertex2f(x-0.062f,y+0.046f);
    glEnd();
    glColor3f(0.08f*f,0.08f*f,0.08f*f);
    fillCircle(x-0.016f,y+0.028f,0.013f);
    fillCircle(x+0.034f,y+0.010f,0.010f);
    glColor3f(0.82f*f,0.82f*f,0.82f*f);
    fillCircle(x+0.080f,y+0.036f,0.020f);
    glColor3f(0,0,0); fillCircle(x+0.088f,y+0.042f,0.004f);
    col(0.64f,0.64f,0.64f); glPointSize(3.0f);
    float loff=0.004f*sin(waveTime*3.0f);
    bLine(x-0.046f,y,x-0.046f+loff,y-0.042f);
    bLine(x-0.014f,y,x-0.014f-loff,y-0.042f);
    bLine(x+0.018f,y,x+0.018f+loff,y-0.042f);
    bLine(x+0.050f,y,x+0.050f-loff,y-0.042f);
    glPointSize(1.0f);
    col(0.62f,0.62f,0.62f);
    ddaLine(x-0.062f,y+0.025f,x-0.090f,y+0.052f);
}

void drawFlower(float x,float y,float r,float g,float b){
    float f = br();

    // 🌿 Stem (slightly curved using small segments)
    col(0.10f,0.45f,0.05f);
    glPointSize(2.0f);

    for(int i=0;i<10;i++){
        float t = i / 10.0f;
        float cx = x + 0.01f * sin(t * 2.0f); // slight curve
        float cy = y - 0.05f * t;
        glBegin(GL_POINTS);
        glVertex2f(cx, cy);
        glEnd();
    }

    glPointSize(1.0f);

    // 🌿 Leaves (two sides)
    col(0.12f,0.55f,0.08f);
    fillCircle(x - 0.015f, y - 0.03f, 0.01f);
    fillCircle(x + 0.015f, y - 0.02f, 0.01f);

    // 🌸 Petals (radial)
    int petals = 8;
    float radius = 0.02f;

    for(int i=0;i<petals;i++){
        float angle = (2 * 3.1416f * i) / petals;

        float px = x + cos(angle) * radius;
        float py = y + sin(angle) * radius;

        // slight color variation for realism
        float shade = 0.8f + 0.2f * sin(i);

        glColor3f(r * shade * f, g * shade * f, b * shade * f);
        fillCircle(px, py, 0.012f);
    }

    // 🌼 Center (yellow + darker ring)
    col(1.0f,0.85f,0.1f);
    fillCircle(x, y, 0.010f);

    col(0.8f,0.5f,0.0f);
    mpCircle(x, y, 0.010f);
}

void drawButterfly(float x,float y,float t){
    float f = br();

    // 🦋 wing flap animation
    float flap = 0.02f * sin(t);

    // LEFT WINGS
    col(1.0f,0.3f,0.6f);
    fillCircle(x - 0.02f, y + flap, 0.02f);

    col(1.0f,0.5f,0.8f);
    fillCircle(x - 0.03f, y - 0.01f + flap, 0.018f);

    // RIGHT WINGS
    col(1.0f,0.3f,0.6f);
    fillCircle(x + 0.02f, y + flap, 0.02f);

    col(1.0f,0.5f,0.8f);
    fillCircle(x + 0.03f, y - 0.01f + flap, 0.018f);

    // 🐛 BODY
    col(0.1f,0.1f,0.1f);
    fillCircle(x, y, 0.008f);

    // ANTENNA
    glPointSize(1.5f);
    ddaLine(x, y+0.01f, x-0.01f, y+0.02f);
    ddaLine(x, y+0.01f, x+0.01f, y+0.02f);
    glPointSize(1.0f);
}

// ── PLAYGROUND (right side of green field) ──
void drawFootballField(){
    // 👉 RIGHT SIDE ONLY
    float left   = 0.20f;
    float right  = 0.95f;
    float top    = RIVER_BOT - 0.02f;
    float bottom = -0.85f;

    // grass
    col(0.15f,0.55f,0.15f);
    glBegin(GL_QUADS);
        glVertex2f(left,bottom);
        glVertex2f(right,bottom);
        glVertex2f(right,top);
        glVertex2f(left,top);
    glEnd();

col(0.12f,0.45f,0.12f);
    glPointSize(2);

    // border
    bLine(left,top,right,top);
    bLine(left,bottom,right,bottom);
    bLine(left,top,left,bottom);
    bLine(right,top,right,bottom);

    // middle line
    float mid = (left + right)/2;
    bLine(mid,top,mid,bottom);

    // center circle
    mpCircle(mid,(top+bottom)/2,0.10f);

    // LEFT goal
    bLine(left, -0.30f, left+0.08f, -0.30f);
    bLine(left+0.08f, -0.30f, left+0.08f, -0.45f);
    bLine(left+0.08f, -0.45f, left, -0.45f);

    // RIGHT goal
    bLine(right, -0.30f, right-0.08f, -0.30f);
    bLine(right-0.08f, -0.30f, right-0.08f, -0.45f);
    bLine(right-0.08f, -0.45f, right, -0.45f);

    glPointSize(1);
}

void drawChild(float x,float y,float ct,float r,float g,float b2){
    float f=br();

    float leg = 0.015f*sin(ct);
    float arm = 0.015f*sin(ct+1.5f);
    float tilt= 0.005f*sin(ct);

    // body (tilted)
    if(isDay)
    col(r,g,b2);
else
    col(r*0.7f, g*0.7f, b2*0.7f);   // 🔥 slightly dim but still visible
    glBegin(GL_QUADS);
        glVertex2f(x-0.015f+tilt,y+0.009f);
        glVertex2f(x+0.015f+tilt,y+0.009f);
        glVertex2f(x+0.015f-tilt,y+0.036f);
        glVertex2f(x-0.015f-tilt,y+0.036f);
    glEnd();

    // head (smaller)
    glColor3f(0.7f*f,0.5f*f,0.3f*f);
    fillCircle(x,y+0.048f,0.013f);

    // legs
    col(0.26f,0.16f,0.07f);
    glPointSize(2.5f);
    bLine(x,y+0.009f,x-0.013f+leg,y-0.020f);
    bLine(x,y+0.009f,x+0.013f-leg,y-0.020f);

    // feet
    bLine(x-0.013f+leg,y-0.020f,x-0.021f+leg,y-0.027f);
    bLine(x+0.013f-leg,y-0.020f,x+0.021f-leg,y-0.027f);

    // arms
    col(0.85f,0.64f,0.38f);
    glPointSize(2.0f);
    ddaLine(x-0.015f,y+0.028f,x-0.030f+arm,y+0.014f);
    ddaLine(x+0.015f,y+0.028f,x+0.030f-arm,y+0.014f);

    glPointSize(1.0f);
}

void drawBall(){
    col(1,1,1);
    fillCircle(ballX, ballY, 0.015f);

    col(0,0,0);
    mpCircle(ballX, ballY, 0.015f);
}

void drawSeesaw(float x,float y){
    float tilt=0.042f*sin(waveTime*1.2f);
    col(0.62f,0.30f,0.08f); glPointSize(3.0f);
    bLine(x,y,x,y+0.052f); glPointSize(1.0f);
    col(0.60f,0.36f,0.12f); glPointSize(2.0f);
    bLine(x-0.10f,y+0.052f-tilt,x+0.10f,y+0.052f+tilt);
    glPointSize(1.0f);
    drawChild(x-0.09f,y+0.052f-tilt,  waveTime*3.0f, 0.80f,0.18f,0.18f);
    drawChild(x+0.09f,y+0.052f+tilt, -waveTime*3.0f, 0.18f,0.38f,0.90f);
}


    // ❌ CHILD REMOVED → now swing is empty


// ══════════════════════════════════════
//  DISPLAY
// ══════════════════════════════════════
void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glRotatef(rotY,0,1,0);
    glRotatef(rotX,1,0,0);
    glTranslatef(transX,transY,0);
    glScalef(scaleS,scaleS,1.0f);

    // ── ZONE 1: SKY + HILLS ──
    drawSky();
    drawStars();
 drawSun();
drawMoon();
drawHills();

    drawCloud(cloudX, 0.80f);
    drawCloud(cloud2X,0.88f);
    drawBirds(birdX,0.68f);

    // ── ZONE 2: RIVER ──
    drawRiver();
    drawBoat(boatX);
    drawDuck(duckX1);
    drawDuck(duckX2);
    drawFish(fishT);
    drawBankTree(-0.84f,0.24f);
    drawBankTree(-0.60f,0.20f);
    drawBankTree( 0.70f,0.22f);
    drawBankTree( 0.88f,0.19f);


    // ── ZONE 3: FIELD ──
    drawField();
   drawMan(manX, manY);

    // LEFT SIDE (VILLAGE)
    float HY = RIVER_BOT - 0.20f;

    drawVillageTree(-0.94f, RIVER_BOT-0.02f, 0.36f);
    drawVillageTree(-0.60f, RIVER_BOT-0.02f, 0.32f);

    drawHut(-0.98f, HY, 0.26f, 0.21f);
    drawHut(-0.75f, HY, 0.22f, 0.18f);
    drawHut(-0.55f, HY, 0.18f, 0.15f);

    drawWell(-0.18f, RIVER_BOT-0.14f);
    drawFence(-0.96f, RIVER_BOT-0.06f, 0.60f);

    // cows (bottom left)
drawCow(cowX1, cowY1);
drawCow(cowX2, cowY2);

    // flowers left
float flx[] = {-0.92f,-0.80f,-0.68f,-0.56f,-0.44f};
float flr[][3] = {
    {1,1,1},
    {1,0.2f,0.2f},
    {1,0.8f,0},
    {0.4f,0.5f,1},
    {0.8f,0.1f,0.8f}
};

for(int i=0;i<5;i++){
    drawFlower(flx[i], -0.92f, flr[i][0], flr[i][1], flr[i][2]);
}

    // ── RIGHT SIDE (PLAYGROUND) ──
   drawFootballField();

float baseY = -0.60f;

drawChild(playerX[0], baseY, childT, 1,0,0);
drawChild(playerX[1], baseY+0.05f, childT, 1,0,0);

drawChild(playerX[2], baseY, childT, 0,0,1);
drawChild(playerX[3], baseY+0.05f, childT, 0,0,1);

drawChild(playerX[4], baseY, childT, 1,1,0);
drawChild(playerX[5], baseY+0.05f, childT, 1,1,0);
    drawVillageTree(1.00f,RIVER_BOT-0.02f,0.30f);

    float cy = RIVER_BOT - 0.22f;

    // players
    drawChild(0.50f, cy, childT*3.5f, 0.9f,0.2f,0.2f);
    drawChild(0.62f, cy, childT*3.5f+1, 0.2f,0.5f,0.9f);

    drawBall();

    // seesaw
    drawSeesaw(-0.55f, RIVER_BOT-0.45f);

    // flowers right
    float frx[]={0.68f,0.80f,0.94f};
    float frr[][3]={{1,1,0},{1,.4f,.7f},{.3f,1,.5f}};
    for(int i=0;i<3;i++)
        drawFlower(frx[i],-0.92f,frr[i][0],frr[i][1],frr[i][2]);

    // ✅ BUTTERFLY (FINAL CORRECT POSITION)
 // Butterfly 1
drawButterfly(butterflyX,
              -0.90f + 0.03f*sin(butterflyT),
              butterflyT);

// Butterfly 2 (different path)
drawButterfly(butterflyX + 0.15f,
              -0.85f + 0.04f*cos(butterflyT + 2),
              butterflyT + 2);

    glutSwapBuffers();
}

// ══════════════════════════════════════
//  TIMER
// ══════════════════════════════════════
void timer(int){
   if(isDay){
    if(sunY < 0.70f) sunY += 0.0005f;
}
    cloudX  +=0.00035f; if(cloudX >1.6f) cloudX =-1.6f;
    cloud2X +=0.00023f; if(cloud2X>1.6f) cloud2X=-1.6f;
    birdX   +=0.00165f; if(birdX  >1.6f) birdX  =-1.6f;
manY += 0.0006f;

// start from bottom, go up toward river
if(manY > -0.05f)   // near river edge
    manY = -0.90f;  // reset to bottom

// road width is roughly -0.07 to 0.07
if(manX > 0.07f) manX = -0.07f;
    waveTime+=0.038f;
    boatX+=0.00055f; if(boatX>0.90f) boatX=-0.90f;
    fishT+=0.013f;
    duckX1+=0.00016f; if(duckX1>-0.12f) duckX1=-0.88f;
    duckX2+=0.00012f; if(duckX2> 0.08f) duckX2=-0.68f;
    butterflyT+=0.035f;

// butterfly moves only in RIGHT playground
butterflyX += 0.0005f;

// keep it inside right side only
if(butterflyX > 0.95f) butterflyX = 0.20f;
// 🐄 natural grazing motion

cowX1 += 0.00020f;
cowX2 += 0.00025f;

// small up-down movement (eating grass effect)
cowY1 = -0.70f + 0.01f * sin(waveTime);
cowY2 = -0.78f + 0.01f * sin(waveTime + 1);

// stop near road
if(cowX1 > -0.12f) cowX1 = -0.12f;
if(cowX2 > -0.12f) cowX2 = -0.12f;
// STOP before road (center path ~ x = -0.08)
if(cowX1 > -0.12f) cowX1 = -0.12f;
if(cowX2 > -0.12f) cowX2 = -0.12f;
// ⚽ football players movement
for(int i=0;i<6;i++){
    playerX[i] += 0.0008f * playerDir[i];

    // 👉 LIMIT TO NEW FIELD AREA
    if(playerX[i] > 0.90f || playerX[i] < 0.22f)
        playerDir[i] *= -1;
}
// ⚽ Ball moves toward target player
float targetX = playerX[targetPlayer];
float targetY = -0.55f;

ballX += (targetX - ballX) * 0.02f;
ballY += (targetY - ballY) * 0.02f;

// if ball reached player → choose new player
if(fabs(ballX - targetX) < 0.01f){
    targetPlayer = rand() % 6;
}
    childT+=0.023f;
    glutPostRedisplay();
    glutTimerFunc(16,timer,0);
}

// ══════════════════════════════════════
//  KEYBOARD
// ══════════════════════════════════════
void keyboard(unsigned char key,int,int){
    switch(key){
        case '+': scaleS+=0.05f; break;
        case '-': scaleS-=0.05f; if(scaleS<0.1f)scaleS=0.1f; break;
        case 'R': rotY+=3.0f; break;
        case 'r': rotY-=3.0f; break;
        case 'T': rotX+=3.0f; break;
        case 't': rotX-=3.0f; break;

        // ✅ DAY / NIGHT CONTROL
        case 'd':
            isDay = true;
            sunY = 0.70f;
            break;

        case 'n':
            isDay = false;
            sunY = -0.10f;
            break;

        case '0': transX=0;transY=0;scaleS=1;rotY=0;rotX=0; break;
        case 27:  exit(0);
    }
    glutPostRedisplay();
}
void specialKeys(int key,int,int){
    switch(key){
        case GLUT_KEY_UP:    transY+=0.05f; break;
        case GLUT_KEY_DOWN:  transY-=0.05f; break;
        case GLUT_KEY_LEFT:  transX-=0.05f; break;
        case GLUT_KEY_RIGHT: transX+=0.05f; break;
    }
    glutPostRedisplay();
}

void init(){
    glClearColor(0,0,0,1);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(-1,1,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glPointSize(1.5f);
}

int main(int argc,char**argv){
    cout<<"========================================\n";
    cout<<"  Bangladeshi Village Scene \n";
    cout<<"========================================\n";
    cout<<"  Arrows -> 2D Translate\n";
    cout<<"  +/-    -> Zoom\n";
    cout<<"  R/r    -> 3D Rotate Y\n";
    cout<<"  T/t    -> 3D Tilt X\n";
    cout<<"  d/n    -> day/night\n";
    cout<<"  0       -> Reset\n";
    cout<<"  ESC  -> Exit\n";
    cout<<"========================================\n";
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(1100,720);
    glutInitWindowPosition(50,20);
    glutCreateWindow("Bangladeshi Village Scene ");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(16,timer,0);
    glutMainLoop();
    return 0;
}
