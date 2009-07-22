#include "Python.h"

typedef PyObject *PO;
#define PB(t) Py_BuildValue("b",(t))
#define PPT PyArg_ParseTuple

#define ch char
#define in int

ch *Scr[10];          /* table of characters */
ch *Col[10];          /* table of colors */
in w[10],h[10];       /* width and height of the screen */
ch b[9999];           /* temp buffer to work */
ch mj[10];            /* max height of the screen */
ch mi[10][100];       /* max width of a screen's line */

#define SetS(n,x,y,c) Scr[(n)][(y)*w[(n)]+(x)]=(c)
#define SetC(n,x,y,c) Col[(n)][(y)*w[(n)]+(x)]=(c)
#define GetS(n,x,y) Scr[(n)][(y)*w[(n)]+(x)]
#define GetC(n,x,y) Col[(n)][(y)*w[(n)]+(x)]
#define isblock(c) (c=='#' || c=='|' || c=='-' || c=='=' || c=='^')
#define iv(c) (c!='#' && c!='&' && c!='^' && c!='|' && c!='-')
#define newl(n,x,y) if ((y)>mj[(n)]) { mj[(n)]=(y); } if ((x)>mi[(n)][(y)]) { mi[(n)][(y)]=(x); }
#define ER return(NULL)

ch colof(ch c)
{
        /* Returns the color of a character */

        ch t='n';
        if (c=='=') t='b';
        if (c=='"' || c=='&') t='v';
        if (c=='.') t='m';
        if (c=='X') t='c';
        if (c=='?') t='r';
        return t;
}

void SFree(in n)
{
        /* Frees a screen */

        if (Scr[n]){free(Scr[n]);free(Col[n]);}
}

void SFill(in n,ch c)
{
        /* Fills a screen with the given character */

        in             i;              /* count variable */

        for (i=0;i<w[n]*h[n];i++){Scr[n][i]=c;Col[n][i]='n';}
}

ch vis(in m,in mx,in my,in x,in y,in n)
{
        in              dx=mx-x;        /* delta x */
        in              dy=my-y;        /* delta y */
        in              e=0,sx=1,sy=1,lx=0,ly=0;
                                        /* misc data */
        ch              *c;             /* map */

        if (dx<0) {dx=-dx;sx=-1;}
        if (dy<0) {dy=-dy;sy=-1;}
        if (dy==0) e=-1;
        c=Scr[m]+y*w[m]+x;

        while (x!=mx || y!=my)
        {
                n++;
                if (!iv(*c) && n>2) return 0;
                        /* the given square isn't visible */

                if (n==2 && !iv(*c))
                {
                        /* special case */
                        in sx=0,sy=0;
                        if (mx>lx) sx=1;
                        if (mx<lx) sx=-1;
                        if (my>ly) sy=1;
                        if (my<ly) sy=-1;

                        return(vis(m,mx,my,lx+sx,ly+sy,n));
                }

                lx=x;ly=y;
                if (e>=0) {y+=sy;e-=dx;c+=sy*w[m];}
                else {x+=sx;e+=dy;c+=sx;}
        }
        return 1;
}

void lookmap(in s,in m,in mx,in my)
{
        in             i,j;            /* count variables */
        in             x,y;            /* coords of current square */
        ch             c;              /* current character */

        for (j=0;j<15;j++)
        {
                for (i=0;i<29;i++)
                {
                        x=mx-14+i;
                        y=my-7+j;
                        c=GetS(m,x,y);
                        if (vis(m,mx,my,x,y,0))
                        {
                                if (x==mx && y==my) c='X';
                        }
                        else c='?';

                        if (y<0 || y>mj[m]) c='"';
                        else if (x<0 || x>mi[m][y]) c='"';

                        SetS(s,i,j+1,c);
                        SetC(s,i,j+1,colof(c));
                }
                newl(s,28,j+1);
        }
}

PO cMap_init(PO self,PO args)
{
        in             n;              /* screen number */
        in             wa,ha;          /* width and height of the screen */

        if (!PPT(args,"iii",&n,&wa,&ha)) ER;
                /* can't get the first argument */

        SFree(n);
        w[n]=wa;h[n]=ha;mj[n]=0;memset(mi[n],0,100);
        Scr[n]=malloc(wa*ha);
        Col[n]=malloc(wa*ha);
        SFill(n,' ');
        /* creates a new screen and fill it with default data */

        return PB(1);
}

PO cMap_put1(PO self,PO args)
{
        in             n;              /* screen number */
        in             x,y;            /* coords in map */
        ch            s,c;            /* char and color */

        if (!PPT(args,"iiicc",&n,&x,&y,&s,&c)) ER;
                /* can't get the arguments */

        SetS(n,x,y,s);
        SetC(n,x,y,c);
        newl(n,x,y);

        return PB(1);
}

PO cMap_put(PO self,PO args)
{
        in             n;              /* screen number */
        in             x,y;            /* coords in map */
        ch            *s;             /* string to put */
        ch            c;              /* color used */
        in             i;              /* count variable */
        ch            *sp;            /* screen pointer */
        ch            *sc;            /* color pointer */

        if (!PPT(args,"iiisc",&n,&x,&y,&s,&c)) ER;
                /* can't get the arguments */

        if (y<h[n])
        {
                sp=Scr[n]+w[n]*y+x;
                sc=Col[n]+w[n]*y+x;
                for (i=0;i<strlen(s);i++) {*(sp++)=s[i];*(sc++)=c;}
                newl(n,x+strlen(s)-1,y);
        }
        return PB(1);
}

PO cMap_get1(PO self,PO args)
{
        in             n;              /* screen number */
        in             x,y;            /* coords in map */

        if (!PPT(args,"iii",&n,&x,&y)) ER;
                /* can't get the arguments */

        return Py_BuildValue("c",GetS(n,x,y));
}

PO cMap_colof(PO self,PO args)
{
        ch            c;              /* character to test */

        if (!PPT(args,"c",&c)) ER;
                /* can't get the argument */

        return Py_BuildValue("c",colof(c));
}

PO cMap_look(PO self,PO args)
{
        in             s,m,mx,my;      /* data */

        if (!PPT(args,"iiii",&s,&m,&mx,&my)) ER;
                /* can't get the arguments */

        lookmap(s,m,mx,my);
        return PB(1);
}

PO cMap_vis(PO self,PO args)
{
        in             m,mx,my,x,y;    /* data */

        if (!PPT(args,"iiiii",&m,&mx,&my,&x,&y)) ER;
                /* can't get the arguments */

        return PB(vis(m,mx,my,x,y,0));
}

PO cMap_gets(PO self,PO args)
{
        in             m;              /* screen number */
        ch            *p;             /* working pointer to ch map */
        ch            ic;             /* initial color */
        in             i,j;            /* count variables */
        ch            c;              /* current color */

        if (!PPT(args,"i",&m)) ER;
                /* can't get the argument */

        p=b;ic='n';
        for (j=0;j<=mj[m];j++)
        {
                for (i=0;i<=mi[m][j];i++)
                {
                        c=GetC(m,i,j);
                        if (c!=ic && c!=' ')
                        {
                                ic=c;
                                *(p++)='õ';
                                *(p++)=c;
                        }
                        *(p++)=GetS(m,i,j);
                }

                *(p++)='\n';
                *(p++)='\r';
        }
        *(p++)='\0';
        return Py_BuildValue("s",b);
}

PyMethodDef cMapMeth[]={
        {"init",cMap_init,METH_VARARGS},
        {"put1",cMap_put1,METH_VARARGS},
        {"get1",cMap_get1,METH_VARARGS},
        {"colof",cMap_colof,METH_VARARGS},
        {"lookmap",cMap_look,METH_VARARGS},
        {"vis",cMap_vis,METH_VARARGS},
        {"put",cMap_put,METH_VARARGS},
        {"gets",cMap_gets,METH_VARARGS},
        {NULL,NULL}
};

void initcMap() {Py_InitModule("cMap",cMapMeth);}
        /* Initialization of the cMap module */
