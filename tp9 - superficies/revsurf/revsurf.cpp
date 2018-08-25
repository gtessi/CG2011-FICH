#include <iostream> // cin, cout
#include <cstdlib> // exit
#include <cmath> // fabs
#include <cstring> // memcpy
#include <GL/glut.h>

using namespace std;

///////////////////////////////////////////////////////////
// clase: puntos de cuatro floats
// 4 para poder hacer racionales y porque GLU pide 3D o 4D
// floats porque GLU pide floats
// tiene solo lo necesario para usarlos aca
const size_t SZF=sizeof(float);
class p4f{
  float c[4];
public:
  p4f(double x=0, double y=0, double z=0, double w=1){
    c[0]=x; c[1]=y; c[2]=z; c[3]=w;
  }
  p4f(const p4f &p){memcpy(c,p.c,4*SZF);}

  operator const float*() const {return c;}

  p4f& operator=(const p4f &p){memcpy(c,p.c,4*SZF); return *this;}

  // modifica el peso pero mantiene el punto en el mismo lugar de R3
  // ==> OJO: no admite peso 0 !!!!!!!!!!!!!!!!!
  p4f& peso(float w){c[0]*=w/c[3]; c[1]*=w/c[3]; c[2]*=w/c[3]; c[3]=w; return *this;}

  bool cerca2D(const p4f &p, float r=1){
    return (fabs(c[0]/c[3]-p[0]/p[3])<=r && fabs(c[1]/c[3]-p[1]/p[3])<=r);
  }

  float& operator[](int i) {return c[i];}
  const float& operator[](int i) const {return c[i];}
  /*
  // asigna el punto interpolado
  p4f& lerp(const p4f& p0,const p4f& p1,float u){
    c[0]=(1-u)*p0[0]+u*p1[0];
    c[1]=(1-u)*p0[1]+u*p1[1];
    c[2]=(1-u)*p0[2]+u*p1[2];
    c[3]=(1-u)*p0[3]+u*p1[3];
    return *this;
  }
  */
};
///////////////////////////////////////////////////////////

// variables
int
  w2,h2,w3,h3, // alto y ancho de la pantalla
  tol=10, // tolerancia para seleccionar un punto
  xclick,yclick, // punto picado
  lod=16; // nivel de detalle (subdivisiones de lineas y superficies parametricas)

int window2=0, window3=0; // handles de las ventanas

float // luces y colores en float; alfa=.5 para probar transparecias
  fondo[]={0.95f,0.98f,1.0f,1.0f},  // color de fondo
  lc_c[]={.4f,.4f,.6f,1.0f},       // poligono de control
  pc_c[]={.2f,.2f,.4f,1.0f},       // puntos de control
  c_c[]={.8f,.4f,.4f,1.0f},        // curva
  lambient[]={.3f,.3f,.3f}, // luz ambiente
  ldiffuse[]={.7f,.7f,.7f}; // luz difusa

// las ventanas son del orden de 1000 entonces fijamos la distancia ojo-target en 2000
// pero cuidado con el z (depth-buffer, polygon offset) hay que ajustar znear y zfar
float
  eye[]={0.f,0.f,2000.f}, target[]={0.f,0.f,0.f}, up[]={0.f,1.f,0.f}, // camara, mirando hacia y vertical
  lpos[]={2000.f,1000.f,5000.f,1.f}; // posicion luz, l[4]: 0 => direccional -- 1 => posicional

float
  escala=1,escala0, // escala de los objetos window/modelo pixeles/unidad
  amy=0,amy0, // angulo del modelo alrededor del eje y
  ac0=0,rc0; // angulo resp x y distancia al target de la camara al clickear

bool
  mini2=false,mini3=false,      // si esta minimizado
  cl_info=true,     // informa por la linea de comandos;
  luz_camara=true,  // luz fija a la camara o al espacio
  perspectiva=false, // perspectiva u ortogonal
  rota=false,       // gira continuamente los objetos respecto de y
  animado=false,    // el objeto actual es animado
  wire=false,       // dibuja lineas o no
  relleno=true,     // dibuja relleno o no
  smooth=true,      // normales por nodo o por plano
  antialias=false,  // antialiasing
  blend=false;      // transparencias

// temporizador:
const int ms_lista[]={1,2,5,10,20,50,100,200,500,1000,2000,5000},ms_n=12;
int ms_i=4,msecs=ms_lista[ms_i]; // milisegundos por frame

const double G2R=atan(1.0)/45;

// NURBS
// en u es la NURBS definida (3er grado)
// en v es 1/4 de circulo (racional, grado 2)
const float W1=(float)(sqrt(2.f)/2.f);
const int MAXPC=64;
const int uo=4,vo=3; // orden (grado+1) en u y v
p4f pc[MAXPC][3]; int npc=0; // puntos de control (npc*3)
float ukv[MAXPC+uo]; int unk=0; // knots en u
float vkv[]={0,0,0,1,1,1}; const int vnk=6; // knots en v
GLUnurbsObj *curva=0, *arco=0, *sup=0; // se crea al inicializar y se destruye al salir
int pcsel=-1; // punto seleccionado
float W0; // peso

// funciones
void check_gl_error(){
#ifdef _DEBUG
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

void salir(){
  gluDeleteNurbsRenderer(curva);
  gluDeleteNurbsRenderer(arco);
  gluDeleteNurbsRenderer(sup);
  exit(EXIT_SUCCESS);
}

//--------
// vector de knots
void set_knot_vector(){
  // NURBS 3er grado
  // condiciones de borde de Bezier => 4 iguales a 0 y 4 iguales al final
  // OpenGL usa uno de mas al ppio y al final, pero en pc estan una sola vez
  // el 1er y ultimo pc van repetidos una vez
  // los knots van desde 0 a npc-3:
  // 4: (000)01111 0(000)1111 00(001)111 000(011)11 0000(111)1 00001(111)
  // 5: (000)012222 0(000)12222 00(001)2222 000(012)222 0000(122)22 00001(222)2 000012(222)
  // 6: (000)0123333 0(000)123333 00(001)23333 000(012)3333 0000(123)333 00001(233)33 000012(333)3 0000123(333)
  // En general, orden=grado+1 (o=g+1), hay npc+o knots
  //    npc-o+2 distintos, que van de 0 a npc-o+1
  //    ser repiten o al principio (=0) y o al final (=npc-o+1)
  int i;
  if (npc<uo) {unk=0; return;}
  unk=npc+uo;
  for (i=0;i<uo-1;i++) {ukv[i]=0; ukv[unk-1-i]=npc-uo+1;} // repetidos
  for (i=0;i<npc-uo+2;i++) ukv[uo-1+i]=i; // distintos (de 0 a npc-o+1)
}

void dibuja2(){
  if (!npc) return;
  int i;

  // puntos de control
  glColor3fv(pc_c);
  glBegin(GL_POINTS);
    for (i=0;i<npc;i++) glVertex4fv(pc[i][0]);
  glEnd();
  if (npc==1) return;
  // poligono de control
  glColor3fv(lc_c); glLineWidth(1);
  glBegin(GL_LINE_STRIP);
    for (i=0;i<npc;i++) glVertex4fv(pc[i][0]);
  glEnd();
  if (npc<uo) return;

  // curva
  glColor3fv(c_c); glLineWidth(2);
  gluNurbsProperty(curva,GLU_U_STEP,float(lod));
  gluBeginCurve(curva);
//  void gluNurbsCurve( GLUnurbsObj *nurbsObj,
//                      GLint nknots, GLfloat *knot, GLint stride,
//                      GLfloat *ctlarray, GLint order, GLenum type );
    gluNurbsCurve(curva,unk,ukv,4*vo,(float*)&pc[0][0][0],uo,GL_MAP1_VERTEX_4);
  gluEndCurve(curva);
}

void dibuja3(){
  if (!npc) return;

  if (wire){
    glDisable(GL_LIGHTING);
    glDisable(GL_AUTO_NORMAL); glDisable(GL_NORMALIZE);
    // puntos y poligono de control del arco
    int i;
    glColor3fv(pc_c);
    glBegin(GL_POINTS); // puntos
    for (i=0;i<3;i++) glVertex4fv(pc[0][i]);
    glEnd();
    glColor3fv(lc_c); glLineWidth(1);
    glBegin(GL_LINE_STRIP); // poligono
    for (i=0;i<3;i++) glVertex4fv(pc[0][i]);
    glEnd();
    // arco
    glColor3fv(c_c); glLineWidth(3);
    gluNurbsProperty(arco,GLU_U_STEP,float(lod));
    gluBeginCurve(arco);
//    void gluNurbsCurve( GLUnurbsObj *nurbsObj,
//                        GLint nknots, GLfloat *knot, GLint stride,
//                        GLfloat *ctlarray, GLint order, GLenum type );
      gluNurbsCurve(arco,vnk,vkv,4,(float*)&pc[0][0][0],vo,GL_MAP1_VERTEX_4);
    gluEndCurve(arco);
    if (npc==1) return;
    glColor3fv(pc_c);
    glBegin(GL_POINTS); // puntos
    for (i=0;i<3;i++) glVertex4fv(pc[npc-1][i]);
    glEnd();
    glColor3fv(lc_c); glLineWidth(1);
    glBegin(GL_LINE_STRIP); // poligono
    for (i=0;i<3;i++) glVertex4fv(pc[npc-1][i]);
    glEnd();
    // arco
    glColor3fv(c_c); glLineWidth(3);
    gluBeginCurve(arco);
//    void gluNurbsCurve( GLUnurbsObj *nurbsObj,
//                        GLint nknots, GLfloat *knot, GLint stride,
//                        GLfloat *ctlarray, GLint order, GLenum type );
      gluNurbsCurve(arco,vnk,vkv,4,(float*)&pc[npc-1][0][0],vo,GL_MAP1_VERTEX_4);
    gluEndCurve(arco);
  }

  if (npc<uo) return;
  if (relleno){
    if (wire) glEnable (GL_POLYGON_OFFSET_FILL);
    else glDisable (GL_POLYGON_OFFSET_FILL);
    gluNurbsProperty(sup,GLU_DISPLAY_MODE,GLU_FILL);
    gluNurbsProperty(sup,GLU_U_STEP,float(lod));
    gluNurbsProperty(sup,GLU_V_STEP,float(lod));
    glEnable(GL_LIGHTING);
    glEnable(GL_AUTO_NORMAL); glEnable(GL_NORMALIZE);
    glColor3fv(c_c);
    gluBeginSurface(sup);
//    void gluNurbsSurface( GLUnurbsObj *nurbsObj,
//            GLint sknot count, GLfloat *sknot,
//            GLint tknot count, GLfloat *tknot,
//            GLint s stride, GLint t stride,
//            GLfloat *ctlarray,
//            GLint sorder, GLint torder,
//            GLenum type );
      gluNurbsSurface(sup,
          unk,ukv,vnk,vkv,4*vo,4,(float*)(&(pc[0][0][0])),uo,vo,
          GL_MAP2_VERTEX_4);
    gluEndSurface(sup);
  }
  if (wire){
    // grilla de lineas
    glDisable(GL_LIGHTING);
    glDisable(GL_NORMALIZE); glDisable(GL_AUTO_NORMAL);
    glColor4fv(lc_c); glLineWidth(1);
    gluNurbsProperty(sup,GLU_DISPLAY_MODE,GLU_OUTLINE_POLYGON);
    gluNurbsProperty(sup,GLU_U_STEP,float(lod));
    gluNurbsProperty(sup,GLU_V_STEP,float(lod));
    gluBeginSurface(sup);
//    void gluNurbsSurface( GLUnurbsObj *nurbsObj,
//            GLint sknot count, GLfloat *sknot,
//            GLint tknot count, GLfloat *tknot,
//            GLint s stride, GLint t stride,
//            GLfloat *ctlarray,
//            GLint sorder, GLint torder,
//            GLenum type );
      gluNurbsSurface(sup,
          unk,ukv,vnk,vkv,4*vo,4,(float*)(&(pc[0][0][0])),uo,vo,
          GL_MAP2_VERTEX_4);
    gluEndSurface(sup);
  }
}

//------------------------------------------------------------
// Regenera la matriz de proyeccion
// cuando cambia algun parametro de la vista
void regen3() {
  int oldWindow=glutGetWindow();
  glutSetWindow(window3);
//  if (cl_info) cout << "regen" << endl;
  if (mini3) return;

  float
      delta[3]={(target[0]-eye[0]), // vector eye->target
                (target[1]-eye[1]),
                (target[2]-eye[2])},
      dist=sqrt(delta[0]*delta[0]+  // distancia eye-targer
                delta[1]*delta[1]+
                delta[2]*delta[2]);
  float znear=1, zfar=2*dist;

  // matriz de proyeccion
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();

  // semiancho y semialto de la ventana a escala en el target
  float w0=w3/2.f/escala,h0=h3/2.f/escala;
  // frustum, perspective y ortho son respecto al eye con los z positivos
  if (perspectiva){ // perspectiva
    w0*=znear/dist,h0*=znear/dist; //w0 y h0 en el near
    glFrustum(-w0,w0,-h0,h0,znear,zfar);
  }
  else { // proyeccion ortogonal
    glOrtho(-w0,w0,-h0,h0,znear,zfar);
  }

  glMatrixMode(GL_MODELVIEW); glLoadIdentity(); // matriz del modelo

  if (luz_camara){ // luz fija a la camara
    glLightfv(GL_LIGHT0,GL_POSITION,lpos);  // ubica la luz
    gluLookAt(   eye[0],   eye[1],   eye[2],
              target[0],target[1],target[2],
                 -up[0],   -up[1],   -up[2]);// ubica la camara
    // rota los objetos alrededor de y
    glRotatef(amy,0,1,0);
  }
  else { // luz fija en el espacio
    gluLookAt(   eye[0],   eye[1],   eye[2],
              target[0],target[1],target[2],
                 -up[0],   -up[1],   -up[2]);// ubica la camara
    // rota los objetos alrededor de y
    glRotatef(amy,0,1,0);
    glLightfv(GL_LIGHT0,GL_POSITION,lpos);  // ubica la luz
  }

  glutPostRedisplay(); // avisa que hay que redibujar
  glutSetWindow(oldWindow);
}
//============================================================
// callbacks

//------------------------------------------------------------
// redibuja cada vez que hace un redisplay
void Display_cb2() {
  if (mini2) return;
  int oldWindow=glutGetWindow();
  glutSetWindow(window2);
  glClear(GL_COLOR_BUFFER_BIT); // borra el buffer de pantalla
  dibuja2();
  glutSwapBuffers();

  check_gl_error();
  glutSetWindow(oldWindow);
}

void Display_cb3() { // Este tiene que estar
  int oldWindow=glutGetWindow();
  glutSetWindow(window3);
  if (!mini3) { // dibuja
    // borra el buffer de pantalla
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    glTranslated(0,-h2/2.f,0);
    dibuja3();
    glRotatef(90.f,0.f,1.f,0.f);
    dibuja3();
    glRotatef(90.f,0.f,1.f,0.f);
    dibuja3();
    glRotatef(90.f,0.f,1.f,0.f);
    dibuja3();
    glutSwapBuffers();
    glPopMatrix();
  }
  check_gl_error();
  glutSetWindow(oldWindow);
}

//------------------------------------------------------------
// Animacion

// Si no hace nada hace esto
// glutIdleFunc lo hace a la velocidad que de la maquina
// glutTimerFunc lo hace cada tantos milisegundos

// glutTimerFunc funciona mal, hace cosas raras que son muy visibles
// cuando la espera (msecs) es grande

// El "framerate" real (cuadros por segundo)
// depende de la complejidad del modelo (lod)
void Idle_cb3() {
  static int suma,counter=0;// esto es para analisis del framerate real
  // Cuenta el lapso de tiempo
  // (int!!?? en seguida se hace >MAX_INT pero ... funciona)
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
  if (rota) { // la camara gira 1 grado alrededor de x
    double yc=eye[1]-target[1],zc=eye[2]-target[2];
    rc0=sqrt(yc*yc+zc*zc); ac0=atan2(yc,zc);
    ac0+=G2R; if (ac0>360) ac0-=360;
    yc=rc0*sin(ac0); zc=rc0*cos(ac0);
    up[1]=zc; up[2]=-yc;  // perpendicular
    eye[1]=target[1]+yc; eye[2]=target[2]+zc;
    regen3();
  }
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto del screen
void Reshape_cb2(int wi, int hi){
  w2=wi;h2=hi;

  if (!h2||!w2) {mini2=true; return;}
  mini2=false;

  glViewport(0,0,w2,h2); // region donde se dibuja

  // matriz de proyeccion
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();
  glOrtho(0,w2,h2,0,-1,1);

  // matriz del modelo constante
  glMatrixMode(GL_MODELVIEW); glLoadIdentity();

  Display_cb2();
}

void Reshape_cb3(int wi, int hi){
  w3=wi;h3=hi;

  if (!h3||!w3) {// minimiza
    mini3=true; // no dibuja mas
    glutIdleFunc(0); // no llama todo el tiempo a esa funcion
    return;
  }
  else if (mini3){// des-minimiza
    mini3=false; // ahora si dibuja
    glutIdleFunc(Idle_cb3); // registra de nuevo el callback
  }

  glViewport(0,0,w3,h3); // region donde se dibuja

  regen3(); //regenera la matriz de proyeccion
}

//------------------------------------------------------------
// Teclado y Mouse
// GLUT ACTIVE SHIFT Set if the Shift modifier or Caps Lock is active.
// GLUT ACTIVE CTRL  Set if the Ctrl modifier is active.
// GLUT ACTIVE ALT   Set if the Alt modifier is active.

short modifiers=0;  // ctrl, alt, shift (de GLUT)
inline short get_modifiers() {return modifiers=(short)glutGetModifiers();}

// Maneja pulsaciones del teclado (ASCII keys)
// x,y posicion del mouse cuando se teclea
void Keyboard_cb2(unsigned char key,int x=0,int y=0) {
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
    if (npc==0) return;
    for (int i=pcsel;i<npc-1;i++) {
      pc[i][0]=pc[i+1][0]; pc[i][1]=pc[i+1][1]; pc[i][2]=pc[i+1][2];
    }
    npc--;
    if (pcsel==npc) pcsel--;
    set_knot_vector();
    Display_cb2();
    Display_cb3();
    return;
  }
}

void Keyboard_cb3(unsigned char key,int x=0,int y=0) {
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
      break;
    case 'l': case 'L': // wire
      wire=!wire;
      if (cl_info) cout << ((wire)? "Lineas" : "Sin Lineas") << endl;
      break;
    case 'p': case 'P':  // perspectiva
      perspectiva=!perspectiva;
      if (cl_info) cout << ((perspectiva)? "Perspectiva" : "Ortogonal") << endl;
      regen3();
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
    case 27: // escape => exit
      get_modifiers();
      if (!modifiers)
        exit(EXIT_SUCCESS);
      break;
    case '+': case '-': // lod
      if (key=='+') lod++; else {lod--; if (lod==0) lod=1;}
      if (cl_info) {cout << "Nivel de detalle: "  << lod << "\r"; cout.flush();}
      break;
  }
  if (!(animado|rota)) glutIdleFunc(0); // no llama todo el tiempo a esa funcion
  else glutIdleFunc(Idle_cb3); // registra el callback
  Display_cb3();
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
void Special_cb2(int key,int xm=0,int ym=0) {
  if (key==GLUT_KEY_F4){ // alt+f4 => exit
    get_modifiers();
    if (modifiers==GLUT_ACTIVE_ALT) salir();
  }
}

void Special_cb3(int key,int xm=0,int ym=0) {
  if (key==GLUT_KEY_F4){ // alt+f4 => exit
    get_modifiers();
    if (modifiers==GLUT_ACTIVE_ALT)
      salir();
  }
  if (key==GLUT_KEY_UP||key==GLUT_KEY_DOWN){ // camara
    // la camara gira alrededor del eje -x
    double yc=eye[1]-target[1],zc=eye[2]-target[2],
           rc=sqrt(yc*yc+zc*zc),ac=atan2(yc,zc);
    ac+=((key==GLUT_KEY_UP) ? -2 : 2)*G2R;
    yc=rc*sin(ac); zc=rc*cos(ac);
    up[1]=zc; up[2]=-yc;  // perpendicular
    eye[1]=target[1]+yc; eye[2]=target[2]+zc;
    regen3();
  }
  if (key==GLUT_KEY_LEFT){ // gira
    amy+=1;
    regen3();
  }
  if (key==GLUT_KEY_RIGHT){ // gira
    amy-=1;
    regen3();
  }
  if (key==GLUT_KEY_PAGE_UP||key==GLUT_KEY_PAGE_DOWN){ // velocidad
    if (key==GLUT_KEY_PAGE_DOWN) ms_i++;
    else ms_i--;
    if (ms_i<0) ms_i=0; if (ms_i==ms_n) ms_i--;
    msecs=ms_lista[ms_i];
    if (cl_info){
      if (msecs<1000)
        {cout << 1000/msecs << "fps" << "\r"; cout.flush();}
      else
        {cout << msecs/1000 << "s/frame)" << "\r"; cout.flush();}
    }
  }
}

// Movimientos del mouse
void Motion_cb2(int x, int y){ // drag
  float W;
  if (modifiers==GLUT_ACTIVE_CTRL){ // cambia el peso
    W=W0*pow(10.0,int(20.0*4.0*(yclick-y)/h2)/20.0);
    if (pcsel>=0&&pcsel<npc) {
      pc[pcsel][0].peso(W);
      pc[pcsel][1].peso(W*W1);
      pc[pcsel][2].peso(W);
    }
    if (cl_info) cout << "\rPeso: "<< W << "             " << flush;
  }
  else{
    // permite drag solo dentro del viewport
    // en caso contrario, sin zoom, no se pueden editar los puntos negativos
    if (x<0) x=0; else if (x>w2-1) x=w2-1;
    if (y<0) y=0; else if (y>h2-1) y=h2-1;
    p4f pm(x,y);
    // si se acerco a otro lo pega
    int i;
    for (i=0;i<npc;i++){
      if (i==pcsel) continue;
      if (!pm.cerca2D(pc[i][0],tol)) continue;
      pm=pc[i][0];
      break;
    }
    if (pm[0]==0) pm[0]=.001;// no admite x=0 porque se ve mal(probar)
    // mueve
    i=pcsel;
    W=pc[i][0][3]; pm.peso(W);
    pc[i][0]=pc[i][1]=pc[i][2]=pm;
    pc[i][1][2]=pc[i][1][0]; pc[i][1].peso(W*W1); // x=z, peso=raiz(2)/2
    pc[i][2][2]=pc[i][2][0]; pc[i][2][0]=0; // x <-> z
  }
  Display_cb2();
  Display_cb3();
}

void Motion_cb3(int x, int y){ // drag
  if (modifiers==GLUT_ACTIVE_SHIFT){ // cambio de escala
    escala=escala0*exp((yclick-y)/100.0);
    regen3();
//    if (cl_info) {cout << "Escala:" << escala  << " znear:" << znear << " zfar:" << zfar << '\r'; cout.flush();}
  }
  //==============================================================
  // esto es para experimentar, se puede borrar o comentar (o dejar)
  else if (modifiers==(GLUT_ACTIVE_ALT|GLUT_ACTIVE_CTRL)){ // cambio en polygon offset
    // viendo lineas y superficie
    // con lod muy chico se ve el efecto y con lod grande el problema
    // (al menos en mi maquina) el 2do numero no afecta en nada
    // y GL_POLYGON_OFFSET_LINE no hace nada
    // EN PESPECTIVA ES DISTINTO DE ORTOGONAL!!
    glPolygonOffset(((yclick-y)/10.0),((xclick-x)/10.0));
    if (cl_info) {cout << "Offset:" << (yclick-y)/10.0 << ", " << (xclick-x)/10.0 << '\r'; cout.flush();}
    glutPostRedisplay();
  }
  //==============================================================
  else { // manipulacion
    double yc=eye[1]-target[1],zc=eye[2]-target[2];
    double ac=ac0+(yclick-y)*720.0*G2R/h3;
    yc=rc0*sin(ac); zc=rc0*cos(ac);
    up[1]=zc; up[2]=-yc;  // perpendicular
    eye[1]=target[1]+yc; eye[2]=target[2]+zc;
    amy=amy0+(xclick-x)*180.0/w3;
    regen3();
  }
}


// Clicks del mouse
// GLUT LEFT BUTTON, GLUT MIDDLE BUTTON, or GLUT RIGHT BUTTON
// The state parameter is either GLUT UP or GLUT DOWN
// glutGetModifiers may be called to determine the state of modifier keys
void Mouse_cb2(int button, int state, int x, int y){
  if (button==GLUT_LEFT_BUTTON){
    int i;
    if (state==GLUT_DOWN) {
      p4f pm(x,y);
      // verifica tolerancia
      for(i=0;i<npc;i++){
        if (!pm.cerca2D(pc[i][0],tol)) continue;
        pcsel=i;
        glutMotionFunc(Motion_cb2);
        return;
      }
      // no pico cerca de ninguno
      get_modifiers();
      if (modifiers==GLUT_ACTIVE_CTRL){ // cambia el peso
        yclick=y; W0=pc[pcsel][0][3];
        glutMotionFunc(Motion_cb2);
        return;
      }
      if (npc==MAXPC) return;
      // agrega entre medio
      for (i=npc;i>pcsel+1;i--) {
        pc[i][0]=pc[i-1][0]; pc[i][1]=pc[i-1][1]; pc[i][2]=pc[i-1][2];
      }
      pc[i][0]=pc[i][1]=pc[i][2]=pm;
      pc[i][1][2]=pc[i][1][0]; pc[i][1].peso(W1); // x=z, peso=raiz(2)/2
      pc[i][2][2]=pc[i][2][0]; pc[i][2][0]=0; // x <-> z
      pcsel=i;
      npc++;
      set_knot_vector();
      Display_cb2();
      Display_cb3();
      glutMotionFunc(Motion_cb2);
      return;
    } // down
    else if (state==GLUT_UP){// fin del drag
      glutMotionFunc(0); // anula el callback para los drags
      return;
    } // up
  } // left
}

void Mouse_cb3(int button, int state, int x, int y){
  static bool old_rota=false;
  if (button==GLUT_LEFT_BUTTON){
    if (state==GLUT_DOWN) {
      xclick=x; yclick=y;
      old_rota=rota; rota=false;
      get_modifiers();
      glutMotionFunc(Motion_cb3); // callback para los drags
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
      glutMotionFunc(0); // anula el callback para los drags
    }
  }
}

//------------------------------------------------------------
// Menu
void Menu_cb2(int value)
{
  switch (value){
    case 127: // del = borra
      Keyboard_cb2(127);
      return;
    case 27: // esc = exit
      salir();
  }
}
void Menu_cb3(int value)
{
  switch (value){
    case 'a':
      Keyboard_cb3('a');
      return;
    case 'f':
      Keyboard_cb3('f');
      return;
    case 'i':
      Keyboard_cb3('i');
      return;
    case 'j':
      Keyboard_cb3('j');
      return;
    case 'l':
      Keyboard_cb3('l');
      return;
    case 'p':
      Keyboard_cb3('p');
      return;
    case 'r':
      Keyboard_cb3('r');
      return;
    case 's':
      Keyboard_cb3('s');
      return;
    case 't':
      Keyboard_cb3('t');
      return;
    case '+':
      Keyboard_cb3('+');
      return;
    case '-':
      Keyboard_cb3('-');
      return;
    case (256+GLUT_KEY_UP):
      Special_cb3(GLUT_KEY_UP);
      return;
    case (256+GLUT_KEY_DOWN):
      Special_cb3(GLUT_KEY_DOWN);
      return;
    case (256+GLUT_KEY_LEFT):
      Special_cb3(GLUT_KEY_LEFT);
      return;
    case (256+GLUT_KEY_RIGHT):
      Special_cb3(GLUT_KEY_RIGHT);
      return;
    case (256+GLUT_KEY_PAGE_UP):
      Special_cb3(GLUT_KEY_PAGE_UP);
      return;
    case (256+GLUT_KEY_PAGE_DOWN):
      Special_cb3(GLUT_KEY_PAGE_DOWN);
      return;
    case 27: //esc
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

// inicializa la ventana para dibujo de la curva
void ini_2D_window() {
  glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_ALPHA);
  glutInitWindowSize(200,500); glutInitWindowPosition(50,50);

  glutCreateWindow("Curva NURBS"); // crea el main window
  window2=glutGetWindow();

  //declara los callbacks
  //los que no se usan no se declaran
  glutDisplayFunc(Display_cb2); // redisplays
  glutReshapeFunc(Reshape_cb2); // resize
  glutKeyboardFunc(Keyboard_cb2); // teclado
  glutSpecialFunc(Special_cb2); // ctrl alt y shift
  glutMouseFunc(Mouse_cb2); // clicks

  glutCreateMenu(Menu_cb2);
    glutAddMenuEntry("[Del] Borra", 127);
    glutAddMenuEntry("---------------------------", 255);
    glutAddMenuEntry("[i] Info ON/OFF", 'i');
    glutAddMenuEntry("[Esc] Exit", 27);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  // ========================
  // estado normal del OpenGL
  // ========================
  glClearColor(fondo[0],fondo[1],fondo[2],1);  // color de fondo
  glPointSize(5);
  glEnable(GL_POINT_SMOOTH); glEnable(GL_LINE_SMOOTH); //antialias

  // propiedades de la NURBS
  curva=gluNewNurbsRenderer();
  gluNurbsProperty(curva,GLU_SAMPLING_METHOD,GLU_DOMAIN_DISTANCE); // para usar lod
  gluNurbsProperty(curva,GLU_U_STEP,float(lod));
  gluNurbsProperty(curva,GLU_DISPLAY_MODE,GLU_FILL);
}

// inicializa la ventana para dibujo de la superficie
void ini_3D_window() {

  // pide color RGB y double buffering
  glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH|GLUT_ALPHA);
  glutInitWindowSize(500,500); glutInitWindowPosition(350,50);
  glutCreateWindow("Superficie NURBS de Revolucion"); // crea el main window
  window3=glutGetWindow();

  //declara los callbacks
  //los que no se usan no se declaran
  glutDisplayFunc(Display_cb3); // redisplays
  glutReshapeFunc(Reshape_cb3); // resize
  glutKeyboardFunc(Keyboard_cb3); // teclado
  glutSpecialFunc(Special_cb3); // ctrl alt y shift
  glutMouseFunc(Mouse_cb3); // clicks
  glutMotionFunc(Motion_cb3);
  if (!(animado|rota)) glutIdleFunc(0); // no llama todo el tiempo a esa funcion
  else glutIdleFunc(Idle_cb3); // registra el callback

  // menu
  glutCreateMenu(Menu_cb3);
    glutAddMenuEntry("     [p]_Perspectiva/Ortogonal ", 'p');
    glutAddMenuEntry("     [r]_Rota                  ", 'r');
    glutAddMenuEntry("     [+]_+Nivel de Detalle     ", '+');
    glutAddMenuEntry("     [-]_-Nivel de Detalle     ", '-');
    glutAddMenuEntry("     [l]_Lineas                ", 'l');
    glutAddMenuEntry("     [f]_Caras Rellenas        ", 'f');
    glutAddMenuEntry("     [s]_Suave/Facetado        ", 's');
    glutAddMenuEntry("     [a]_Antialiasing          ", 'a');
    glutAddMenuEntry("     [t]_Transparencia         ", 't');
    glutAddMenuEntry("    [Up]_Sube Camara           ", (256+GLUT_KEY_UP));
    glutAddMenuEntry("  [Down]_Baja Camara           ", (256+GLUT_KEY_DOWN));
    glutAddMenuEntry("  [Left]_Gira objeto           ", (256+GLUT_KEY_LEFT));
    glutAddMenuEntry(" [Right]_Gira objeto           ", (256+GLUT_KEY_RIGHT));
    glutAddMenuEntry("  [PgUp]_Aumenta Framerate     ", (256+GLUT_KEY_PAGE_UP));
    glutAddMenuEntry("  [Pgdn]_Disminuye Framerate   ", (256+GLUT_KEY_PAGE_DOWN));
    glutAddMenuEntry("     [j]_Luz fija ON/OFF       ", 'j');
    glutAddMenuEntry("     [i]_Info ON/OFF           ", 'i');
    glutAddMenuEntry("   [Esc]_Exit                  ", 27);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  // ========================
  // estado normal del OpenGL
  // ========================
  glClearColor(fondo[0],fondo[1],fondo[2],1);  // color de fondo

  glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); // habilita el z-buffer
  glEnable(GL_NORMALIZE); // para que el scaling no moleste
  glDisable(GL_AUTO_NORMAL); // para nurbs
  glPolygonOffset (1, 1); // coplanaridad

  glPointSize(5);

  // interpola normales por nodos o una normal por plano
  glShadeModel((smooth) ? GL_SMOOTH : GL_FLAT);

  // antialiasing
  if (antialias){
    glEnable(GL_POINT_SMOOTH); glEnable(GL_LINE_SMOOTH); glEnable(GL_POLYGON_SMOOTH);
  }
  else {
    glDisable(GL_POINT_SMOOTH); glDisable(GL_LINE_SMOOTH); glDisable(GL_POLYGON_SMOOTH);
  }

  // transparencias
  if (blend) glEnable(GL_BLEND);
  else glDisable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  glFrontFace(GL_CCW); // direccion de los poligonos
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

  // define luces
  glLightfv(GL_LIGHT0,GL_AMBIENT,lambient);
  glLightfv(GL_LIGHT0,GL_DIFFUSE,ldiffuse);
  // caras de atras y adelante distintos (1) o iguales (0)
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,0);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);

  // material estandar
  const float
    fambient[]={0.2f,0.09f,0.03f,.5f},
    fdiffuse[]={0.5f,0.2f,0.07f,.5f},
    fspecular[]={0.6f,0.2f,0.07f,.5f},
    fshininess=50.0f,
    bambient[]={0.1f,0.2f,0.2f,0.5f},
    bdiffuse[]={0.5f,0.9f,0.6f,.5f},
    bspecular[]={0.3f,0.3f,0.3f,.5f},
    bshininess=10.0f;
  glMaterialfv(GL_FRONT,GL_AMBIENT,fambient);
  glMaterialfv(GL_FRONT,GL_DIFFUSE,fdiffuse);
  glMaterialfv(GL_FRONT,GL_SPECULAR,fspecular);
  glMaterialf(GL_FRONT,GL_SHININESS,fshininess);
  glMaterialfv(GL_BACK,GL_AMBIENT,bambient);
  glMaterialfv(GL_BACK,GL_DIFFUSE,bdiffuse);
  glMaterialfv(GL_BACK,GL_SPECULAR,bspecular);
  glMaterialf(GL_BACK,GL_SHININESS,bshininess);

  // propiedades de las NURBS
  arco=gluNewNurbsRenderer();
  gluNurbsProperty(arco,GLU_SAMPLING_METHOD,GLU_DOMAIN_DISTANCE); // para usar lod
  gluNurbsProperty(arco,GLU_DISPLAY_MODE,GLU_FILL);

  sup=gluNewNurbsRenderer();
  gluNurbsProperty(sup,GLU_SAMPLING_METHOD,GLU_DOMAIN_DISTANCE); // para usar lod
//  gluNurbsProperty(sup,GLU_CULLING,GL_TRUE); // si esta fuera del viewport

}

void initialize() {

  // ========================
  // info
  if (cl_info){
      cout << "\n\n\nPuntos de control:\n"
         << endl;
      cout << "\n\n\nUso del mouse:\n\n"
         << " Curva:\n"
         << "       Click izq -> Selecciona (tol. 10) o\n"
            "                    Agrega despues del seleccionado\n"
         << "        Drag izq -> Mueve el seleccionado\n"
         << "           <DEL> -> Borra el seleccionado\n"
         << "   Ctrl+Drag izq -> Cambia el peso del seleccionado\n\n"
         << " Superficie:\n"
         << "   <Shift>+Drag izq -> Zoom\n"
         << "           Drag izq -> Manipula\n\n"
         << endl;
  }

  // ========================
  ini_3D_window();
  ini_2D_window();
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv);// inicializa glut
  initialize(); // condiciones iniciales de la ventana y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0; //solo para que el builder se quede tranquilo (aqui nunca llega)
}
