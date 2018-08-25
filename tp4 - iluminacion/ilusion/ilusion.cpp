// Ilusión de Hinton o Lilac chaser

#include <cstdlib> // exit
#include <iostream> // cout
#include <cmath> // fabs sin cos
#include <GL/glut.h>

using namespace std;

//------------------------------------------------------------
// variables globales
int
  w=640,h=480; // tamaño inicial de la ventana

// color de los objetos y luminancia del fondo
float c[3]={.8,.4,.8},a[3]={.4,.53,.4},l=.5;
bool cambia_after=false;


//============================================================
// callbacks

//------------------------------------------------------------

// arma un un nuevo buffer (back) y reemplaza el framebuffer
void Display_cb() {
  // arma el back-buffer
  glClearColor(l,l,l,1);
  glClear(GL_COLOR_BUFFER_BIT);// rellena con color de fondo

  int i,n=16,r=((w<h)? w : h)/4,wc=r/8; double ang=8*atan(1)/n;
  static int salto=0; salto=(salto+1)%n;

  // dibuja

  // centro
  glColor3fv(a);
  glBegin(GL_QUADS);
    glVertex2i(-wc,-wc);
    glVertex2i( wc,-wc);
    glVertex2i( wc, wc);
    glVertex2i(-wc, wc);
  glEnd();

  // circulo
  glColor3fv(c);
  for (i=0;i<n;i++){
    if (i==salto) continue;
    glPushMatrix();
    glTranslated(r*cos(i*ang),r*sin(i*ang),0);
    glBegin(GL_QUADS);
      glVertex2i(-wc,-wc);
      glVertex2i( wc,-wc);
      glVertex2i( wc, wc);
      glVertex2i(-wc, wc);
    glEnd();
    glPopMatrix();
  }

  glutSwapBuffers(); // lo manda al monitor
}

// Si no hace nada ==> hace esto
// El "framerate" real (cuadros por segundo)
// depende de la complejidad del modelo (lod) y la aceleracion por hardware

static const int ms_lista[]={1,2,5,10,20,50,100,200,500,1000,2000,5000},ms_n=12;
static int ms_i=6,msecs=ms_lista[ms_i]; // milisegundos por frame

void Idle_cb() {
  // Cuenta el lapso de tiempo
  static int anterior=glutGet(GLUT_ELAPSED_TIME); // milisegundos desde que arranco
  if (msecs!=1){ // si msecs es 1 no pierdo tiempo
    int tiempo=glutGet(GLUT_ELAPSED_TIME), lapso=tiempo-anterior;
    if (lapso<msecs) return;
    anterior=tiempo;
  }
  Display_cb(); // redibuja
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
  glMatrixMode(GL_MODELVIEW);  glLoadIdentity();
  glTranslatef(w/2,h/2,0);
  Display_cb(); // Redibuja mientras hace el reshape
}


//------------------------------------------------------------
// Teclado

// Maneja pulsaciones del teclado (ASCII keys)
// x,y posicion del mouse cuando se teclea
void Keyboard_cb(unsigned char key,int x=0,int y=0) {
  if (key=='a'||key=='A') {cambia_after=true; return;}
  if (key=='c'||key=='C') {cambia_after=false; return;}

  int i; float sum;
       if (key=='r') {i=0; sum= .1;}
  else if (key=='R') {i=0; sum=-.1;}
  else if (key=='g') {i=1; sum= .1;}
  else if (key=='G') {i=1; sum=-.1;}
  else if (key=='b') {i=2; sum= .1;}
  else if (key=='B') {i=2; sum=-.1;}
  else if (key=='l') {i=3; sum= .1;}
  else if (key=='L') {i=3; sum=-.1;}
  else return;
  if (i!=3){// color
    if (!cambia_after){
      c[i]+=sum; if (c[i]>1) c[i]=0; else if (c[i]<0) c[i]=1;
    }
    else{
      a[i]+=sum; if (a[i]>1) a[i]=0; else if (a[i]<0) a[i]=1;
    }
  }
  else{ // luminancia del fondo
    l+=sum; if (l>1) l=0; else if (l<0) l=1;
  }
  cout <<
    "C=" << c[0] << ',' << c[1] << ',' << c[2] <<
    " L=" << l  <<
    " A=" << a[0] << ',' << a[1] << ',' << a[2] <<
    "            \r"; cout.flush();
  Display_cb();
}

// Special keys (non-ASCII)
// teclas de funcion, flechas, page up/dn, home/end, insert
void Special_cb(int key,int xm=0,int ym=0) {
  if (key==GLUT_KEY_F4 && glutGetModifiers()==GLUT_ACTIVE_ALT) // alt+f4 => exit
    exit(EXIT_SUCCESS);
  if (key==GLUT_KEY_PAGE_UP||key==GLUT_KEY_PAGE_DOWN){ // velocidad
    if (key==GLUT_KEY_PAGE_DOWN) ms_i++;
    else ms_i--;
    if (ms_i<0) ms_i=0; if (ms_i==ms_n) ms_i--;
    msecs=ms_lista[ms_i];
    if (msecs<1000)
      cout << 1000/msecs << "fps" << endl;
    else
      cout << msecs/1000 << "s/frame)" << endl;
  }
}

//------------------------------------------------------------
// Menu
void Menu_cb(int value)
{
  switch (value){
    case 'a':
      Keyboard_cb('a');
      return;
    case 'c':
      Keyboard_cb('c');
      return;
    case 'r':
      Keyboard_cb('r');
      return;
    case 'R':
      Keyboard_cb('R');
      return;
    case 'g':
      Keyboard_cb('g');
      return;
    case 'G':
      Keyboard_cb('G');
      return;
    case 'b':
      Keyboard_cb('b');
      return;
    case 'B':
      Keyboard_cb('B');
      return;
    case 'l':
      Keyboard_cb('l');
      return;
    case 'L':
      Keyboard_cb('L');
      return;
    case (256+GLUT_KEY_PAGE_UP):
      Special_cb(GLUT_KEY_PAGE_UP);
      return;
    case (256+GLUT_KEY_PAGE_DOWN):
      Special_cb(GLUT_KEY_PAGE_DOWN);
      return;
    case 27: //esc
      exit(EXIT_SUCCESS);
  }
}

//------------------------------------------------------------
void inicializa() {
  // GLUT
  glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);// pide color RGB y double buffering
  glutInitWindowSize(w,h); glutInitWindowPosition(50,50);
  glutCreateWindow("Ilusion"); // crea el main window

  //declara los callbacks, los que (aun) no se usan (aun) no se declaran
  glutDisplayFunc(Display_cb);
  glutIdleFunc(Idle_cb);
  glutReshapeFunc(Reshape_cb);
  glutKeyboardFunc(Keyboard_cb);
  glutSpecialFunc(Special_cb);


  // crea el menu
  glutCreateMenu(Menu_cb);
    glutAddMenuEntry("      [a]_Cambia Afterimage      ", 'a');
    glutAddMenuEntry("      [c]_Cambia Color           ", 'c');
    glutAddMenuEntry("      [r]_+Rojo                  ", 'r');
    glutAddMenuEntry("      [R]_-Rojo                  ", 'r');
    glutAddMenuEntry("      [g]_+Verde                 ", 'g');
    glutAddMenuEntry("      [G]_-Verde                 ", 'g');
    glutAddMenuEntry("      [b]_+Azul                  ", 'b');
    glutAddMenuEntry("      [B]_-Azul                  ", 'b');
    glutAddMenuEntry("      [l]_+Luminancia del fondo  ", 'l');
    glutAddMenuEntry("      [L]_-Luminancia del fondo  ", 'l');
    glutAddMenuEntry("[PgUp]_Aumenta Framerate      ", (256+GLUT_KEY_PAGE_UP));
    glutAddMenuEntry("[Pgdn]_Disminuye Framerate    ", (256+GLUT_KEY_PAGE_DOWN));
    glutAddMenuEntry("  [Esc]_Exit                   ", 27);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv); // inicialización interna de GLUT
  inicializa(); // define el estado inicial de GLUT y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0; // nunca se llega acá, es sólo para evitar un warning
}
