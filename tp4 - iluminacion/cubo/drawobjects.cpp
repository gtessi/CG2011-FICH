#include <GL/glut.h>

//==========================================
// globales

extern bool wire,relleno;
extern int plano;

//==========================================

void drawColorCube(){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();

  glDisable(GL_LIGHTING);
  glTranslated(-.5,-.5,-.5);

  // ejes
  glBegin(GL_LINES);
    glColor3d(1,0,0); glVertex3d(1.1,0,0); glVertex3d(1.5,0,0);
    glColor3d(0,1,0); glVertex3d(0,1.1,0); glVertex3d(0,1.5,0);
    glColor3d(0,0,1); glVertex3d(0,0,1.1); glVertex3d(0,0,1.5);
  glEnd();

  if (wire){
    // aristas
    glBegin(GL_LINE_STRIP); // no hago loop por el color
      glColor3f(0,0,0); glVertex3i(0,0,0);
      glColor3f(0,1,0); glVertex3i(0,1,0);
      glColor3f(1,1,0); glVertex3i(1,1,0);
      glColor3f(1,0,0); glVertex3i(1,0,0);
      glColor3f(0,0,0); glVertex3i(0,0,0); // cierra en z=0

      glColor3f(0,0,1); glVertex3i(0,0,1); // arista vertical
      glColor3f(1,0,1); glVertex3i(1,0,1);
      glColor3f(1,1,1); glVertex3i(1,1,1);
      glColor3f(0,1,1); glVertex3i(0,1,1);
      glColor3f(0,0,1); glVertex3i(0,0,1); // cierra en z=1
    glEnd();

    // las tres aristas verticales faltantes
    glBegin(GL_LINES);
      glColor3f(0,1,0); glVertex3i(0,1,0);
      glColor3f(0,1,1); glVertex3i(0,1,1);
      glColor3f(1,1,0); glVertex3i(1,1,0);
      glColor3f(1,1,1); glVertex3i(1,1,1);
      glColor3f(1,0,0); glVertex3i(1,0,0);
      glColor3f(1,0,1); glVertex3i(1,0,1);
    glEnd();
  }

  if (relleno){
    for (int i=0;i<=10;i+=2){
      float ii=float(i)/10;
      glBegin(GL_QUADS);
        if (plano==0){
          glColor4f(ii,0,0,.5f); glVertex3f(ii,0,0);
          glColor4f(ii,0,1,.5f); glVertex3f(ii,0,1);
          glColor4f(ii,1,1,.5f); glVertex3f(ii,1,1);
          glColor4f(ii,1,0,.5f); glVertex3f(ii,1,0);
        }
        else if (plano==1){
          glColor4f(0,ii,0,.5f); glVertex3f(0,ii,0);
          glColor4f(0,ii,1,.5f); glVertex3f(0,ii,1);
          glColor4f(1,ii,1,.5f); glVertex3f(1,ii,1);
          glColor4f(1,ii,0,.5f); glVertex3f(1,ii,0);
        }
        else{
          glColor4f(0,0,ii,.5f); glVertex3f(0,0,ii);
          glColor4f(0,1,ii,.5f); glVertex3f(0,1,ii);
          glColor4f(1,1,ii,.5f); glVertex3f(1,1,ii);
          glColor4f(1,0,ii,.5f); glVertex3f(1,0,ii);
        }
      glEnd();
    }
  }

  glPopAttrib();
  glPopMatrix();
}

