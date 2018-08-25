// Transformaciones

#include <cmath> // atan sqrt
#include <cstdlib> // exit
#include <iostream> // cout
#include <fstream> // file io
#include <GL/glut.h>
#include <iomanip>

/*
// comentar este bloque si se quiere una ventana de comando
#ifdef _WIN32
 #pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#endif
*/

using namespace std;

//------------------------------------------------------------
// variables globales y defaults

float aang=0; // orientacion
float ax=0,ay=0; // posicion en la pista
float aacel=0; // acelerador (eje y del mouse)
float aspeed=0; // velocidad actual
float topspeed=50; // velocidad maxima
float rang=0; // direccion de las ruedas delanteras respecto al auto (eje x del mouse) 
float rang2=0; // giro de las ruedas sobre su eje, cuando el auto avanza 

int
  w=640,h=480, // tamaño de la ventana
  boton=-1, // boton del mouse clickeado
  xclick,yclick, // x e y cuando clickeo un boton
  lod=10; // nivel de detalle (subdivisiones de lineas y superficies parametricas)
float // luces y colores en float
  lpos[]={2,1,5,0}, // posicion luz, l[4]: 0 => direccional -- 1 => posicional
  escala=100,escala0, // escala de los objetos window/modelo pixeles/unidad
  eye[]={0,2,3}, target[]={0,0,-1}, up[]={0,1,0}, // camara, mirando hacia y vertical
  znear=2, zfar=50, //clipping planes cercano y alejado de la camara (en 5 => veo de 3 a -3)
  amy,amy0, // angulo del modelo alrededor del eje y
  ac0=1,rc0, // angulo resp x y distancia al target de la camara al clickear
//  sky_color[]={.4,.4,.8,0}; // color del fondo y de la niebla (azul)
  sky_color[]={0,0,0,0}; // color del fondo y de la niebla (negro)

bool // variables de estado de este programa
  luz_camara=true,  // luz fija a la camara o al espacio
  perspectiva=true, // perspectiva u ortogonal
  animado=false,    // el auto se mueve por la pista
  dibuja=true,      // false si esta minimizado
  relleno=true,     // dibuja relleno o no
  cl_info=true,     // informa por la linea de comandos
  fog=false;

// tamaño de la pista sobre
int text_w = 1024/4; 
int text_h = 1024/4;

short modifiers=0;  // ctrl, alt, shift (de GLUT)
inline short get_modifiers() {return modifiers=(short)glutGetModifiers();}

// temporizador:
static int msecs=20; // milisegundos por frame

static const double R2G=45/atan(1.0);
static const double G2R=atan(1.0)/45;

GLuint texid;

// Load a PPM Image
bool mipmap_ppm(const char *ifile) {
  char dummy; int maxc,wt,ht;
  ifstream fileinput(ifile, ios::binary);
  if (!fileinput.is_open()) {cerr<<"Not found"<<endl; return false;}
  fileinput.get(dummy);
  if (dummy!='P') {cerr<<"Not P6 PPM file"<<endl; return false;}
  fileinput.get(dummy);
  if (dummy!='6') {cerr<<"Not P6 PPM file"<<endl; return false;}
  fileinput.get(dummy);
  dummy=fileinput.peek();
  if (dummy=='#') do {
    fileinput.get(dummy);
  } while (dummy!=10);
  fileinput >> wt >> ht;
  fileinput >> maxc;
  fileinput.get(dummy);
  
  unsigned char *img=new unsigned char[3*wt*ht];
  fileinput.read((char *)img, 3*wt*ht);
  fileinput.close();
  //gluBuild2DMipmaps(GL_TEXTURE_2D, 3, wt, ht,  GL_RGB, GL_UNSIGNED_BYTE, img);
  // conversion a rgba alpha=255-(r+g+b)/3 (blanco=transparente, negro=opaco)
  unsigned char *imga=new unsigned char[4*wt*ht];
  unsigned char r,g,b;
  int i;
  for (i=0;i<wt*ht;i++){
    r=imga[4*i+0]=img[3*i+0];
    g=imga[4*i+1]=img[3*i+1];
    b=imga[4*i+2]=img[3*i+2];
    imga[4*i+3]=((r+g+b==765)? 0: 255);
    //imga[4*i+3]=255-(r+g+b)/3;
  }
  delete[] img;
  gluBuild2DMipmaps(GL_TEXTURE_2D, 4, wt, ht,  GL_RGBA, GL_UNSIGNED_BYTE, imga);
  delete[] imga;
  return true;
}

//------------------------------------------------------------
// redibuja los objetos
extern void drawObjects();
// Cada vez que hace un redisplay
void Display_cb() { // Este tiene que estar
//  if (cl_info) cout << "Display\t"; cout.flush();
  if (!dibuja) return;
  // borra los buffers de pantalla y z
  glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
  if (relleno) 
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  else
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  
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

  glLightfv(GL_LIGHT0,GL_POSITION,lpos);  // ubica la luz
  gluLookAt(   eye[0],   eye[1],   eye[2],
            target[0],target[1],target[2],
                up[0],    up[1],    up[2]);// ubica la camara

  glRotatef(amy,0,1,0); // rota los objetos alrededor de y

  glLightfv(GL_LIGHT0,GL_POSITION,lpos);  // ubica la luz

  Display_cb(); // avisa que hay que redibujar
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
  static int anterior=glutGet(GLUT_ELAPSED_TIME); // milisegundos desde que arranco
  if (msecs!=1){ // si msecs es 1 no pierdo tiempo
    int tiempo=glutGet(GLUT_ELAPSED_TIME), lapso=tiempo-anterior;
    if (lapso<msecs) return;
    anterior=tiempo;
  }
  aspeed+=aacel*.75-.25;
  if (aspeed<0) aspeed=0;
  else if (aspeed>topspeed) aspeed=topspeed;
  ax+=aspeed*cos(aang*G2R)/100;
  ay-=aspeed*sin(aang*G2R)/100;
  aang+=rang*aspeed/150;
  rang2+=aspeed;
  if (ax<-text_w) ax+=text_w*2;
  else if (ax>text_w) ax-=text_w*2;
  if (ay<-text_h) ay+=text_h*2;
  else if (ay>text_h) ay-=text_h*2;
  if (cl_info) cerr<<setprecision(2)<<fixed<<"\rPos: "<<ax<<","<<ay<<"  Gas: "<<aacel<<"  Vel: "<<aspeed<<"  Ang: "<<aang<<"  ";
  
  glutPostRedisplay();
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
// Movimientos del mouse

// controlar el auto, dirección y aceleración
void Passive_Motion_cb(int xm, int ym){ // drag
  rang=float(xm-w/2)/w*float(15*aspeed+60*(topspeed-aspeed))/topspeed;
  aacel=float(h-ym*1.5)/h;
  glutPostRedisplay();
}

// girar la camara o hacer zoom
void Motion_cb(int xm, int ym){ // drag
  if (boton==GLUT_LEFT_BUTTON){
    if (modifiers==GLUT_ACTIVE_SHIFT){ // cambio de escala
      escala=escala0*exp((yclick-ym)/100.0);
      regen();
    } else { // manipulacion
      double yc=eye[1]-target[1],zc=eye[2]-target[2];
      double ac=ac0+(ym-yclick)*720.0/h/R2G;
      yc=rc0*sin(ac); zc=rc0*cos(ac);
      up[1]=zc; up[2]=-yc;  // perpendicular
      eye[1]=target[1]+yc; eye[2]=target[2]+zc;
      amy=amy0+(xm-xclick)*180.0/w;
      regen();
    }
  }
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
      glutMotionFunc(Motion_cb);
      if (modifiers==GLUT_ACTIVE_SHIFT) escala0=escala; // escala      
      else if (modifiers==(GLUT_ACTIVE_ALT|GLUT_ACTIVE_CTRL)) return; // origen textura
      else { // manipulacion
        double yc=eye[1]-target[1],zc=eye[2]-target[2];
        rc0=sqrt(yc*yc+zc*zc); ac0=atan2(yc,zc);
        amy0=amy;
      }
    }
    else if (state==GLUT_UP){
      boton=-1;
      glutMotionFunc(NULL);
    }
  }
}
//------------------------------------------------------------
// Teclado
// Special keys (non-ASCII)
// aca es int key
void Special_cb(int key,int xm=0,int ym=0) {
  if (key==GLUT_KEY_F4){ // alt+f4 => exit
    get_modifiers();
    if (modifiers==GLUT_ACTIVE_ALT)
      exit(EXIT_SUCCESS);
  }
  if (key==GLUT_KEY_UP||key==GLUT_KEY_DOWN){ // camara
    // la camara gira alrededor del eje -x
    double yc=eye[1]-target[1],zc=eye[2]-target[2],
           rc=sqrt(yc*yc+zc*zc),ac=atan2(yc,zc);
    ac+=((key==GLUT_KEY_UP) ? 2 : -2)/R2G;
    yc=rc*sin(ac); zc=rc*cos(ac);
    up[1]=zc; up[2]=-yc;  // perpendicular
    eye[1]=target[1]+yc; eye[2]=target[2]+zc;
    regen();
  }
  if (key==GLUT_KEY_LEFT){ // gira
    amy-=1;
    regen();
  }
  if (key==GLUT_KEY_RIGHT){ // gira
    amy+=1;
    regen();
  }
}

// Maneja pulsaciones del teclado (ASCII keys)
void Keyboard_cb(unsigned char key,int x=0,int y=0) {
  switch (key){
    case 'f': case 'F': // relleno
      relleno=!relleno;
      if (relleno) 
        glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      else
        glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
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
    case 'p': case 'P':  // perspectiva
      perspectiva=!perspectiva;
      if (cl_info) cout << ((perspectiva)? "Perspectiva" : "Ortogonal") << endl;
      regen();
      break;
    case 'r': case 'R': // movimiento
      if ((animado=!animado)) 
        glutIdleFunc(Idle_cb);
      else
        glutIdleFunc(0);
      if (cl_info) cout << ((animado)? "Animado" : "No Animado") << endl;
      break;
    case 'n': case 'N':  // niebla
      fog=!fog;
      if (fog) glEnable (GL_FOG); else glDisable(GL_FOG);
      if (cl_info) cout << ((fog)? "Con Nibla" : "Sin Niebla") << endl;
      break;
    case 27: // escape => exit
      get_modifiers();
      if (!modifiers)
        exit(EXIT_SUCCESS);
      break;
    case '+': case '-': // lod
      if (key=='+') lod++; else {lod--; if (lod==0) lod=1;}
      if (cl_info) cout << "Nivel de detalle: "  << lod << endl;
      break;
  }
  regen();
  glutPostRedisplay();
}

//------------------------------------------------------------
// Menu
void Menu_cb(int value)
{
  switch (value){
    case 'n':
      Keyboard_cb('n');
      return;
    case 'f':
      Keyboard_cb('f');
      return;
    case 'i':
      Keyboard_cb('i');
      return;
    case 'p':
      Keyboard_cb('p');
      return;
    case 'r':
      Keyboard_cb('r');
      return;
    case '+':
      Keyboard_cb('+');
      return;
    case '-':
      Keyboard_cb('-');
      return;
    case 27: //esc
      exit(EXIT_SUCCESS);
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
  // pide z-buffer, color RGB y double buffering
  glutInitDisplayMode(GLUT_DEPTH|GLUT_RGB|GLUT_ALPHA|GLUT_DOUBLE);

  glutInitWindowSize(w,h); glutInitWindowPosition(50,50);

  glutCreateWindow("Transformaciones-F1"); // crea el main window

  //declara los callbacks
  //los que no se usan no se declaran
  glutDisplayFunc(Display_cb); // redisplays
  glutReshapeFunc(Reshape_cb); // cambio de alto y ancho
  glutKeyboardFunc(Keyboard_cb); // teclado
  glutSpecialFunc(Special_cb); // teclas especiales
  glutMouseFunc(Mouse_cb); // botones picados
  glutPassiveMotionFunc(Passive_Motion_cb); // callback para los drags      
  /*glutMotionFunc(Motion_cb); // callback para los drags      */
  if (!(dibuja&&(animado))) glutIdleFunc(0); // no llama a cada rato a esa funcion
  else glutIdleFunc(Idle_cb); // registra el callback

  // crea el menu
  glutCreateMenu(Menu_cb);
    glutAddMenuEntry("   [f]_Caras Rellenas        ", 'f');
    glutAddMenuEntry("   [i]_Info On/Off           ", 'i');
    glutAddMenuEntry("   [p]_Perspectiva/Ortogonal ", 'p');
    glutAddMenuEntry("   [r]_Anima                 ", 'r');
    glutAddMenuEntry("   [n]_Niebla On/Off         ", 'n');
    glutAddMenuEntry("   [+]_+Nivel de Detalle     ", '+');
    glutAddMenuEntry("   [-]_-Nivel de Detalle     ", '-');
    glutAddMenuEntry(" [Esc]_Exit                  ", 27);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  // ========================
  // estado normal del OpenGL
  // ========================

  glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); // habilita el z-buffer
  glEnable(GL_NORMALIZE); // para que el scaling no moleste
  glEnable(GL_AUTO_NORMAL); // para nurbs??
  glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset (1,1); // coplanaridad

  // interpola normales por nodos o una normal por plano
//  glShadeModel(GL_FLAT);

  // color de fondo
  glClearColor(sky_color[0],sky_color[1],sky_color[2],sky_color[3]);

  // direccion de los poligonos
  glFrontFace(GL_CCW); glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glCullFace(GL_BACK); glDisable(GL_CULL_FACE);// se habilita por objeto (cerrado o abierto)

  // define luces
  static const float
    lambient[]={.4f,.4f,.4f,1}, // luz ambiente
    ldiffuse[]={.7f,.7f,.7f,1}, // luz difusa
    lspecular[]={1,1,1,1};      // luz especular
  glLightfv(GL_LIGHT0,GL_AMBIENT,lambient);
  glLightfv(GL_LIGHT0,GL_DIFFUSE,ldiffuse);
  glLightfv(GL_LIGHT0,GL_SPECULAR,lspecular);
  // caras de atras y adelante distintos (1) o iguales (0)
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,0);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,0);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);

  // material estandar
  static const float
    fambient[]={.4f,.2f,.1f,1},
    fdiffuse[]={.5f,.2f,.1f,.5},
    fspecular[]={1,1,1,1},
    bambient[]={.2f,.4f,.4f,1},
    bdiffuse[]={.2f,.5f,.4f,.5},
    bspecular[]={1,1,1,1};
  static const int
    fshininess=25,
    bshininess=50;
  glMaterialfv(GL_FRONT,GL_AMBIENT,fambient);
  glMaterialfv(GL_FRONT,GL_DIFFUSE,fdiffuse);
  glMaterialfv(GL_FRONT,GL_SPECULAR,fspecular);
  glMateriali(GL_FRONT,GL_SHININESS,fshininess);
  glMaterialfv(GL_BACK,GL_AMBIENT,bambient);
  glMaterialfv(GL_BACK,GL_DIFFUSE,bdiffuse);
  glMaterialfv(GL_BACK,GL_SPECULAR,bspecular);
  glMateriali(GL_BACK,GL_SHININESS,bshininess);
  glEnable(GL_COLOR_MATERIAL);
  
  // textura
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glGenTextures(1, &texid);
  glBindTexture(GL_TEXTURE_2D, texid);
  mipmap_ppm("pista.ppm");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  
  // efecto de niebla
  glFogi (GL_FOG_MODE, GL_EXP2);
  glFogfv (GL_FOG_COLOR, sky_color);
  glFogf (GL_FOG_DENSITY, .03);
  if (fog) glEnable(GL_FOG);

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
