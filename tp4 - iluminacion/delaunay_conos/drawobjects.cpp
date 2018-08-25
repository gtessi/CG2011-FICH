//#include <windows.h>
#include <cmath> // sin cos
#include <GL/glut.h>

//==========================================
// globales
extern int posicion_cono[100][2],cantidad;
extern float color_cono[100][3];
extern float radio;
//==========================================

void drawObjects(){

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  //propiedades del cono
  GLdouble baseRadius=radio, topRadius=0, height=1000;
  GLint slices=50, stacks=50;

  static GLUquadricObj *q=gluNewQuadric();
  gluQuadricNormals(q,GLU_SMOOTH);//GLU_FLAT
  gluQuadricOrientation(q,GLU_OUTSIDE);
  //GLU FILL, GLU LINE, GLU POINT or GLU SILHOUETTE
  gluQuadricDrawStyle(q,GLU_FILL);

  int x,y;
  float r,g,b;

  for (int j=0;j<cantidad;j++){
    x=posicion_cono[j][0]; y=posicion_cono[j][1];
    r=color_cono[j][0]; g=color_cono[j][1]; b=color_cono[j][2];
    //punto picado;
    glColor3f(1-r,1-g,1-b);
    glBegin(GL_POLYGON);
      glVertex3f(x-5,y-5,0);
      glVertex3f(x+5,y-5,0);
      glVertex3f(x+5,y+5,0);
      glVertex3f(x-5,y+5,0);
    glEnd();
    //cono
    glColor3f(r,g,b);
    glPushMatrix();
    glTranslated(x,y,-height);
    gluCylinder(q,baseRadius,topRadius,height,slices,stacks);
    glPopMatrix();
  }//end FOR;

//  gluDeleteQuadric(q);
  glPopAttrib();
}
