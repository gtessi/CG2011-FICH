#include <iostream> // cout
#include <cmath> // sin cos
#include <GL/glut.h>
#include <GL/gl.h>
using namespace std;

extern int text_w;
extern int text_h;
extern bool relleno;
//==========================================
// globales

extern float aang; // orientacion
extern float ax,ay; // posicion en la pista
extern float aacel; // acelerador (eje y del mouse)
extern float aspeed; // velocidad actual
extern float topspeed; // velocidad maxima
extern float rang; // direccion de las ruedas delanteras respecto al auto (eje x del mouse) 
extern float rang2; // giro de las ruedas sobre su eje, cuando el auto avanza

extern float lpos[]; // posición de la luz
extern int lod; // nivel de detalle para los graficos

static const double PI=4*atan(1.0),R2G=180/PI, DOS_PI=2*PI, G2R=atan(1.0)/45;
//==========================================

//==========================================
// algunos objetos
//==========================================

void drawRueda(int lod=10) {
  int i;
  static int llod=-1;
  static double *cosv=NULL;
  static double *sinv=NULL;
  static double dr;
  if (llod!=lod) {
    llod=lod;
    if (cosv) delete cosv;
    if (sinv) delete sinv;
    cosv=new double[lod+1];
    sinv=new double[lod+1];
    dr=DOS_PI/lod;
    double r=0;
    for (i=0;i<=lod;i++) {
      cosv[i]=cos(r)/2;
      sinv[i]=sin(r)/2;
      r+=dr;
    }
  }
  
  glColor3f(.3,.13,0);
  glBegin(GL_QUADS);
  for (i=0;i<lod;i++) {
    //capa de afuera
    glNormal3f(0,cosv[i],sinv[i]);
    glVertex3f(1,cosv[i]*2,sinv[i]*2);
    glVertex3f(-1,cosv[i]*2,sinv[i]*2);
    glNormal3f(0,cosv[i+1],sinv[i+1]);
    glVertex3f(-1,cosv[i+1]*2,sinv[i+1]*2);
    glVertex3f(1,cosv[i+1]*2,sinv[i+1]*2);
    //capa de adentro
    glNormal3f(0,cosv[i],sinv[i]);
    glVertex3f(1,cosv[i],sinv[i]);
    glVertex3f(-1,cosv[i],sinv[i]);
    glNormal3f(0,cosv[i+1],sinv[i+1]);
    glVertex3f(-1,cosv[i+1],sinv[i+1]);
    glVertex3f(1,cosv[i+1],sinv[i+1]);
    
    // tapa frente
    glNormal3f(-1,0,0);
    glVertex3f(-1,cosv[i]*2,sinv[i]*2);
    glVertex3f(-1,cosv[i+1]*2,sinv[i+1]*2);
    glVertex3f(-1,cosv[i+1],sinv[i+1]);
    glVertex3f(-1,cosv[i],sinv[i]);
    // tapa atras
    glNormal3f(1,0,0);
    glVertex3f(1,cosv[i]*2,sinv[i]*2);
    glVertex3f(1,cosv[i+1]*2,sinv[i+1]*2);
    glVertex3f(1,cosv[i+1],sinv[i+1]);
    glVertex3f(1,cosv[i],sinv[i]);
  }
  
  glColor3f(.5,.5,.5);
  for (i=0;i<lod;i++) {
    // tapa frente
    int i2=(i+lod)%lod;
    glVertex3f(-.7,cosv[i],sinv[i]);
    glVertex3f(-.7,cosv[i+1],sinv[i+1]);
    glVertex3f(-.7,-cosv[i2],-sinv[i2]);
    glVertex3f(-.7,-cosv[i2],-sinv[i]);
    glVertex3f(.7,cosv[i],sinv[i]);
    glVertex3f(.7,cosv[i+1],sinv[i+1]);
    glVertex3f(.7,-cosv[i2],-sinv[i2]);
    glVertex3f(.7,-cosv[i2],-sinv[i]);
  }
  
  glEnd();
}

void drawChasis(bool alt_color=false) {
  if (alt_color)
    glColor3f(.8,.3,.2);
  else
    glColor3f(1,.5,.3);
  glBegin(GL_TRIANGLES);
    glNormal3f(.4,-1,0);
    glVertex3f(-1,-1,-1);
    glVertex3f(-1,-1,1);
    glVertex3f(1,0,0);
    glNormal3f(.4,1,0);
    glVertex3f(-1,1,-1);
    glVertex3f(-1,1,1);
    glVertex3f(1,0,0);
    glNormal3f(.4,0,1);
    glVertex3f(-1,1,1);
    glVertex3f(-1,-1,1);
    glVertex3f(1,0,0);
    glNormal3f(.4,0,-1);
    glVertex3f(-1,1,-1);
    glVertex3f(-1,-1,-1);
    glVertex3f(1,0,0);
  glEnd();
  glBegin(GL_QUADS);
    glNormal3f(-1,0,0);
    glVertex3f(-1,-1,-1);
    glVertex3f(-1,1,-1);
    glVertex3f(-1,1,1);
    glVertex3f(-1,-1,1);
  glEnd();
}

void drawAleron() {
  glColor3f(1,0,0);
  glBegin(GL_TRIANGLES);
  glNormal3f(0,-1,0);
    glVertex3f(-1,-1,-1);
    glVertex3f(-1,-1,1);
    glVertex3f(1,-1,-1);
    glNormal3f(0,1,0);
    glVertex3f(-1,1,-1);
    glVertex3f(-1,1,1);
    glVertex3f(1,1,-1);
  glEnd();
  glBegin(GL_QUADS);
    glNormal3f(0,0,-1);
    glVertex3f(-1,1,-1);
    glVertex3f(-1,-1,-1);
    glVertex3f(1,-1,-1);
    glVertex3f(1,1,-1);
    glNormal3f(0.5,0,1);
    glVertex3f(-1,1,1);
    glVertex3f(-1,-1,1);
    glVertex3f(1,-1,-1);
    glVertex3f(1,1,-1);
    glNormal3f(-1,0,0);
    glVertex3f(-1,1,-1);
    glVertex3f(-1,-1,-1);
    glVertex3f(-1,-1,1);
    glVertex3f(-1,1,1);
  glEnd();
}

void drawCasco(int lod) {
  glutSolidSphere(1,lod,lod);
}

void drawWiredCube() {
  glColor3f(1,1,1);
  glBegin(GL_LINES);
  glVertex3f(-1,-1,-1); glVertex3f(1,-1,-1);
  glVertex3f(-1,-1,-1); glVertex3f(-1,1,-1);
  glVertex3f(-1,-1,-1); glVertex3f(-1,-1,1);
  glVertex3f(1,1,1); glVertex3f(1,1,-1);
  glVertex3f(1,1,1); glVertex3f(1,-1,1);
  glVertex3f(1,1,1); glVertex3f(-1,1,1);
  glVertex3f(-1,-1,1); glVertex3f(1,-1,1);
  glVertex3f(-1,-1,1); glVertex3f(-1,1,1);
  glVertex3f(-1,1,-1); glVertex3f(-1,1,1);
  glVertex3f(-1,1,-1); glVertex3f(1,1,-1);
  glVertex3f(1,-1,-1); glVertex3f(1,1,-1);
  glVertex3f(1,-1,-1); glVertex3f(1,-1,1);
  glEnd();
  
  // ejes
  // @@@@
  glLineWidth(4);
  glBegin(GL_LINES);
  glColor3d(1,0,0); glVertex3d(0,0,0); glVertex3d(1.5,0,0); // x
  glColor3d(0,1,0); glVertex3d(0,0,0); glVertex3d(0,1.5,0); // y
  glColor3d(0,0,1); glVertex3d(0,0,0); glVertex3d(0,0,1.5); // z
  glEnd();
  glLineWidth(2);
  
}

void drawToma() {    
  glColor3f(.8,.3,.2);
  glBegin(GL_QUADS);
    glNormal3f(0,0,-1);
    glVertex3f(-1,-1,-1);
    glVertex3f(-1,1,-1);
    glVertex3f(1,1,-1);
    glVertex3f(.3,-.3,-1);
    glNormal3f(.3,0,1);
    glVertex3f(-1,-1,1);
    glVertex3f(-1,1,1);
    glVertex3f(1,1,.3);
    glVertex3f(.3,-.3,.3);
    glNormal3f(.3,-1,0);
    glVertex3f(-1,-1,-1);
    glVertex3f(-1,-1,1);
    glVertex3f(.3,-.3,.3);
    glVertex3f(.3,-.3,-1);
  glEnd();
}

void drawPista() {
  const int w_2=text_w, h_2=text_h;
  glEnable(GL_TEXTURE_2D);
  glColor3f(0,0,.7);
  glBegin(GL_QUADS);
  glNormal3f(0,0,1);
  glTexCoord2d(0,3); glVertex2f(-w_2,h_2);
  glTexCoord2d(3,3); glVertex2f(w_2,h_2);
  glTexCoord2d(3,0); glVertex2f(w_2,-h_2);
  glTexCoord2d(0,0); glVertex2f(-w_2,-h_2);
  glEnd();
  glDisable(GL_TEXTURE_2D);
}

void drawObjects() {  
  // Para mantener las proporciones, dividir las medidas por 2,
  // ya que el espacio es un cubo de 2x2x2.

  glPushMatrix();    
    gluLookAt(ax-2*cos((aang-rang)*G2R),ay+2*sin((aang-rang)*G2R),0.2,ax,ay,0,0,0,1);
    
    drawPista();
    
    glTranslatef(ax,ay,0.2f);
    glRotatef(-aang,0,0,1);
    glRotatef(2,0,1,0);
    
    drawWiredCube();
    
    // Chasis inferior
    glPushMatrix();
      glScalef(1.0f,0.3f,0.1f);
      drawChasis(false);
    glPopMatrix();
    
    // Ruedas
    // Rueda trasera izquierda
    glPushMatrix();
      glTranslatef(-0.6f,0.4f,0);
      glRotatef(rang2,0,1,0); // rotación sobre su eje
      glRotatef(-90,0,0,1);
      glScalef(0.15f,0.2f,0.2f);    
      drawRueda(lod);
    glPopMatrix();
    
    // Rueda trasera derecha
    glPushMatrix();
      glTranslatef(-0.6f,-0.4f,0);
      glRotatef(rang2,0,1,0); // rotación sobre su eje
      glRotatef(90,0,0,1);
      glScalef(0.15f,0.2f,0.2f);    
      drawRueda(lod);
    glPopMatrix();
    
    // Las ruedas delanteras deben ser 80% de las traseras.
    // (0.3,0.4,0.4) => 0.8*(0.3,0.4,0.4) = (0.24,0.32,0.32)
    
    // Rueda delantera izquierda
    glPushMatrix();
      glTranslatef(0.44f,0.28f,0);
      glRotatef(-rang,0,0,1); // giro de la rueda
      glRotatef(rang2,0,1,0); // rotación sobre su eje
      glRotatef(-90,0,0,1);
      glScalef(0.12f,0.16f,0.16f);    
      drawRueda(lod);
    glPopMatrix();
    
    // Rueda delantera derecha
    glPushMatrix();
      glTranslatef(0.44f,-0.28f,0);
      glRotatef(-rang,0,0,1); // giro de la rueda
      glRotatef(rang2,0,1,0); // rotación sobre su eje      
      glRotatef(90,0,0,1);
      glScalef(0.12f,0.16f,0.16f);    
      drawRueda(lod);
    glPopMatrix();
    
    // Tomas
    // Toma izquierda
    glPushMatrix();
      glTranslatef(0.0f,0.25f,0.01f);
      glScalef(0.35f,-0.25f,0.05f);
      drawToma();
    glPopMatrix();
    
    // Toma derecha
    glPushMatrix();
      glTranslatef(0.0f,-0.25f,0.01f);
      glScalef(0.35f,0.25f,0.05f);
      drawToma();
    glPopMatrix();
    
    // Chasis superior
    glPushMatrix();
      glTranslatef(-0.2f,0.0f,0.05f);
      glRotatef(10,0,1,0);
      glScalef(0.6f,0.15f,0.1f);
      drawChasis(true);
    glPopMatrix();
    
    // Alerones
    // Alerón trasero
    glPushMatrix();
      glTranslatef(-0.75f,0.0f,0.3f);
      glScalef(0.2f,0.5f,0.05f);
      drawAleron();
    glPopMatrix();
    
    // Alerón delantero
    glPushMatrix();
      glTranslatef(0.8f,0.0f,0.0f);
      glScalef(0.1f,0.3f,0.03f);
      drawAleron();
    glPopMatrix();
    
    // Casco
    glPushMatrix();
      glTranslatef(0.1f,0.0f,0.07f);
      glScalef(0.08f,0.08f,0.08f);
      drawCasco(lod);
    glPopMatrix();
  
  glPopMatrix();
  
  // Lo que está fijo en el modelo, se dibuja antes del
  // gluLookAt, luego todo lo que se mueve con la cámara.  
}

