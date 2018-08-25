#include <iostream> // cin, cout
#include <cstdlib> // exit
#include <ctime> // time
#include <cmath> // fabs
#include <GL/glut.h>

#include "utiles.h"
#include "p2e.h"
#include "delaunay.h"

using namespace std;

// comentar este bloque si se quiere una ventana de comando
#ifdef _WIN32
 #pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#endif

// variables
static int
  linewidth=1,pointsize=5,
  tol=5; // tolerancia para seleccionar un punto

static float // luces y colores en float;
  fondo[]={0.95f,0.98f,1.0},       // color de fondo
  delaunay_color[]={.2f,.2f,.6f},
  voronoi_color[]={.6f,.2f,.2f},
  circle_color[]={.2f,.6f,.6f},
  vertex_color[]={.1f,.1f,.3f},
  center_color[]={.3f,.1f,.1f};

static bool
  mini=false,      // si esta minimizado
  cl_info=true;     // informa por la linea de comandos;

static const bool borra=true; // para no poner dibuja(true) sino dibuja(borra)

static short modifiers=0;  // ctrl, alt, shift (de GLUT)

// delaunay
static delaunay D; // una instancia de la clase
static pila_ptr<p2e> puntos; // listas de puntos
static p2e *pt=0; // el punto que se agrega mueve o borra
static p2e ll,ur; static bool haybb=false; // bounding box
static bool dibujar_c=false, dibujar_t=false, dibujar_v=false;

// funciones
static void check_gl_error(){
//#ifdef _DEBUG
  // chequea errores
  int errornum=glGetError();
  while(errornum!=GL_NO_ERROR){
    if (cl_info){
           if(errornum==GL_INVALID_ENUM)
             cout << "GL_INVALID_ENUM" << endl;
      else if(errornum==GL_INVALID_VALUE)
             cout << "GL_INVALID_VALUE" << endl;
      else if (errornum==GL_INVALID_OPERATION)
             cout << "GL_INVALID_OPERATION" << endl;
      else if (errornum==GL_STACK_OVERFLOW)
             cout << "GL_STACK_OVERFLOW" << endl;
      else if (errornum==GL_STACK_UNDERFLOW)
             cout << "GL_STACK_UNDERFLOW" << endl;
      else if (errornum==GL_OUT_OF_MEMORY)
             cout << "GL_OUT_OF_MEMORY" << endl;
    }
    errornum=glGetError();
  }
//#endif // _DEBUG
}

static void salir(){
  exit(EXIT_SUCCESS);
}

static void circulo_Bresenham(int xc, int yc, int r) {
  glPointSize(linewidth); // si, la circunferencia es una linea
  glBegin(GL_POINTS);
  int x=0, y=r, h=1-r, E=3, SE=5-(r<<1);
  while (y>x){
    glVertex2i( x+xc, y+yc);
    glVertex2i( x+xc,-y+yc);
    glVertex2i(-x+xc, y+yc);
    glVertex2i(-x+xc,-y+yc);

    glVertex2i( y+xc, x+yc);
    glVertex2i( y+xc,-x+yc);
    glVertex2i(-y+xc, x+yc);
    glVertex2i(-y+xc,-x+yc);
    if (h>=0){ // SE
      h+=SE; E+=2; SE+=4;
      y--;
    }
    else { // E
      h+=E; E+=2; SE+=2;
    }
    x++;
  }
  // x==y => no hace falta intercambiar
  glVertex2i( x+xc, y+yc);
  glVertex2i( x+xc,-y+yc);
  glVertex2i(-x+xc, y+yc);
  glVertex2i(-x+xc,-y+yc);
  glEnd();
  glPointSize(pointsize);
}

//--------
static void dibuja(){
  if (!puntos) return;
  int i,j,r,rv,ntris=D.deep, npoints=puntos.deep;
  p2e c,cv;

  // triangulos circulos y voronoi
  for(j=0;j<ntris;j++){
    Dtri &t=D[j]; if (dibujar_c||dibujar_v) t.circulo(c,r);
    // circunferencias
    if (dibujar_c) {
      glColor3fv(circle_color);
      circulo_Bresenham(c[0],c[1],r);
    }
    if (dibujar_v){
      glColor3fv(voronoi_color);
      // dibuja una linea entre el centro de este triangulo y el de cada vecino
      // para hacerlo solo una vez (no a-b y b-a) va del de mayor puntero al de menor puntero
      // cuando es vecino de la frontera dibuja una linea (larga) perpendicular a la frontera
      Dtri *v;
      i=2; do{
        v=t.vecino[i];
        if(v>&t) continue; // compara punteros para hacer una sola linea por par
        glBegin(GL_LINES);
        glVertex2iv(c);
        if (!v) cv=c+((t[(i+1)%3]-t[(i+2)%3]).giro90())*2; // frontera
        else v->circulo(cv,rv);// hay vecino
        glVertex2iv(cv);
        glEnd();
      }while(i--);
    }
    // triangulos
    if (dibujar_t) {
      glColor3fv(delaunay_color);
      glBegin(GL_LINE_STRIP);
      glVertex2iv(t[0]);
      glVertex2iv(t[1]);
      glVertex2iv(t[2]);
      glVertex2iv(t[0]);
      glEnd();
    }
  }
  // puntos
  glPointSize(pointsize);
  glColor3fv(vertex_color);
  glBegin(GL_POINTS);
  for(j=0;j<npoints;j++) glVertex2iv(puntos[j]);
  glEnd();
}

//============================================================
// callbacks

//------------------------------------------------------------
// redibuja los objetos
// Cada vez que hace un redisplay
void Display_cb() { // Este tiene que estar
  if (mini) return;
  // borra el buffer de pantalla
  glClear(GL_COLOR_BUFFER_BIT);
  dibuja();
  glutSwapBuffers();
  check_gl_error();
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto del screen
void Reshape_cb(int w, int h){
  if (cl_info) cout << "reshape " << w << "x" << h << endl;

  if (!h||!w) {mini=true; return;}
  mini=false;

  glViewport(0,0,w,h); // region donde se dibuja

  // matriz de proyeccion
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();
  glOrtho(0,w,h,0,-1,1);

  glutPostRedisplay();
}


//------------------------------------------------------------
// Teclado y Mouse
// GLUT ACTIVE SHIFT Set if the Shift modifier or Caps Lock is active.
// GLUT ACTIVE CTRL  Set if the Ctrl modifier is active.
// GLUT ACTIVE ALT   Set if the Alt modifier is active.

static inline short get_modifiers() {return modifiers=(short)glutGetModifiers();}

// Maneja pulsaciones del teclado (ASCII keys)
// x,y posicion del mouse cuando se teclea
void Keyboard_cb(unsigned char key,int x=0,int y=0) {
  if (key==27){ // escape => exit
    get_modifiers();
    if (!modifiers) salir();
  }
  else if (key=='i'||key=='I'){ // info
    cl_info=!cl_info;
    cout << ((cl_info)? "Info" : "Sin Info") << endl;
    return;
  }
  else if (key==127){ // del => borra pt
    if (!pt||puntos.deep<=4) return;
    D.quitapunto(*pt);
    puntos.remove(pt);
    delete pt; pt=0;
	glutPostRedisplay();
    return;
  }
  else if (key=='t'||key=='T'){ // Triangulos
    dibujar_t=!dibujar_t;
    if (dibujar_t) {
      glutChangeToMenuEntry(1, "[t] No Dibujar Triangulos", 't');
      if (cl_info) cout << "Dibuja Triangulos" << endl;
    }
    else {
      glutChangeToMenuEntry(1, "[t] Dibujar Triangulos", 't');
      if (cl_info) cout << "No Dibuja Triangulos" << endl;
    }
	glutPostRedisplay();
    return;
  }
  else if (key=='c'||key=='C'){ // Circunferencias
    dibujar_c=!dibujar_c;
    if (dibujar_c) {
      glutChangeToMenuEntry(2, "[c] No Dibujar Circunferencias", 'c');
      if (cl_info) cout << "Dibuja Circunferencias" << endl;
    }
    else {
      glutChangeToMenuEntry(2, "[c] Dibujar Circunferencias", 'c');
      if (cl_info) cout << "No Dibuja Circunferencias" << endl;
    }
	glutPostRedisplay();
    return;
  }
  else if (key=='v'||key=='V'){ // Voronoi
    dibujar_v=!dibujar_v;
    if (dibujar_v) {
      glutChangeToMenuEntry(3, "[v] No Dibujar Voronoi", 'v');
      if (cl_info) cout << "Dibuja Voronoi" << endl;
    }
    else {
      glutChangeToMenuEntry(3, "[v] Dibujar Voronoi", 'v');
      if (cl_info) cout << "No Dibuja Voronoi" << endl;
    }
	glutPostRedisplay();
    return;
  }
  else if (key=='b'||key=='B'){ // Borra Todo
    while (puntos.deep>0) delete puntos.pop();
    haybb=false;
	glutPostRedisplay();
    return;
  }
}

// Special keys (non-ASCII)
//  GLUT KEY F[1,12] F[1,12] function key.
//  GLUT KEY LEFT Left directional key.
//  GLUT KEY UP Up directional key.
//  GLUT KEY RIGHT Right directional key.
//  GLUT KEY DOWN Down directional key.
//  GLUT KEY PAGE UP Page up directional key.
//  GLUT KEY PAGE DOWN Page down directional key.
//  GLUT KEY HOME Home directional key.
//  GLUT KEY END End directional key.
//  GLUT KEY INSERT Inset directional key.

// aca es int key
void Special_cb(int key,int xm=0,int ym=0) {
  if (key==GLUT_KEY_F4){ // alt+f4 => exit
    get_modifiers();
    if (modifiers==GLUT_ACTIVE_ALT) salir();
  }
}

// Movimientos del mouse
void Motion_cb(int x, int y){ // drag
  if (!pt) return;
  p2e pm(x,y);
  if (D.muevepunto(*pt,pm)) glutPostRedisplay();
  else pt=0;
}

void passive_motion_cb(int x, int y){ // movimiento
  int x0=ll[0], y0=ll[1], x1=ur[0], y1=ur[1];
  glColor3fv(fondo); // borra el viejo rectangulo
  glLineWidth(3.0);
  glBegin(GL_LINE_LOOP);
    glVertex2i(x0,y0);
    glVertex2i(x1,y0);
    glVertex2i(x1,y1);
    glVertex2i(x0,y1);
  glEnd();
  x1=ur[0]=x; y1=ur[1]=y;
  glLineWidth(1.0);
  glColor3fv(delaunay_color); // dibuja el nuevo;
  glEnable(GL_LINE_STIPPLE);
  glLineStipple(1,0x7070);
  glBegin(GL_LINE_LOOP);
    glVertex2i(x0,y0);
    glVertex2i(x1,y0);
    glVertex2i(x1,y1);
    glVertex2i(x0,y1);
  glEnd();
  glDisable(GL_LINE_STIPPLE);
  glLineWidth(linewidth);
  glutSwapBuffers();
}

// Clicks del mouse
// GLUT LEFT BUTTON, GLUT MIDDLE BUTTON, or GLUT RIGHT BUTTON
// The state parameter is either GLUT UP or GLUT DOWN
// glutGetModifiers may be called to determine the state of modifier keys
void Mouse_cb(int button, int state, int x, int y){
  static bool pendiente=false;
  if (button==GLUT_LEFT_BUTTON){
    if (state==GLUT_DOWN) {
      if (!haybb) {// define el bounding box
        if (!pendiente){// primer punto
          ll[0]=x; ll[1]=y; ur=ll;
          glClear(GL_COLOR_BUFFER_BIT);
          glutPassiveMotionFunc(passive_motion_cb);
          glutMotionFunc(passive_motion_cb);
          pendiente=true;
        }
        return;
      }
      // selecciona para mover o borrar
      p2e pm(x,y); pt=0;
      // verifica tolerancia
      int len=puntos.deep;
      while(puntos.deep>4) {
        pt=(p2e*)puntos.pop();
        if (pm.distanciac(*pt)<tol) break;
        pt=0;
      }puntos.deep=len;
      if (!pt) {// agrega // agrega un punto
        pt=new p2e(pm); puntos.push(pt);
        if (D.agregapunto(*pt)) glutPostRedisplay();
        else {puntos.deep--; delete pt; pt=0;}
      }
      // pt es el punto a editar
      glutMotionFunc(Motion_cb); // mueve
      return;
    } // down
    else if (state==GLUT_UP){
      if (!haybb) {// define el bounding box
        if (pendiente){// segundo punto
          ur[0]=x;ur[1]=y;
          if (abs(ll[0]-ur[0])<2*tol||
              abs(ll[1]-ur[1])<2*tol) return; // menos de tol px no
          glutPassiveMotionFunc(0);// anula el callback
          glutMotionFunc(Motion_cb);
          haybb=true; pendiente=false;
          // ordena
          if (ll[0]>ur[0]) intercambia(ll[0],ur[0]);
          if (ll[1]>ur[1]) intercambia(ll[1],ur[1]);
          //inicializa delaunay
          p2e *b0=new p2e(ll),*b1=new p2e(ur,ll),
              *b2=new p2e(ur),*b3=new p2e(ll,ur);
          puntos.push(b0);puntos.push(b1);puntos.push(b2);puntos.push(b3);
          D.init(*b0,*b1,*b2,*b3);
		  glutPostRedisplay();
          return;
        }
      }
      else { // fin del drag
        glutMotionFunc(0); // anula el callback para los drags
        return;
      }
    } // up
  } // left
}

//------------------------------------------------------------
// Menu
void Menu_cb(int value)
{
  switch (value){
    case 'i':  // info
      Keyboard_cb('i');
      return;
    case 'e': // edita
      Keyboard_cb('e');
      return;
    case 'b': // borra
      Keyboard_cb('b');
      return;
    case 't':  // triangulos
      Keyboard_cb('t');
      return;
    case 'c':  // circunferencias
      Keyboard_cb('c');
      return;
    case 'v':  // voronoi
      Keyboard_cb('v');
      return;
    case 127: // del = borra
      Keyboard_cb(127);
      return;
    case 27: // esc = exit
      salir();
  }
}

int integerv(GLenum pname){
  int value;
  glGetIntegerv(pname,&value);
  return value;
}
#define _PRINT_INT_VALUE(pname) #pname << ": " << integerv(pname) <<endl

//------------------------------------------------------------
// Inicializa GLUT y OpenGL
void initialize() {
  // pide color RGB y double buffering
  glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);

  glutInitWindowSize(640,480); glutInitWindowPosition(50,50);

  glutCreateWindow("Delaunay"); // crea el main window

  //declara los callbacks
  //los que no se usan no se declaran
  glutDisplayFunc(Display_cb); // redisplays
  glutReshapeFunc(Reshape_cb); // resize
  glutKeyboardFunc(Keyboard_cb); // teclado
  glutSpecialFunc(Special_cb); // ctrl alt y shift
  glutMouseFunc(Mouse_cb); // clicks

  glutCreateMenu(Menu_cb);
    if (dibujar_t)
      glutAddMenuEntry("[t] No Dibujar Triangulos", 't');
    else
      glutAddMenuEntry("[t] Dibujar Triangulos", 't');
    if (dibujar_c)
      glutAddMenuEntry("[c] No Dibujar Circunferencias", 'c');
    else
      glutAddMenuEntry("[c] Dibujar Circunferencias", 'c');
    if (dibujar_v)
      glutAddMenuEntry("[v] No Dibujar Voronoi", 'v');
    else
      glutAddMenuEntry("[v] Dibujar Voronoi", 'v');
      glutAddMenuEntry("---------------------------", 255);
      glutAddMenuEntry("[Del] Borra", 127);
      glutAddMenuEntry("[b] Borra Todo", 'b');
      glutAddMenuEntry("---------------------------", 255);
      glutAddMenuEntry("[i] Info ON/OFF", 'i');
      glutAddMenuEntry("[Esc] Exit", 27);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  // ========================
  // estado normal del OpenGL
  // ========================
  glClearColor(fondo[0],fondo[1],fondo[2],1);  // color de fondo
  glDrawBuffer(GL_BACK);
  glMatrixMode(GL_MODELVIEW); glLoadIdentity(); // constante
  glShadeModel(GL_FLAT); // no interpola color
  glLineWidth(linewidth);

  // ========================
  // info
  if (cl_info)
    cout << "Vendor:         " << glGetString(GL_VENDOR) << endl
         << "Renderer:       " << glGetString(GL_RENDERER) << endl
         << "GL_Version:     " << glGetString(GL_VERSION) << endl
         << "GL_Extensions:  " << glGetString(GL_EXTENSIONS) << endl
         << "GLU_Version:    " << gluGetString(GLU_VERSION) << endl
         << "GLU_Extensions: " << gluGetString(GLU_EXTENSIONS) << endl
         << _PRINT_INT_VALUE(GL_DOUBLEBUFFER)
         << _PRINT_INT_VALUE(GL_STEREO)
         << _PRINT_INT_VALUE(GL_AUX_BUFFERS)
         << _PRINT_INT_VALUE(GL_RED_BITS)
         << _PRINT_INT_VALUE(GL_GREEN_BITS)
         << _PRINT_INT_VALUE(GL_BLUE_BITS)
         << _PRINT_INT_VALUE(GL_ALPHA_BITS)
         << _PRINT_INT_VALUE(GL_DEPTH_BITS)
         << _PRINT_INT_VALUE(GL_STENCIL_BITS)
         << _PRINT_INT_VALUE(GL_ACCUM_RED_BITS)
         << _PRINT_INT_VALUE(GL_ACCUM_GREEN_BITS)
         << _PRINT_INT_VALUE(GL_ACCUM_BLUE_BITS)
         << _PRINT_INT_VALUE(GL_ACCUM_ALPHA_BITS)
         ;
  // ========================
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv);// inicializa glut
  initialize(); // condiciones iniciales de la ventana y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0; //solo para que el builder se quede tranquilo (aqui nunca llega)
}
