void bLine(float fx1,float fy1,float fx2,float fy2)
{
    int x1=(int)(fx1*700),y1=(int)(fy1*700);
    int x2=(int)(fx2*700),y2=(int)(fy2*700);
    int dx=abs(x2-x1),dy=abs(y2-y1);
    int sx=x1<x2?1:-1,sy=y1<y2?1:-1,e=dx-dy;
    glBegin(GL_POINTS);
    for(;;){
        glVertex2f(x1/700.0f,y1/700.0f);
        if(x1==x2&&y1==y2)break;
        int e2=2*e;
        if(e2>-dy){e-=dy;x1+=sx;}
        if(e2< dx){e+=dx;y1+=sy;}
    }
    glEnd();
}