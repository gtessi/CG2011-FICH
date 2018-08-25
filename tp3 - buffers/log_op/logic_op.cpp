// Programa Operaciones Logicas

#include <iostream> // cout
#include <cstdlib> // exit
#include <cmath> // fabs
#include <GL/glut.h>

using namespace std;

//------------------------------------------------------------
// variables globales
int
  w=480,h=360, // tamaño inicial de la ventana
  npuntos=3, // cantidad de puntos
  pt[6]={120,100,360,100,240,260}, // los puntos (hasta 3: x0,y0,x1,y1,x2,y2)
  xini,yini,xfin,yfin;

bool draw_changed=true; // esto es por la diferencia glut/freeglut

//============================================================
// callbacks

//------------------------------------------------------------

// arma un un nuevo buffer (back) y reemplaza el framebuffer
void Display_cb() {
  if (!draw_changed) return; draw_changed=false;
  int i;
  static int count=0;
  cout << "display: " << ++count <<endl;
  
  // arma el color-buffer
  glClear(GL_COLOR_BUFFER_BIT);// rellena con color de fondo

  // dibuja el triángulo
  glColor3ub(0,127,127);
  glBegin(GL_TRIANGLES);
    for(i=0;i<npuntos;i++) glVertex2i(pt[2*i],pt[2*i+1]);
  glEnd();
  
  // puntos (después del triángulo, para que se vean encima)
  glColor3ub(0,0,255);
  glPointSize(5.0); // punto grueso
  glBegin(GL_POINTS);
    for(i=0;i<npuntos;i++) glVertex2i(pt[2*i],pt[2*i+1]);
  glEnd();
  
  //bool habilitado=glIsEnabled(GL_COLOR_LOGIC_OP);
  glFlush(); // lo manda al monitor
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto de la ventana
void Reshape_cb(int width, int height){
  // cout << "reshape " << width << "x" << height << endl;
  if (!width||!height) return; // minimizado ==> nada
  w=width; h=height;
  glViewport(0,0,w,h); // región donde se dibuja (toda la ventana)
  // rehace la matriz de proyección (la porcion de espacio visible)
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();
  glOrtho(0,w,0,h,-1,1); // unidades = pixeles
  draw_changed=true; Display_cb(); // Redibuja mientras hace el reshape
}

//------------------------------------------------------------
// Mouse
static void draw_rect(){
  glBegin(GL_LINE_LOOP);
    glVertex2i(xini,yini); glVertex2i(xfin,yini);
    glVertex2i(xfin,yini); glVertex2i(xfin,yfin);
    glVertex2i(xfin,yfin); glVertex2i(xini,yfin);
  glEnd();
  glFlush();
}

// Drag (movimiento con algun boton apretado)
void Motion_cb(int x, int y){  
  draw_rect(); // Dibuja la linea vieja (borra)
  xfin=x; yfin=h-y;
  draw_rect(); // Dibuja la linea nueva
}

// Botones picados o soltados
void Mouse_cb(int button, int state, int x, int y){
  if(button==GLUT_LEFT_BUTTON){ // boton izquierdo
    if(state==GLUT_DOWN){ // clickeado
   	  glEnable(GL_LINE_STIPPLE); // lineas de trazos
      glLogicOp(GL_INVERT); // invierte el color      
//      glColor3ub(127,127,127); glLogicOp(GL_XOR); // xorea el color
      glEnable(GL_COLOR_LOGIC_OP); // habilita las operaciones logicas en RGB
      glutMotionFunc(Motion_cb); // define el callback para los drags
      xini=xfin=x;
      yini=yfin=h-y;
    } // fin clickeado
    else if (state==GLUT_UP){ // soltado
      draw_rect(); // Dibuja la linea vieja (borra)
      glDisable(GL_COLOR_LOGIC_OP);
      glDisable(GL_LINE_STIPPLE);
      glutMotionFunc(0); // anula el callback para los drags
    } // fin soltado
  } // fin botón izquierdo
}

//------------------------------------------------------------
// Teclado

// Maneja pulsaciones del teclado (ASCII keys)
// x,y posicion del mouse cuando se teclea (aqui no importan)
void Keyboard_cb(unsigned char key,int x,int y) {
  if (key==27){
    exit(EXIT_SUCCESS);
  }
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
  // (con double buffer reemplazar glFlush por glutSwapBuffers, pero no tiene mucho sentido)
  glutInitDisplayMode(GLUT_RGB|GLUT_SINGLE);// pide color RGB y single buffering

  glutInitWindowSize(w,h); glutInitWindowPosition(50,50);
  glutCreateWindow("Operaciones lógicas"); // crea el main window

  //declara los callbacks, los que (aun) no se usan (aun) no se declaran
  glutDisplayFunc(Display_cb);
  glutReshapeFunc(Reshape_cb);
  glutKeyboardFunc(Keyboard_cb);
  glutSpecialFunc(Special_cb);
  glutMouseFunc(Mouse_cb);
  //glutMotionFunc(Motion_cb);

  // OpenGL
  //glClearColor(0.85f,0.9f,0.95f,1.f); // color de fondo
  glClearColor(0.9f,0.9f,0.9f,1.f); // color de fondo

  // patrón de lineas de guiones
  GLint factor=1;GLushort patron=0xFF00;//0x5555 0xFF00 0x3333 0x7733
	glLineStipple(factor,patron);
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv); // inicialización interna de GLUT
  inicializa(); // define el estado inicial de GLUT y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0; // nunca se llega acá, es sólo para evitar un warning
}
