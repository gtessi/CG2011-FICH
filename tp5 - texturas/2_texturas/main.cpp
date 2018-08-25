// Aplicación de texturas
// Consigna: Buscar las @@@@ y seguir las instrucciones

#include <cmath> // atan sqrt
#include <cstdlib> // exit
#include <iostream> // cout
#include <fstream> // file io
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

int
  w=480,h=360, // tamaño de la ventana
  boton=-1, // boton del mouse clickeado
  xclick,yclick, // x e y cuando clickeo un boton
  lod=32; // nivel de detalle (subdivisiones de lineas y superficies parametricas)
float // luces y colores en float
  lpos[]={2,1,5,1}, // posicion luz, l[4]: 0 => direccional -- 1 => posicional
  line_color[]={.2f,.3f,.4f,1},
  control_color[]={.5f,.5f,.1f,1}, // poliedros y poligonos de control
  point_color[]={.3f,.3f,.1f,1},   // puntos sueltos o puntos de control
  linewidth=1,pointsize=5, // ancho de lineas y puntos
  escala=100,escala0, // escala de los objetos window/modelo pixeles/unidad
  eye[]={0,0,5}, target[]={0,0,0}, up[]={0,1,0}, // camara, mirando hacia y vertical
  znear=2, zfar=8, //clipping planes cercano y alejado de la camara (en 5 => veo de 3 a -3)
  amy,amy0, // angulo del modelo alrededor del eje y
  ac0,rc0; // angulo resp x y distancia al target de la camara al clickear

bool // variables de estado de este programa
  luz_camara=true,  // luz fija a la camara o al espacio
  perspectiva=true, // perspectiva u ortogonal
  rota=false,       // gira continuamente los objetos respecto de y
  animado=false,    // el objeto actual es animado
  dibuja=true,      // false si esta minimizado
  wire=false,       // dibuja lineas o no
  relleno=true,     // dibuja relleno o no
  smooth=true,      // normales por nodo o por plano
  cl_info=true,     // informa por la linea de comandos
  antialias=false,  // antialiasing
  clamp=false,      // textura clamp/repeat
  nearest=false,    // filtro GL_NEAREST/GL_LINEAR_MIPMAP_LINEAR
  blend=false;      // transparencias

short modifiers=0;  // ctrl, alt, shift (de GLUT)
inline short get_modifiers() {return modifiers=(short)glutGetModifiers();}

// temporizador:
static const int ms_lista[]={1,2,5,10,20,50,100,200,500,1000,2000,5000},ms_n=12;
static int ms_i=4,msecs=ms_lista[ms_i]; // milisegundos por frame

static const double R2G=45/atan(1.0);

GLuint texid;
static const unsigned int automodelist[]={GL_OBJECT_LINEAR,GL_EYE_LINEAR,GL_SPHERE_MAP};
static const char automodestring[3][20]={"GL_OBJECT_LINEAR","GL_EYE_LINEAR","GL_SPHERE_MAP"};
static const unsigned int texmodelist[]={GL_DECAL,GL_REPLACE,GL_MODULATE,GL_BLEND,0};
static const char texmodestring[5][20]={"GL_DECAL","GL_REPLACE","GL_MODULATE","GL_BLEND","Sin Textura"};
int
  texmode=2, // modo de aplicacion de textura
  Sautomode=2, // modo de generacion de la textura
  Tautomode=2, // modo de generacion de la textura
  st=2; // modo S, T o ST

// Load a PPM Image
bool mipmap_ppm(char *ifile)
{
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
  // conversion a rgba, con alpha=255-(r+g+b)/3 (blanco=transparente, negro=opaco)
  unsigned char r,g,b,*imga=new unsigned char[4*wt*ht];
  for (int i=0;i<wt*ht;i++){
    r=imga[4*i+0]=img[3*i+0];
    g=imga[4*i+1]=img[3*i+1];
    b=imga[4*i+2]=img[3*i+2];
    imga[4*i+3]=255-(r+g+b)/3;
  }
  delete[] img;
  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, wt, ht,  GL_RGBA, GL_UNSIGNED_BYTE, imga);

  delete[] imga;
  return true;
}

//------------------------------------------------------------
// redibuja los objetos
extern void drawObjects(int cambia=0);
// Cada vez que hace un redisplay
void Display_cb() { // Este tiene que estar
//  if (cl_info) cout << "Display\t"; cout.flush();
  if (!dibuja) return;
  // borra los buffers de pantalla y z
  glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

  drawObjects();
  glutSwapBuffers();

#ifdef _DEBUG
  // chequea errores
  int errornum=glGetError();
  while(errornum!=GL_NO_ERROR){
    cout << "                                 ERROR: ";
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

  glRotatef(amy,0,1,0); // rota los objetos alrededor de y

  if (!luz_camara) // luz fija en el espacio del modelo
    glLightfv(GL_LIGHT0,GL_POSITION,lpos);  // ubica la luz

  glutPostRedisplay(); // avisa que hay que redibujar
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
      cout << "<ms/frame>= " << suma/100.0 << endl;
      counter=suma=0;
    }
    anterior=tiempo;
  }
  if (rota) { // los objetos giran 1 grado alrededor de y
    amy+=1; if (amy>360) amy-=360;
    regen();
  }
  else glutPostRedisplay();
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
  Display_cb();
}


//------------------------------------------------------------
// Movimientos del mouse
void Motion_cb(int xm, int ym){ // drag
  if (boton==GLUT_LEFT_BUTTON){
    if (modifiers==GLUT_ACTIVE_SHIFT){ // cambio de escala
      escala=escala0*exp((yclick-ym)/100.0);
      regen();
    }
    else { // manipulacion
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
  static bool old_rota=false;
  if (button==GLUT_LEFT_BUTTON){
    if (state==GLUT_DOWN) {
      xclick=x; yclick=y;
      boton=button;
      old_rota=rota; rota=false;
      get_modifiers();
      glutMotionFunc(Motion_cb); // callback para los drags
      if (modifiers==GLUT_ACTIVE_SHIFT){ // cambio de escala
        escala0=escala;
      }
      else { // manipulacion
        double yc=eye[1]-target[1],zc=eye[2]-target[2];
        rc0=sqrt(yc*yc+zc*zc); ac0=atan2(yc,zc);
        amy0=amy;
      }
    }
    else if (state==GLUT_UP){
      rota=old_rota;
      boton=-1;
      glutMotionFunc(0); // anula el callback para los drags
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
  if (key==GLUT_KEY_F5){ // relee textura
    mipmap_ppm("textura.ppm");
    glutPostRedisplay();
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
// Maneja pulsaciones del teclado (ASCII keys)
void Keyboard_cb(unsigned char key,int x=0,int y=0) {
  switch (key){
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
    case 'b': case 'B': // modo de aplicacion de textura
      if (key=='b') texmode=(texmode+4)%5; else texmode=(texmode+1)%5;
      if (texmode<4){
        glEnable(GL_TEXTURE_2D);
          //@@@@
          //Fijar aqui el modo seleccionado;
          glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,texmodelist[texmode]);          
      }
      else
        glDisable(GL_TEXTURE_2D);
      if (cl_info) cout << "Blending Mode: " << texmodestring[texmode] << endl;
      break;
    case 'c': case 'C': // cambia
      drawObjects((key=='c') ? 1 : -1);
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
    case 'n': case 'N':  // filtro GL_NEAREST/GL_LINEAR_MIPMAP_LINEAR
      nearest=!nearest;     
      //@@@@
      //Fijar aqui el modo seleccionado
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, (nearest) ? GL_NEAREST : GL_LINEAR_MIPMAP_LINEAR);
      if (cl_info) cout << "Filtro " << ((nearest)? "GL_NEAREST" : "GL_LINEAR_MIPMAP_LINEAR") << endl;
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
    case 't': case 'T': // transparencia
      blend=!blend;
      if (blend){
        glEnable(GL_BLEND);
        if (cl_info) cout << "Transparencia" << endl;
      }
      else {
        glDisable(GL_BLEND);
        if (cl_info) cout << "Sin Transparencia" << endl;
      }
      break;
    case 'm': case 'M': // modo S, T o ST
      if (key=='m') st=(st+1)%3; else st=(st+2)%3;
      if (st==0){
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,(clamp)? GL_CLAMP : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,(!clamp)? GL_CLAMP : GL_REPEAT);
      }
      if (st==1){
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,(!clamp)? GL_CLAMP : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,(clamp)? GL_CLAMP : GL_REPEAT);
      }
      if (st==2){
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,(clamp)? GL_CLAMP : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,(clamp)? GL_CLAMP : GL_REPEAT);
      }
      if (cl_info) cout << ((st==0)? "Modo S" : ((st==1)? "Modo T" : "Modo ST")) << endl;
      break;
    case 'v': case 'V':  // Clamp/Repeat
      clamp=!clamp;
      //@@@@
      //Fijar aqui el modo seleccionado (s y t juntos o agregar otro case asi se pueden separar)
      if (st==0){
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,(clamp)? GL_CLAMP : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,(!clamp)? GL_CLAMP : GL_REPEAT);
      }
      if (st==1){
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,(!clamp)? GL_CLAMP : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,(clamp)? GL_CLAMP : GL_REPEAT);
      }
      if (st==2){
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,(clamp)? GL_CLAMP : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,(clamp)? GL_CLAMP : GL_REPEAT);
      }
      if (cl_info) cout << ((clamp)? "Clamp" : "Repeat") << endl;
      break;
    case 'x': case 'X': // modo de generacion de textura
      if (key==x) Sautomode=(Sautomode+2)%3; else Sautomode=(Sautomode+1)%3;
      //@@@@
      //Fijar aqui el modo seleccionado
      glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,automodelist[Sautomode]);
      if (cl_info) cout << "Generation Mode S: " << automodestring[Sautomode] << endl;
      break;
    case 'y': case 'Y': // modo de generacion de textura
      if (key==y) Tautomode=(Tautomode+2)%3; else Tautomode=(Tautomode+1)%3;
      //@@@@
      //Fijar aqui el modo seleccionado
      glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,automodelist[Tautomode]);
      if (cl_info) cout << "Generation Mode T: " << automodestring[Tautomode] << endl;
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
  if (!(animado|rota)) glutIdleFunc(0); // no llama a cada rato a esa funcion
  else glutIdleFunc(Idle_cb); // registra el callback
  glutPostRedisplay();
}

//------------------------------------------------------------
// Menu
void Menu_cb(int value)
{
  if (value<256) Keyboard_cb(value);
  else Special_cb(256-value);
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
  glutInitDisplayMode(GLUT_DEPTH|GLUT_RGBA|GLUT_DOUBLE);

  glutInitWindowSize(w,h); glutInitWindowPosition(50,50);

  glutCreateWindow("Aplicacion de Texturas"); // crea el main window

  //declara los callbacks
  //los que no se usan no se declaran
  glutDisplayFunc(Display_cb); // redisplays
  glutReshapeFunc(Reshape_cb); // cambio de alto y ancho
  glutKeyboardFunc(Keyboard_cb); // teclado
  glutSpecialFunc(Special_cb); // teclas especiales
  glutMouseFunc(Mouse_cb); // botones picados
  if (!(dibuja&&(animado|rota))) glutIdleFunc(0); // no llama a cada rato a esa funcion
  else glutIdleFunc(Idle_cb); // registra el callback

  // crea el menu
  glutCreateMenu(Menu_cb);
    glutAddMenuEntry("     [a]_Antialiasing          ", 'a');
    glutAddMenuEntry("     [b]_Modo Aplicacion Sig.  ", 'b');
    glutAddMenuEntry("     [B]_Modo Aplicacion Ant.  ", 'B');
    glutAddMenuEntry("     [c]_Objeto Siguiente      ", 'c');
    glutAddMenuEntry("     [C]_Objeto Anterior       ", 'C');
    glutAddMenuEntry("     [f]_Caras Rellenas        ", 'f');
    glutAddMenuEntry("     [i]_Info ON/OFF           ", 'i');
    glutAddMenuEntry("     [j]_Luz fija ON/OFF       ", 'j');
    glutAddMenuEntry("     [l]_Lineas                ", 'l');
    glutAddMenuEntry("     [m]_Modo S, T o ST Sig.   ", 'm');
    glutAddMenuEntry("     [m]_Modo S, T o ST Ant.   ", 'M');
    glutAddMenuEntry("     [n]_Nearest/LinMipmapLin  ", 'n');
    glutAddMenuEntry("     [p]_Perspectiva/Ortogonal ", 'p');
    glutAddMenuEntry("     [r]_Rota                  ", 'r');
    glutAddMenuEntry("     [s]_Suave/Facetado        ", 's');
    glutAddMenuEntry("     [t]_Transparencia         ", 't');
    glutAddMenuEntry("     [v]_Clamp/Repeat          ", 'v');
    glutAddMenuEntry("     [x]_Modo Generacion S Sig.", 'x');
    glutAddMenuEntry("     [X]_Modo Generacion S Ant.", 'X');
    glutAddMenuEntry("     [y]_Modo Generacion T Sig.", 'y');
    glutAddMenuEntry("     [Y]_Modo Generacion T Ant.", 'Y');
    glutAddMenuEntry("     [+]_+Nivel de Detalle     ", '+');
    glutAddMenuEntry("     [-]_-Nivel de Detalle     ", '-');
    glutAddMenuEntry("    [F5]_Relee textura.ppm     ", (256+GLUT_KEY_F5));
    glutAddMenuEntry("    [Up]_Sube Camara           ", (256+GLUT_KEY_UP));
    glutAddMenuEntry("  [Down]_Baja Camara           ", (256+GLUT_KEY_DOWN));
    glutAddMenuEntry("  [Left]_Gira objeto           ", (256+GLUT_KEY_LEFT));
    glutAddMenuEntry(" [Right]_Gira objeto           ", (256+GLUT_KEY_RIGHT));
    glutAddMenuEntry("  [PgUp]_Aumenta Framerate     ", (256+GLUT_KEY_PAGE_UP));
    glutAddMenuEntry("  [Pgdn]_Disminuye Framerate   ", (256+GLUT_KEY_PAGE_DOWN));
    glutAddMenuEntry("   [Esc]_Exit                  ", 27);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  // ========================
  // estado normal del OpenGL
  // ========================

  glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); // habilita el z-buffer
  glEnable(GL_NORMALIZE); // para que el scaling no moleste
  glDisable(GL_AUTO_NORMAL); // para nurbs??
  glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset (1,1); // coplanaridad

  // interpola normales por nodos o una normal por plano
  glShadeModel((smooth) ? GL_SMOOTH : GL_FLAT);

  // ancho de lineas y puntos
  glLineWidth(linewidth); glPointSize(pointsize);

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

  // transparencias
  if (blend) glEnable(GL_BLEND);
  else glDisable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  // color de fondo
  glClearColor(.6f,.8f,.6f,0);

  // direccion de los poligonos
  glFrontFace(GL_CCW); glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glCullFace(GL_BACK); // se habilita por objeto (cerrado o abierto)

  // define luces
  static const float
    lambient[]={.5f,.5f,.5f,1}, // luz ambiente
    ldiffuse[]={.7f,.7f,.7f,1}, // luz difusa
    lspecular[]={1,1,1,1};      // luz especular
  glLightfv(GL_LIGHT0,GL_AMBIENT,lambient);
  glLightfv(GL_LIGHT0,GL_DIFFUSE,ldiffuse);
  glLightfv(GL_LIGHT0,GL_SPECULAR,lspecular);
  // caras de atras y adelante distintos (1) o iguales (0)
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,0);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);

  // material estandar
  static const float
    fambient[]={.4f,.2f,.1f,1},
    fdiffuse[]={.5f,.2f,.1f,1},
    fspecular[]={1,1,1,1},
    bambient[]={.2f,.4f,.4f,1},
    bdiffuse[]={.2f,.5f,.4f,1},
    bspecular[]={1,1,1,1};
  static const int
    fshininess=50,
    bshininess=100;
  glMaterialfv(GL_FRONT,GL_AMBIENT,fambient);
  glMaterialfv(GL_FRONT,GL_DIFFUSE,fdiffuse);
  glMaterialfv(GL_FRONT,GL_SPECULAR,fspecular);
  glMateriali(GL_FRONT,GL_SHININESS,fshininess);
  glMaterialfv(GL_BACK,GL_AMBIENT,bambient);
  glMaterialfv(GL_BACK,GL_DIFFUSE,bdiffuse);
  glMaterialfv(GL_BACK,GL_SPECULAR,bspecular);
  glMateriali(GL_BACK,GL_SHININESS,bshininess);

  // textura
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glGenTextures(1, &texid);
  glBindTexture(GL_TEXTURE_2D, texid);
  mipmap_ppm("textura.ppm");
  //@@@@
  //Fijar aqui el modo seleccionado para clamp/repeat
  //Fijar aqui el modo seleccionado para los filtros
  //Fijar aqui el modo seleccionado para generacion automatica

  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  if (texmode<4){
    glEnable(GL_TEXTURE_2D);
    //@@@@
    //Fijar aqui el modo seleccionado para blending
  }
  else
    glDisable(GL_TEXTURE_2D);
  float splane[]={1.f,0.f,0.f,0.f},tplane[]={0.f,1.f,0.f,0.f};
  glTexGenfv(GL_S, GL_OBJECT_PLANE, splane);
  glTexGenfv(GL_T, GL_OBJECT_PLANE, tplane);
  glTexGenfv(GL_S, GL_EYE_PLANE, splane);
  glTexGenfv(GL_T, GL_EYE_PLANE, tplane);

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
