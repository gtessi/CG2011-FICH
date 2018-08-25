// Test de Stencil
// Analizar:
//   El funcionamiento del stencil
//   El funcionamiento de glClear()
//   El movimiento de la cerradura
//   El zoom
//   El Passive Motion

#include <cmath> // sqrt
#include <cstdlib> // exit
#include <iostream> // cout
#include <GL/glut.h>

using namespace std;

//------------------------------------------------------------
// variables globales y defaults

int
  w=480,h=360, // tamaño de la ventana
  lod=8, // nivel de detalle (subdivisiones de lineas y superficies parametricas)
  boton=-1, // boton del mouse clickeado
  xclick,yclick; // x e y cuando clickeo un boton
float // luces y colores en float
  lambient[]={.5f,.5f,.5f,1}, // luz ambiente
  ldiffuse[]={.7f,.7f,.7f,1}, // luz difusa
  fondo[]={.95f,.95f,.97f,1.f},   // color de fondo
  face_color[]={.6f,.5f,.2f,.5f}, // color de caras
  line_color[]={.3f,.2f,.1f,.5f}, // color de lineas
  escala=100,escala0, // escala de los objetos window/modelo pixeles/unidad
  eye[]={0,0,5}, target[]={0,0,0}, up[]={0,1,0}, // camara, mirando hacia y vertical
  znear=2, zfar=8, //clipping planes cercano y alejado de la camara (en 5 => veo de 3 a -3)
  lpos[]={2,1,5,1}, // posicion luz, l[4]: 0 => direccional -- 1 => posicional
  xk=0,yk=0; // posicion del ojo de cerradura

bool // variables de estado de este programa
  luz_camara=true,  // luz fija a la camara o al espacio
  perspectiva=true, // perspectiva u ortogonal
  rota=true,        // gira continuamente los objetos respecto de y
  dibuja=true,      // false si esta minimizado
  smooth=true,      // normales por nodo o por plano
  wire=false,       // dibuja lineas o no
  relleno=true,     // dibuja relleno o no
  cl_info=true,     // informa por la linea de comandos
  antialias=false,  // antialiasing
  color=false;      // color/material

short modifiers=0;  // ctrl, alt, shift (de GLUT)

// temporizador:
static const int ms_lista[]={1,2,5,10,20,50,100,200,500,1000,2000,5000},ms_n=12;
static int ms_i=4,msecs=ms_lista[ms_i]; // milisegundos por frame


////////////////////////////////////////////////////////////////////////////////////////
// dibujo

void drawObjects(){

  glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

  // fondo negro donde el stencil tiene un 0
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glStencilFunc (GL_EQUAL,0,~0);
  glDepthMask(false); //no escribe en el z buffer
  glDisable(GL_CULL_FACE); glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glDisable(GL_LIGHTING); // para que no calcule iluminación
  glColor3i(0,0,0);
  float xr=float(w)/2/escala,yr=float(h)/2/escala;
  glBegin(GL_QUADS);
    glVertex2f(-xr,-yr);
    glVertex2f( xr,-yr);
    glVertex2f( xr, yr);
    glVertex2f(-xr, yr);
  glEnd();
  glPopAttrib();

  // objetos donde el stencil tiene 1
  glStencilFunc (GL_EQUAL,1,~0);
  static int a=0;
  if (rota) a=(a+2)%360;

  // toro
  glPushMatrix();
  glRotatef (a, 1.0, 0.0, 0.0);
  glFrontFace(GL_CCW);
  if (wire){
    if (!color) glDisable(GL_LIGHTING); // lineas sin material (siempre en color)
    glColor4fv(line_color); // color de lineas
    glutWireTorus(0.25, .75, 2*lod, 4*lod);
    if (!color) glEnable(GL_LIGHTING); // vuelve a habilitar material
  }
  if (relleno){
    glEnable(GL_CULL_FACE);
    glColor4fv(face_color); // color de caras
    glutSolidTorus(0.25, .75, 2*lod, 4*lod);
    glDisable(GL_CULL_FACE);
  }
  glPopMatrix();

  // tetera
  glPushMatrix();
  glRotatef (a, 0.0, 1.0, 0.0);
  glFrontFace(GL_CW);
  if (wire){
    if (!color) glDisable(GL_LIGHTING); // lineas sin material (siempre en color)
    glColor4fv(line_color); // color de lineas
    glutWireTeapot(.25);
    if (!color) glEnable(GL_LIGHTING); // vuelve a habilitar material
  }
  if (relleno){
    glEnable(GL_CULL_FACE);
    glColor4fv(face_color); // color de caras
    glutSolidTeapot(.25);
    glDisable(GL_CULL_FACE);
  }
  glPopMatrix();

}

void drawKey(){
  if (!dibuja) return; // minimizado

  // dibuja el agujero en el stencil
  glPushAttrib(GL_ALL_ATTRIB_BITS);
    glColorMask(false,false,false,false); // no dibuja en el color buffer
    glDisable(GL_DEPTH_TEST); // no actualiza el depth
    glClear(GL_STENCIL_BUFFER_BIT); // llena con el valor de fondo (0)
    //8 bit => rango [0, 255=2^8-1], pero, para no errarle ~0 es todos los bits en 1
    glStencilFunc (GL_ALWAYS,1,~0); // pasa siempre
    glStencilOp (GL_KEEP, GL_KEEP, GL_REPLACE); // pone en 1 (reemplaza con el ref value)
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glDisable(GL_LIGHTING);
    glPushMatrix();
      glTranslatef(xk,yk,0);
      //circulo
      static GLUquadricObj* q=gluNewQuadric();
      glPushMatrix();
        glTranslatef(0,.25,0);
        gluDisk(q,0,.25,36,1);
      glPopMatrix();
      // triangulo
      glBegin(GL_TRIANGLES);
        glVertex2f(0,.5);
        glVertex2f(-.25,-.5);
        glVertex2f( .25,-.5);
      glEnd();
    glPopMatrix();

  glPopAttrib();
}

////////////////////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------
// redibuja los objetos
// Cada vez que hace un redisplay
void Display_cb() {
//  if (cl_info) cout << "Display\t"; cout.flush();
  if (!dibuja) return;

  drawObjects();
  glutSwapBuffers();

#ifdef _DEBUG
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
#endif // _DEBUG
}

//------------------------------------------------------------
// Regenera la matriz de proyeccion
// cuando cambia algun parametro de la vista
void regen() {
//  if (cl_info) cout << "regen" << endl;
  if (!dibuja) return;

  // matriz de proyeccion
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();

  double w0=(double)w/2/escala,h0=(double)h/2/escala; // semiancho y semialto en el target

  // frustum, perspective y ortho son respecto al eye pero con los z positivos (delante del ojo)
  if (perspectiva){ // perspectiva
    double // "medio" al cuete porque aqui la distancia es siempre 5
      delta[3]={(target[0]-eye[0]), // vector ojo-target
                (target[1]-eye[1]),
                (target[2]-eye[2])},
      dist=sqrt(delta[0]*delta[0]+
                delta[1]*delta[1]+
                delta[2]*delta[2]);
    w0*=znear/dist,h0*=znear/dist;  // w0 y h0 en el near
    glFrustum(-w0,w0,-h0,h0,znear,zfar);
  }
  else { // proyeccion ortogonal
    glOrtho(-w0,w0,-h0,h0,znear,zfar);
  }

  glMatrixMode(GL_MODELVIEW); glLoadIdentity(); // matriz del modelo->view

  if (luz_camara) // luz fija a la camara
    glLightfv(GL_LIGHT0,GL_POSITION,lpos);  // ubica la luz

  gluLookAt(   eye[0],   eye[1],   eye[2],
            target[0],target[1],target[2],
                up[0],    up[1],    up[2]);// ubica la camara

  if (!luz_camara) // luz fija en el espacio del modelo
    glLightfv(GL_LIGHT0,GL_POSITION,lpos);  // ubica la luz

  drawKey();
  Display_cb();
}

//------------------------------------------------------------
// Animacion

// Si no hace nada hace esto
// glutIdleFunc lo hace a la velocidad que de la maquina
// glutTimerFunc lo hace cada tantos milisegundos

// glutTimerFunc funciona mal, hace cosas raras que son muy visibles
// cuando la espera (msecs) es grande

// El "framerate" real (cuadros por segundo)
// depende de la complejidad del modelo (lod) y la aceleracion por hardware
void Idle_cb() {
  static int suma,counter=0;// esto es para analisis del framerate real
  // Cuenta el lapso de tiempo
  static int anterior=glutGet(GLUT_ELAPSED_TIME); // milisegundos desde que arranco
  if (msecs!=1){ // si msecs es 1 no pierdo tiempo
    int tiempo=glutGet(GLUT_ELAPSED_TIME), lapso=tiempo-anterior;
    if (lapso<msecs) return;
    suma+=lapso;
    if (++counter==100) {
//      cout << "<ms/frame>= " << suma/100.0 << endl;
      counter=suma=0;
    }
    anterior=tiempo;
  }
  Display_cb(); // redibuja
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto de la ventana
void Reshape_cb(int width, int height){
  h=height; w=width;
  if (cl_info) cout << "reshape: " << w << "x" << h << endl;

  if (w==0||h==0) {// minimiza
    dibuja=false; // no dibuja mas
    glutIdleFunc(0); // no llama a cada rato a esa funcion
    return;
  }
  else if (!dibuja&&w&&h){// des-minimiza
    dibuja=true; // ahora si dibuja
    glutIdleFunc(Idle_cb); // registra de nuevo el callback
  }

  glViewport(0,0,w,h); // region donde se dibuja
  regen(); //regenera la matriz de proyeccion
}

//------------------------------------------------------------
// Teclado
/*
GLUT ACTIVE SHIFT //Set if the Shift modifier or Caps Lock is active.
GLUT ACTIVE CTRL //Set if the Ctrl modifier is active.
GLUT ACTIVE ALT //Set if the Alt modifier is active.
*/
inline short get_modifiers() {return modifiers=(short)glutGetModifiers();}

// Maneja pulsaciones del teclado (ASCII keys)
// x,y posicion del mouse cuando se teclea
void Keyboard_cb(unsigned char key,int x=0,int y=0) {
  switch (key){
    case 27: // escape => exit
      get_modifiers();
      if (!modifiers)
        exit(EXIT_SUCCESS);
      break;
    case 'a': case 'A': // Antialiasing
      antialias=!antialias;
      if (antialias){
        glEnable(GL_POINT_SMOOTH); glEnable(GL_LINE_SMOOTH); glEnable(GL_POLYGON_SMOOTH);
        if (cl_info) cout << "Antialiasing" << endl;
      }
      else {
        glDisable(GL_POINT_SMOOTH); glDisable(GL_LINE_SMOOTH); glDisable(GL_POLYGON_SMOOTH);
        if (cl_info) cout << "Sin Antialiasing" << endl;
      }
      break;
    case 'f': case 'F': // relleno
      relleno=!relleno;
      if (cl_info) cout << ((relleno)? "Relleno" : "Sin Relleno") << endl;
      break;
    case 'i': case 'I': // info
      cl_info=!cl_info;
      cout << ((cl_info)? "Info" : "Sin Info") << endl;
      break;
    case 'j': case 'J': // luz fija a la camara o en el espacio
      luz_camara=!luz_camara;
      if (cl_info)
        cout << "Luz fija " << ((luz_camara)? "a la camara" : "en el espacio") << endl;
      regen();
      break;
    case 'l': case 'L': // wire
      wire=!wire;
      if (cl_info) cout << ((wire)? "Lineas" : "Sin Lineas") << endl;
      break;
    case 'm': case 'M': // color/material
      color=!color;
      if (color) glDisable(GL_LIGHTING); else glEnable(GL_LIGHTING);
      if (cl_info) cout << ((color)? "Color" : "Material") << endl;
      break;
    case 'p': case 'P':  // perspectiva
      perspectiva=!perspectiva;
      if (cl_info) cout << ((perspectiva)? "Perspectiva" : "Ortogonal") << endl;
      regen();
      break;
    case 'r': case 'R': // rotacion
      rota=!rota;
      if (cl_info) cout << ((rota)? "Gira" : "No Gira") << endl;
      break;
    case 's': case 'S': // smooth
      smooth=!smooth;
      glShadeModel((smooth) ? GL_SMOOTH : GL_FLAT);
      if (cl_info) cout << ((smooth)? "Suave" : "Facetado") << endl;
      break;
    case '+': case '-': // lod
      if (key=='+') lod++; else {lod--; if (lod==0) lod=1;}
      if (cl_info) cout << "Nivel de detalle: "  << lod << endl;
      break;
  }
  if (!rota) glutIdleFunc(0); // no llama a cada rato a esa funcion
  else glutIdleFunc(Idle_cb); // registra el callback
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
    get_modifiers();
    if (modifiers==GLUT_ACTIVE_ALT)
      exit(EXIT_SUCCESS);
  }
  if (key==GLUT_KEY_PAGE_UP||key==GLUT_KEY_PAGE_DOWN){ // velocidad
    if (key==GLUT_KEY_PAGE_DOWN) ms_i++;
    else ms_i--;
    if (ms_i<0) ms_i=0; if (ms_i==ms_n) ms_i--;
    msecs=ms_lista[ms_i];
    if (cl_info){
      if (msecs<1000)
        cout << 1000/msecs << "fps" << endl;
      else
        cout << msecs/1000 << "s/frame)" << endl;
    }
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
    case 'f':
      Keyboard_cb('f');
      return;
    case 'i':
      Keyboard_cb('i');
      return;
    case 'j':
      Keyboard_cb('j');
      return;
    case 'l':
      Keyboard_cb('l');
      return;
    case 'm':
      Keyboard_cb('m');
      return;
    case 'p':
      Keyboard_cb('p');
      return;
    case 'r':
      Keyboard_cb('r');
      return;
    case 's':
      Keyboard_cb('s');
      return;
    case '+':
      Keyboard_cb('+');
      return;
    case '-':
      Keyboard_cb('-');
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
// Movimientos del mouse
void Motion_cb(int xm, int ym){ // drag
  if (boton==GLUT_LEFT_BUTTON){
    if (modifiers==GLUT_ACTIVE_SHIFT){ // cambio de escala
      escala=escala0*exp((yclick-ym)/100.0);
//      cout << "escala: " << escala << "       \r"; cout.flush();
      regen();
    }
    else {
      xk+=float(xm-xclick)/escala;yk+=float(yclick-ym)/escala;
      xclick=xm;yclick=ym;
      drawKey();
      Display_cb();
    }
  }
}

void PMotion_cb(int xm, int ym){
  unsigned char s;
  glReadPixels(xm,h-ym,1,1,GL_STENCIL_INDEX,GL_UNSIGNED_BYTE,&s);
  cout << "\r                   \r" << int(s); cout.flush();
}


// Clicks del mouse
// GLUT LEFT BUTTON, GLUT MIDDLE BUTTON, or GLUT RIGHT BUTTON
// The state parameter is either GLUT UP or GLUT DOWN
// glutGetModifiers may be called to determine the state of modifier keys
void Mouse_cb(int button, int state, int x, int y){
  if (button==GLUT_LEFT_BUTTON){
    if (state==GLUT_DOWN) {
      xclick=x; yclick=y;
      boton=button;
      get_modifiers();
      glutMotionFunc(Motion_cb); // callback para los drags
      if (modifiers==GLUT_ACTIVE_SHIFT){ // cambio de escala
        escala0=escala;
      }
    }
    else if (state==GLUT_UP){
      cout << endl;
      boton=-1;
      glutMotionFunc(0); // anula el callback para los drags
    }
  }
}

//------------------------------------------------------------
// pregunta a OpenGL por el valor de una variable de estado
int integerv(GLenum pname){
  int value;
  glGetIntegerv(pname,&value);
  return value;
}
#define _PRINT_INT_VALUE(pname) #pname << ": " << integerv(pname) <<endl

//------------------------------------------------------------
// Inicializa GLUT y OpenGL
void initialize() {
  // pide z-buffer, color RGBA y double buffering
  glutInitDisplayMode(GLUT_DEPTH|GLUT_RGBA|GLUT_DOUBLE|GLUT_STENCIL);

  glutInitWindowSize(w,h); glutInitWindowPosition(50,50);

  glutCreateWindow("Prueba de Stencil"); // crea el main window

  //declara los callbacks
  //los que no se usan no se declaran
  glutDisplayFunc(Display_cb); // redisplays
  glutReshapeFunc(Reshape_cb); // cambio de alto y ancho
  glutKeyboardFunc(Keyboard_cb); // teclado
  glutSpecialFunc(Special_cb); // teclas especiales
  glutPassiveMotionFunc(PMotion_cb);
  glutMouseFunc(Mouse_cb); // botones picados
  if (!(dibuja&&rota)) glutIdleFunc(0); // no llama a cada rato a esa funcion
  else glutIdleFunc(Idle_cb); // registra el callback

  // crea el menu
  glutCreateMenu(Menu_cb);
    glutAddMenuEntry("     [p]_Perspectiva/Ortogonal ", 'p');
    glutAddMenuEntry("     [r]_Rota                  ", 'r');
    glutAddMenuEntry("     [+]_+Nivel de Detalle     ", '+');
    glutAddMenuEntry("     [-]_-Nivel de Detalle     ", '-');
    glutAddMenuEntry("     [l]_Lineas                ", 'l');
    glutAddMenuEntry("     [f]_Caras Rellenas        ", 'f');
    glutAddMenuEntry("     [a]_Antialiasing ON/OFF   ", 'a');
    glutAddMenuEntry("     [m]_Color/Material        ", 's');
    glutAddMenuEntry("  [PgUp]_Aumenta Framerate     ", (256+GLUT_KEY_PAGE_UP));
    glutAddMenuEntry("  [Pgdn]_Disminuye Framerate   ", (256+GLUT_KEY_PAGE_DOWN));
    glutAddMenuEntry("     [j]_Luz fija ON/OFF       ", 'j');
    glutAddMenuEntry("     [i]_Info ON/OFF           ", 'i');
    glutAddMenuEntry("   [Esc]_Exit                  ", 27);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  // ========================
  // estado normal del OpenGL
  // ========================

  glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); // habilita el z-buffer
  glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset (1,1); // coplanaridad

  // antialiasing
  if (antialias){
    glEnable(GL_POINT_SMOOTH); glEnable(GL_LINE_SMOOTH); glEnable(GL_POLYGON_SMOOTH);
  }
  else {
    glDisable(GL_POINT_SMOOTH); glDisable(GL_LINE_SMOOTH); glDisable(GL_POLYGON_SMOOTH);
  }
  glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
  glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);

  // interpola normales por nodos o una normal por plano
  glShadeModel((smooth) ? GL_SMOOTH : GL_FLAT);

  // poligonos
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glCullFace(GL_BACK); // se habilita por objeto (cerrado o abierto)

  // color o material+luces
  if (color) glDisable(GL_LIGHTING); else glEnable(GL_LIGHTING);

  // define luces
  glLightfv(GL_LIGHT0,GL_AMBIENT,lambient);
  glLightfv(GL_LIGHT0,GL_DIFFUSE,ldiffuse);
  // caras de atras y adelante distintos (1) o iguales (0)
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,0);
  glEnable(GL_LIGHT0);

  // color de fondo
  glClearColor(fondo[0],fondo[1],fondo[2],fondo[3]);

  // para el stencil
  glEnable(GL_STENCIL_TEST);
  glClearStencil(0); // al borrar el stencil lo llena de unos
  glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP); // normalmente no escribe en el stencil

  // material estandar
  static const float
    fambient[]={0.4f,0.2,0.1,.5f},
    fdiffuse[]={0.5f,0.2f,0.1f,.5f},
    fspecular[]={0.6f,0.2f,0.1f,.5f},
    bambient[]={0.2f,0.4f,0.4f,0.5f},
    bdiffuse[]={0.2f,0.5f,0.4f,.5f},
    bspecular[]={0.2f,0.6f,0.4f,.5f};
  static const int
    fshininess=50,
    bshininess=10;
  glMaterialfv(GL_FRONT,GL_AMBIENT,fambient);
  glMaterialfv(GL_FRONT,GL_DIFFUSE,fdiffuse);
  glMaterialfv(GL_FRONT,GL_SPECULAR,fspecular);
  glMateriali(GL_FRONT,GL_SHININESS,fshininess);
  glMaterialfv(GL_BACK,GL_AMBIENT,bambient);
  glMaterialfv(GL_BACK,GL_DIFFUSE,bdiffuse);
  glMaterialfv(GL_BACK,GL_SPECULAR,bspecular);
  glMateriali(GL_BACK,GL_SHININESS,bshininess);

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

  regen(); // para que setee las matrices antes del 1er draw
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv);// inicializa glut
  initialize(); // condiciones iniciales de la ventana y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0;
}
