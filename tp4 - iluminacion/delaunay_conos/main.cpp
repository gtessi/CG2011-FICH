#include <cmath> // exp
#include <cstdlib> // exit
#include <cstring> // memmove
#include <iostream> // cout
#include <GL/glut.h>

/*
// comentar este bloque si se quiere una ventana de comando
#ifdef _WIN32
 #pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#endif
*/

using namespace std;

//------------------------------------------------------------
// variables globales y defaults
int posicion_cono[100][2],cantidad=0;
float color_cono[100][3];
int cono_editado=-1; // si -1 => no se esta desplazando ningun cono
float radio=1000, radio0=1000;
int xclick,yclick; // x e y cuando clickeo un boton

float // luces y colores en float;
  lambient[]={.4f,.4f,.4f}, // luz ambiente
  lspecular[]={1,1,1}, // luz reflejada
  ldiffuse[]={1,1,1}, // luz difusa
  fondo[]={.9f,.9f,.9f},    // color de fondo
  lpos[]={100,100,100,1};          // posicion luz, l[4]: 0 => direccional -- 1 => posicional

bool zbuffer=true;          // habilita el z-buffer
bool luz=true;              // habilita la iluminacion

short modifiers=0;  // ctrl, alt, shift (de GLUT)
//------------------------------------------------------------
// redibuja los objetos
extern void drawObjects();

// Cada vez que hace un redisplay
void Display_cb() { // Este tiene que estar
  // borra los buffers de pantalla y z
  glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
  drawObjects();
  glutSwapBuffers();
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto del screen
void Reshape_cb(int w, int h){
  if (!(w&&h)) return;

  glViewport(0,0,w,h); // region donde se dibuja
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,w,h,0,1,1003);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0,0,2,0,0,0,0,1,0);
  glLightfv(GL_LIGHT0,GL_POSITION,lpos);
  glutPostRedisplay();
}

//------------------------------------------------------------
// Teclado y Mouse
/*
modificadores que devuelve glutGetModifiers()
GLUT ACTIVE SHIFT //Set if the Shift modifier or Caps Lock is active.
GLUT ACTIVE CTRL //Set if the Ctrl modifier is active.
GLUT ACTIVE ALT //Set if the Alt modifier is active.
*/

// Maneja pulsaciones del teclado (ASCII keys)
// x,y posicion del mouse cuando se teclea
void Keyboard_cb(unsigned char key,int x=0,int y=0) {
  if (key==27){ // escape => exit
    if (!glutGetModifiers())
      exit(EXIT_SUCCESS);
  }
  else if (key==127){ // del => borra
    static const size_t szi=sizeof(int);
    static const size_t szf=sizeof(float);
    if(cono_editado==-1) return;
    memmove(//posicion
      &posicion_cono[cono_editado][0], // destino
      &posicion_cono[cono_editado+1][0],//fuente
      2*(99-cono_editado)*szi);//cantidad
    memmove(//color
      &color_cono[cono_editado][0], // destino
      &color_cono[cono_editado+1][0],//fuente
      3*(99-cono_editado)*szf);//cantidad
    cantidad--;
    cono_editado=-1;
  }
  else if (key=='b'||key=='B'){ // z-buffer
    zbuffer=!zbuffer;
    if (zbuffer) glEnable(GL_DEPTH_TEST); // habilita el z-buffer
    else glDisable(GL_DEPTH_TEST);// deshabilita
    if (zbuffer)
      cout << "Z-Buffer Habilitado" << endl;
    else
      cout << "Z-Buffer Deshabilitado" << endl;
  }
  else if (key=='l'||key=='L'){ // luces
    luz=!luz;
    if (luz) {glEnable(GL_LIGHTING);glEnable(GL_LIGHT0);glEnable(GL_COLOR_MATERIAL);}
    else {glDisable(GL_LIGHTING);glDisable(GL_LIGHT0);glDisable(GL_COLOR_MATERIAL);}
    if (luz)
      cout << "Iluminacion" << endl;
    else
      cout << "Color" << endl;
  }
  else if (key=='p'||key=='P'){ // Posicional
    lpos[3]=1-lpos[3];
    glLightfv(GL_LIGHT0,GL_POSITION,lpos);
    if (lpos[3])
      cout << "Luz Posicional" << endl;
    else
      cout << "Luz Direccional" << endl;
  }
  else if (key=='d'||key=='D'){ // borrar todos los conos
    cantidad=0;
  }

  Display_cb();
}

// Special keys (non-ASCII)
/*
  GLUT KEY F[1,12] F[1,12] function key.
  GLUT KEY LEFT Left directional key.
  GLUT KEY UP Up directional key.
  GLUT KEY RIGHT Right directional key.
  GLUT KEY DOWN Down directional key.
  GLUT KEY PAGE UP Page up directional key.
  GLUT KEY PAGE DOWN Page down directional key.
  GLUT KEY HOME Home directional key.
  GLUT KEY END End directional key.
  GLUT KEY INSERT Inset directional key.
*/
// aca es int key
void Special_cb(int key,int xm=0,int ym=0) {
  if (key==GLUT_KEY_F4){ // alt+f4 => exit
    if (glutGetModifiers()==GLUT_ACTIVE_ALT)
      exit(EXIT_SUCCESS);
  }
}

// Movimientos del mouse
void PassiveMotion_cb(int xm, int ym){ // movimiento
  cout << "\r" << xm << "," << ym << "                ";  cout.flush();
}
void Motion_cb(int xm, int ym){ // drag
  cout << "\r" << xm << "," << ym << "                ";  cout.flush();
  if (modifiers==GLUT_ACTIVE_SHIFT){ // cambio de radio
    radio=radio0*exp((yclick-ym)/100.0);
    Display_cb();
  }
  else if (modifiers==GLUT_ACTIVE_ALT){ // cambio de posicion la luz
    if (!luz) return;
    lpos[0]=xm; lpos[1]=ym;
    glLightfv(GL_LIGHT0,GL_POSITION,lpos);
    Display_cb();
  }
  else{ // traslada un cono
    posicion_cono[cono_editado][0]=xm;
    posicion_cono[cono_editado][1]=ym;
  }
  glutPostRedisplay(); // avisa que hay que redibujar
}

// Clicks del mouse
// GLUT LEFT BUTTON, GLUT MIDDLE BUTTON, or GLUT RIGHT BUTTON
// The state parameter is either GLUT UP or GLUT DOWN
// glutGetModifiers may be called to determine the state of modifier keys
void Mouse_cb(int button, int state, int x, int y){
  xclick=x; yclick=y;
  if (button==GLUT_LEFT_BUTTON){
    if (state==GLUT_DOWN){
      modifiers=(short)glutGetModifiers();
      if (modifiers){
        glutMotionFunc(Motion_cb);
        return;
      }
      cono_editado=-1;
      static int epsilon=10;
      int j,d=epsilon;
      for (j=0;(j<cantidad)&&(d>=epsilon);j++){
        d=abs(x-posicion_cono[j][0])+abs(y-posicion_cono[j][1]);
      }
      if (d>=epsilon){// ninguno cerca => agrega un punto
        // genero un color distinto para cada cono
        posicion_cono[cantidad][0]=x;
        posicion_cono[cantidad][1]=y;
        color_cono[cantidad][0]=float(rand())/RAND_MAX;
        color_cono[cantidad][1]=float(rand())/RAND_MAX;
        color_cono[cantidad][2]=float(rand())/RAND_MAX;
        cantidad++;
        cono_editado=cantidad-1;
      }else{
        cono_editado=j-1;
      }
      glutMotionFunc(Motion_cb); // callback para los drags
      Display_cb();
      return;
    }//end GLUT_DOWN
    else if (state==GLUT_UP){
      glutMotionFunc(0); // anula el callback para los drags
      radio0=radio;
    }
  }//end IF GLUT_LEFT_BUTTON
}

//------------------------------------------------------------
// Menu
void Menu_cb(int value)
{
  switch (value){
    case 'b':
      Keyboard_cb('b');
      return;
    case 'l':
      Keyboard_cb('l');
      return;
    case 'p':
      Keyboard_cb('p');
      return;
    case 'd':
      Keyboard_cb('d');
      return;
    case 27: //esc
      exit(EXIT_SUCCESS);
  }
}

//------------------------------------------------------------
// Inicializa GLUT y OpenGL
void initialize() {
  // pide z-buffer, color RGBA y double buffering
  glutInitDisplayMode(GLUT_DEPTH|GLUT_RGB|GLUT_DOUBLE);

  glutInitWindowSize(400,300); glutInitWindowPosition(350,50);

  glutCreateWindow("Objetos en Glut"); // crea el main window

  //declara los callbacks
  //los que no se usan no se declaran
  glutDisplayFunc(Display_cb); // redisplays
  glutReshapeFunc(Reshape_cb);
  glutKeyboardFunc(Keyboard_cb);
  glutSpecialFunc(Special_cb);
  glutMouseFunc(Mouse_cb);
  glutPassiveMotionFunc(PassiveMotion_cb);

  glutCreateMenu(Menu_cb);
  glutAddMenuEntry("     [b]_Z-Buffer ON/OFF       ", 'b');
  glutAddMenuEntry("     [l]_Luz ON/OFF            ", 'l');
  glutAddMenuEntry("     [p]_Luz Posicional ON/OFF ", 'p');
  glutAddMenuEntry("     [d]_Borrar conos          ", 'd');
  glutAddMenuEntry("   [Esc]_Exit                  ",  27);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  // ========================
  // estado normal del OpenGL
  // ========================

  glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); // habilita el z-buffer
  glEnable(GL_NORMALIZE); // para que el scaling no moleste
  glDisable(GL_AUTO_NORMAL); // para nurbs??
  glEnable (GL_POLYGON_OFFSET_FILL); glPolygonOffset (1., 1.); // coplanaridad ****

  // interpola normales por nodos o una normal por plano
  glShadeModel(GL_SMOOTH); //GL_FLAT

  // color de fondo
  glClearColor(fondo[0],fondo[1],fondo[2],1);

  // direccion de los poligonos
  glFrontFace(GL_CW); // la cara delantera esta definida en sentido horario
  glPolygonMode(GL_FRONT,GL_FILL); // las caras de adelante se dibujan rellenas
  glCullFace(GL_BACK);glEnable(GL_CULL_FACE);// se descartan las caras de atras

  // define la luz
  glLightfv(GL_LIGHT0,GL_AMBIENT,lambient);
  glLightfv(GL_LIGHT0,GL_DIFFUSE,ldiffuse);
  glLightfv(GL_LIGHT0,GL_SPECULAR,lspecular);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);// las caras traseras de los polígonos no se iluminarán  
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,(lpos[3])?GL_TRUE:GL_FALSE);//Local or Infinite Viewpoint

  // define material estandar para los conos
  // el color ambiente y difuso se cambian con glColor
//  glMaterialfv(GL_FRONT,GL_SPECULAR,lspecular);// el color reflejado es cte para todos
//  glMaterialf(GL_FRONT,GL_SHININESS,1);// el brillo es cte para todos
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

  if (luz) {glEnable(GL_LIGHTING);glEnable(GL_LIGHT0);glEnable(GL_COLOR_MATERIAL);}
  else {glDisable(GL_LIGHTING);glDisable(GL_LIGHT0);glDisable(GL_COLOR_MATERIAL);}

  glutPostRedisplay(); // avisa que hay que redibujar
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv);// inicializa glut
  initialize(); // condiciones iniciales de la ventana y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0;
}
