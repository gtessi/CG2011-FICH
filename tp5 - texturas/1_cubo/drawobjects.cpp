#include <iostream> // cout
#include <cmath> // sin cos
#include <GL/glut.h>
using namespace std;

//==========================================
// globales

extern int texmode;
extern bool wire,relleno,cl_info;
extern float line_color[];

//==========================================

void drawColorCube(){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();

  glTranslated(-.5,-.5,-.5);

  if (relleno){
    // caras
    glBegin(GL_QUADS);
    
      // Aplicación de texturas
      // COLOR - TEXTURA - VERTICE
      // Se utiliza una sola textura de ladrillos,
      // porque las otras texturas son idénticas.
    
      //g=0 normal hacia -g
      // Pasto
      glColor3f(0,0,0); glTexCoord2f(0.0f,0.0f); glVertex3i(0,0,0);
      glColor3f(1,0,0); glTexCoord2f(0.25f,0.0f); glVertex3i(1,0,0);
      glColor3f(1,0,1); glTexCoord2f(0.25f,0.33f); glVertex3i(1,0,1);
      glColor3f(0,0,1); glTexCoord2f(0.0f,0.33f); glVertex3i(0,0,1);
    
      //b=1 normal hacia +b
      // Pared 1
      glColor3f(0,0,1); glTexCoord2f(0.0f,0.33f); glVertex3i(0,0,1);
      glColor3f(1,0,1); glTexCoord2f(0.25f,0.33f); glVertex3i(1,0,1);
      glColor3f(1,1,1); glTexCoord2f(0.25f,0.66f); glVertex3i(1,1,1);
      glColor3f(0,1,1); glTexCoord2f(0.0f,0.66f); glVertex3i(0,1,1);
      
      //r=1 normal hacia +r
      // Pared 2
      glColor3f(1,0,1); glTexCoord2f(0.0f,0.33f); glVertex3i(1,0,1);
      glColor3f(1,0,0); glTexCoord2f(0.25f,0.33f); glVertex3i(1,0,0);
      glColor3f(1,1,0); glTexCoord2f(0.25f,0.66f); glVertex3i(1,1,0);
      glColor3f(1,1,1); glTexCoord2f(0.0f,0.66f); glVertex3i(1,1,1);
    
      //b=0 normal hacia -b
      // Pared 3
      glColor3f(1,0,0); glTexCoord2f(0.0f,0.33f); glVertex3i(1,0,0);
      glColor3f(0,0,0); glTexCoord2f(0.25f,0.33f); glVertex3i(0,0,0);
      glColor3f(0,1,0); glTexCoord2f(0.25f,0.66f); glVertex3i(0,1,0);
      glColor3f(1,1,0); glTexCoord2f(0.0f,0.66f); glVertex3i(1,1,0);

      //r=0 normal hacia -r
      // Pared 4
      glColor3f(0,0,0); glTexCoord2f(0.0f,0.33f); glVertex3i(0,0,0);
      glColor3f(0,0,1); glTexCoord2f(0.25f,0.33f); glVertex3i(0,0,1);
      glColor3f(0,1,1); glTexCoord2f(0.25f,0.66f); glVertex3i(0,1,1);
      glColor3f(0,1,0); glTexCoord2f(0.0f,0.66f); glVertex3i(0,1,0);

      //g=1 normal hacia +g
      // Reja (al final por el test de z)
      glColor3f(0,1,1); glTexCoord2f(0.0f,0.66f); glVertex3i(0,1,1);
      glColor3f(1,1,1); glTexCoord2f(0.25f,0.66f); glVertex3i(1,1,1);
      glColor3f(1,1,0); glTexCoord2f(0.25f,1.0f); glVertex3i(1,1,0);
      glColor3f(0,1,0); glTexCoord2f(0.0f,1.0f); glVertex3i(0,1,0);
    glEnd();
  }
  glDisable(GL_LIGHTING); glDisable(GL_TEXTURE_2D);
  if (wire&&!relleno){ // si esta relleno no tiene sentido esto
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

  // ejes
  glBegin(GL_LINES);
    glColor3d(1,0,0); glVertex3d(1.1,0,0); glVertex3d(1.5,0,0);
    glColor3d(0,1,0); glVertex3d(0,1.1,0); glVertex3d(0,1.5,0);
    glColor3d(0,0,1); glVertex3d(0,0,1.1); glVertex3d(0,0,1.5);
  glEnd();

  glPopMatrix();
  glPopAttrib();
}

