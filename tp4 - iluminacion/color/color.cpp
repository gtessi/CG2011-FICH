// Triángulo de colores y fondo variable

#include <iostream> // cout
#include <cstdlib> // exit
#include <cmath> // fabs
#include <GL/glut.h>

#include <cstdio>
using namespace std;

//------------------------------------------------------------
// variables globales
int
  w=480,h=360; // tamaño inicial de la ventana

float c[3]={.5,.5,.5},l=1,r;

static const float G2R=atan(1)/45, SQR3=sqrt(3.0);

inline float q(const float f){return f*f;}

double wp=1;
//============================================================
// callbacks

//------------------------------------------------------------

// arma un un nuevo buffer (back) y reemplaza el framebuffer
void Display_cb() {
  //static int counter=0; cout << "display: " << counter++ << endl;

  // arma el back-buffer
  glClearColor(c[0],c[1],c[2],1.f); // color de fondo
  glClear(GL_COLOR_BUFFER_BIT);// rellena con color de fondo

  // dibuja
  glBegin(GL_TRIANGLES);
    glColor3f(0,l,0);
    glVertex4d(0,r,0,wp);
    glColor3f(l,0,0);
    glVertex2d(r*SQR3/2,-r/2);
    glColor3f(0,0,l);
    glVertex2d(-r*SQR3/2,-r/2);
  glEnd();

  glutSwapBuffers(); // lo manda al monitor
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto de la ventana
void Reshape_cb(int width, int height){
//  cout << "reshape " << width << "x" << height << endl;
  if (!width||!height) return; // minimizado ==> nada
  w=width; h=height;
  glViewport(0,0,w,h); // región donde se dibuja (toda la ventana)
  // rehace la matriz de proyección (la porcion de espacio visible)
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();
  glOrtho(0,w,0,h,-1,1);
  glMatrixMode(GL_MODELVIEW); glLoadIdentity();
  glTranslatef(w/2.0,h/2.0,0);
  r=((w<h)? w :h)/2.5;
  Display_cb(); // Redibuja mientras hace el reshape
}

//------------------------------------------------------------
// Mouse

// Drag (movimiento con algun boton apretado)
void PasssiveMotion_cb(int x, int y){


  // interpolando en el triangulo (o extrapolado fuera, pero se clampea)
  x=x-w/2; y=h/2-y;
  float X=x/r,Y=y/r;
  c[1]=l*(1+2*Y)/3; //g
  c[0]=l*(1+SQR3*X-Y)/3; //r
  c[2]=l*(1-SQR3*X-Y)/3; //b

  // leyendo el pixel debajo
//  glReadPixels(x,h-y,1,1,GL_RGB,GL_FLOAT,c);

  printf("x=%i,y=%i - R=%.3f,G=%.3f,B=%.3f,(L=%.3f)          \r",x,y,c[0],c[1],c[2],l);
  cout.flush();

  Display_cb(); // Redibuja mientras va moviendo
}

//------------------------------------------------------------
// Teclado

// Maneja pulsaciones del teclado (ASCII keys)
// x,y posicion del mouse cuando se teclea (aqui no importan)
void Keyboard_cb(unsigned char key,int x,int y) {
  float lant=l;
  switch (key){
    case '+': case 'L': // aumenta l
      wp+=.025;// if (l>1) l=0;
      break;
    case '-': case 'l': // disminuye l
      wp-=.025;// if (l<0) l=1;
      break;
    default:
      return; // porque hay que laburar mas
  }
  // dos pasos para arreglar el fondo
  Display_cb(); // Redibuja el triangulo pero el fondo mal
  PasssiveMotion_cb(x,y); // Recalcula y redibuja el fondo
}

// Special keys (non-ASCII)
// teclas de funcion, flechas, page up/dn, home/end, insert
void Special_cb(int key,int xm=0,int ym=0) {
  if (key==GLUT_KEY_F4 && glutGetModifiers()==GLUT_ACTIVE_ALT) // alt+f4 => exit
    exit(EXIT_SUCCESS);
}

//------------------------------------------------------------
void inicializa() {
  // GLUT
  glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);// pide color RGB y double buffering
  glutInitWindowSize(w,h); glutInitWindowPosition(50,50);
  glutCreateWindow("Interpolacion del Color"); // crea el main window

  //declara los callbacks, los que (aun) no se usan (aun) no se declaran
  glutDisplayFunc(Display_cb);
  glutReshapeFunc(Reshape_cb);
  glutKeyboardFunc(Keyboard_cb);
  glutSpecialFunc(Special_cb);
  glutPassiveMotionFunc(PasssiveMotion_cb);
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv); // inicialización interna de GLUT
  inicializa(); // define el estado inicial de GLUT y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0; // nunca se llega acá, es sólo para evitar un warning
}
