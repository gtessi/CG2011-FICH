#include <iostream> // cin, cout
#include <cstdlib> // exit
#include <cmath> // fabs sqrt
#include <cstring> // memcpy
#include <GL/glut.h>

using namespace std;

///////////////////////////////////////////////////////////
// clase: puntos de cuatro floats
// MANEJESE CON CUIDADO!!!
// 4 para poder hacer racionales y porque GLU pide 3D o 4D
// floats porque GLU pide floats
// tiene solo lo necesario para usarlos aca
// z siempre es 0
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

  float& operator[](int i) {return c[i];}
  const float& operator[](int i) const {return c[i];}

  // modifica el peso pero mantiene el punto en el mismo lugar de R3
  // ==> OJO: no admite peso 0 !!!!!!!!!!!!!!!!!
  // (para hacer peso=0 poner p[3]=0 y ya esta)
  p4f& peso(float w){
    if (fabs(w)<1e-7) {if (w<0) w=-1e-7; else w=1e-7;}
    float f=w/c[3];
    c[0]*=f; c[1]*=f; c[2]*=f; c[3]=w; return *this;
  }

  bool cerca2D(const p4f &p, float r=1){
    float cw=c[3],pw=p[3];
    return (fabs(c[0]/cw-p[0]/pw)<=r && fabs(c[1]/cw-p[1]/pw)<=r);
  }

  p4f& neg(){c[0]=-c[0]; c[1]=-c[1]; c[2]=-c[2]; c[3]=-c[3]; return *this;}
  p4f operator -() const {return p4f(-c[0],-c[1],-c[2],-c[3]);}

  // asigna el punto interpolado
  p4f& lerp(const p4f& p0,const p4f& p1,float u){
    c[0]=(1-u)*p0[0]+u*p1[0];
    c[1]=(1-u)*p0[1]+u*p1[1];
    c[2]=(1-u)*p0[2]+u*p1[2];
    c[3]=(1-u)*p0[3]+u*p1[3];
    return *this;
  }

  float mod() const {
    if (c[3]!=0) return sqrt(c[0]*c[0]+c[1]*c[1]+c[2]*c[2])/c[3]; // punto: distancia al origen
    else return sqrt(c[0]*c[0]+c[1]*c[1]+c[2]*c[2]); // vector: modulo
  }
  p4f& dir() {return operator/=(mod());}

  p4f operator -(const p4f &p) const{
    return p4f(c[0]-p[0],c[1]-p[1],c[2]-p[2],c[3]-p[3]);
  }
  p4f& operator -=(const p4f &p) {
    c[0]-=p[0];c[1]-=p[1];c[2]-=p[2];c[3]-=p[3]; return *this;
  }
  p4f operator +(const p4f &p) const{
    return p4f(c[0]+p[0],c[1]+p[1],c[2]+p[2],c[3]+p[3]);
  }
  p4f& operator +=(const p4f &p) {
    c[0]+=p[0];c[1]+=p[1];c[2]+=p[2];c[3]+=p[3]; return *this;
  }
  p4f operator *(float f) const{
    return p4f(c[0]*f,c[1]*f,c[2]*f,c[3]*f);
  }
  p4f &operator *=(float f) {
    c[0]*=f;c[1]*=f;c[2]*=f;c[3]*=f; return *this;
  }
  friend p4f operator*(float f,const p4f &p) {return (p*f);}
  p4f operator /(float f) const{
    return p4f(c[0]/f,c[1]/f,c[2]/f,c[3]/f);
  }
  p4f &operator /=(float f) {
    c[0]/=f;c[1]/=f;c[2]/=f;c[3]/=f; return *this;
  }
  friend p4f operator/(float f,const p4f &p) {return (p/f);}
};
///////////////////////////////////////////////////////////

// variables
int
  w,h, // alto y ancho de la pantalla
  tol=10, // tolerancia para seleccionar un punto
  yclick, // y picado
  lod=32; // nivel de detalle (subdivisiones de lineas)

float // colores
  fondo[]={0.95f,0.98f,1.0f}, // color de fondo
  lc_c[]={.4f,.4f,.6f},       // poligono de control
  pc_c[]={.2f,.2f,.4f},       // puntos de control
  c_c[]={.8f,.4f,.4f},        // curva
  la_c[]={.6f,.8f,.6f},       // lineas accesorias
  p_c[]={1.f,0.f,0.f};        // punto de la curva

bool
  mini=false,      // si esta minimizado
  antialias=true;  // antialiasing

// Interpolacion con Bezier
// Hay npc puntos de control, los interpolados y los invisibles
// El primero y el ultimo interpolan ==> :
//  segundo grado entre el primero, el primer invisible y el segundo
//  tercer grado en el medio
//  segundo grado entre el penultimo, el ultimo invisible y el ultimo
// El primero no tiene invisible asociado (usa el del segundo) y lo mismo para el ultimo
// Se complica la aritmetica de cuales (i??) hay que recalcular por cambios
int npc=0; const int MAXPC=252; p4f pc[255];  // puntos de control
int pcsel=-3; // punto seleccionado
int metodo=0,metodos=5; // 0=Catmull-Rohm 1=Overhausser .....

// funciones
// recalcula (lo que sea necesario) al modificar un punto
// 0 (1) 2 (3) (4) 5 (6) (7) 8 ...
// (i-4) i-3 (i-2) (i-1) i (i+1) (i+2) i+3 (i+4)
// ... (n-4) n-3 (n-2) n-1
// primario = llamado desde fuera, debe recalcular vecinos
// no primario = llamado por recursión desde aqui mismo
void recalc(int i,bool primario=true){
  if (npc<=2) return;
  if (i==0){ // primer interpolado
    if (primario) recalc(2,false); // recalcula los asociados al 2do interpolado
    return;
  }
  if (i==npc-1){
    if (primario) recalc(npc-3,false); // recalcula los asociados al penultimo interpolado
    return;
  }

  int iant=i-3; if (i==2) iant=0;
  int ipos=i+3; if (i==npc-3) ipos=npc-1;
  p4f l;
  p4f vant=pc[i]-pc[iant], vpos=pc[ipos]-pc[i];
  float dant=(vant).mod(),dpos=(vpos).mod();
  if (dant<1e-6) dant=1e-6; if (dpos<1e-6) dpos=1e-6; 
  if (metodo==0){ // Catmull-Rohm
    // direccion que une anterior y posterior 
    // derivada = distancia anterior a posterior  /2/grado
    // derivadas iguales a ambos lados
    l=(vant+vpos)/2;
    if (i!=2)     pc[i-1]=pc[i]-l/3; else pc[i-1]=pc[i]-l/2; 
    if (i!=npc-3) pc[i+1]=pc[i]+l/3; else pc[i+1]=pc[i]+l/2; 
  }
  else if (metodo==1){ // overhauser chord-length parameterization
    // direccion proporcional a la cercania
    // derivada media = distancia anterior a posterior /2/3
    // derivadas proporcionales a las distancias
    //@@@@ Overhauser:
    //  1) calcular vectores diferencia y sus modulos (distancias)
    //  2) dirección en proporcion inversa a las distancias (apunta mas al mas cercano)
    //  3) puntos invisibles en proporcion directa con las distancias (mas corto el mas cercano)
	//------------------------------------------------------------------------------------------
	// vectores "velocidad" v sub i-1/2 (vmen) y v sub i+1/2 (vmay)
	// v sub i+1/2 = p sub i+1 - p sub i (vector)
	//              -------------------------------
	//               u sub i+1 - u sub i (módulo)
	p4f vmen=vant/dant, // vmen es v sub i-1/2
		vmay=vpos/dpos, // vmay es v sub i+1/2
	//------------------------------------------------------------------------------------------
	// v sub i = (u sub i+1 - u sub i) * v sub i-1/2 + (u sub i - u sub i-1) * v sub i+1/2
	//           ---------------------------------------------------------------------------
	//                                     u sub i+1 - u sub i-1
		vi=(dpos*vmen+dant*vmay)/(dpos+dant); // vi es la velocidad ponderada en el punto interpolado i
	//------------------------------------------------------------------------------------------
	// ahora vienen las correcciones para dibujar la curva, a partir
	// del vector vi y los módulos dant o dpos, según correspondan
	// -vi*dant, +vi*dpos
	if (i!=2)     pc[i-1]=pc[i]-vi*dant/3; else pc[i-1]=pc[i]-vi*dant/2; 
	if (i!=npc-3) pc[i+1]=pc[i]+vi*dpos/3; else pc[i+1]=pc[i]+vi*dpos/2; 
  }
  //@@@@ aca implementar algun(os) otros modo(s) de evitar el overshooting
  else if (metodo==2){ // overhauser centripetal parameterization
	// idem overhauser, pero cambia la parametrización
	// usando las raíces cuadradas de los módulos
	float rdant=sqrt(dant),
		  rdpos=sqrt(dpos);
	p4f vmen=vant/rdant, // vmen es v sub i-1/2
		vmay=vpos/rdpos, // vmay es v sub i+1/2	  
		vi=(rdpos*vmen+rdant*vmay)/(rdant+rdpos); // vi es la velocidad ponderada en el punto interpolado i
	if (i!=2)     pc[i-1]=pc[i]-vi*rdant/3; else pc[i-1]=pc[i]-vi*rdant/2; 
	if (i!=npc-3) pc[i+1]=pc[i]+vi*rdpos/3; else pc[i+1]=pc[i]+vi*rdpos/2;
  }
  else if (metodo==3){ // Tension–Continuity–Bias splines
	// éste método ajusta la curva a partir de tres nuevos
	// parámetros, que son tensión, continuidad y tendencia
	//------------------------------------------------------------------------------------------
	// tensión controla lo estrecho de la curva
	// continuidad controla la (dis)continuidad de la primer derivada
	// tendencia controla el efecto de overshooting/undershooting
	//------------------------------------------------------------------------------------------
	// TCB ajusta los puntos de control de Catmull-Rom de acuerdo
	// a los tres nuevos parámetros, con la notación de las primeras
	// derivadas derecha e izquierda
	//------------------------------------------------------------------------------------------
	// D sub q- sub i =      lim        q(u sub i) - q(u)      = 3 * (p sub i - p- sub i)
	//                    u->u- sub i  -----------------------
	//                                      u sub i - u
	// D sub q+ sub i =      lim        q(u) - q(u sub i)      = 3 * (p+ sub i - p sub i)
	//                    u->u+ sub i  -----------------------
	//                                      u - u sub i
	//------------------------------------------------------------------------------------------
	// despejando, D sub q- sub i = D sub q+ sub i = 1/2 * v sub i-1/2 + 1/2 * v sub i+1/2
	// ***hay que aplicar los parametros al parentesis anterior
	  p4f vmen=vant/dant, // vmen es v sub i-1/2
		  vmay=vpos/dpos; // vmay es v sub i+1/2
  }
  
  if (primario){
    recalc(iant,false);
    recalc(ipos,false);
  }
}

void recalc_all(){
  for (int i=2;i<=npc-3;i+=3) recalc(i,false);
}

void check_gl_error(){
  int errornum=glGetError();
  while(errornum!=GL_NO_ERROR){
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
    errornum=glGetError();
  }
}

void salir(){
  exit(EXIT_SUCCESS);
}

//--------
void dibuja(){
  if (!npc) return;

  int i;

  // puntos de control
  glPointSize(5); glColor3fv(p_c);
  glBegin(GL_POINTS);
    for (i=2;i<=npc-3;i+=3) glVertex4fv(pc[i]);
    glVertex4fv(pc[0]); glVertex4fv(pc[npc-1]);
  glEnd();

  glPointSize(1); glColor3fv(pc_c); 
  glBegin(GL_POINTS);
    for (i=2;i<=npc-3;i+=3) {
      glVertex4fv(pc[i-1]); glVertex4fv(pc[i+1]);
    }
  glEnd();


  // dibuja la curva
  if (npc==1) return; // hay un solo punto
  glColor3fv(c_c); glLineWidth(2);
  if (npc==2){ // hay dos puntos => una linea
    glBegin(GL_LINES);
      glVertex4fv(pc[0]);
      glVertex4fv(pc[1]);
    glEnd();
    return;
  }
  // curvas de Bezier de grado 2 al principio y final
  glEnable(GL_MAP1_VERTEX_4);
  glMap1f(GL_MAP1_VERTEX_4, 0.0, 1.0, 4, 3, (const float*)(&pc[0]));
  glMapGrid1f(lod,0,1); glEvalMesh1(GL_LINE,0,lod);
  check_gl_error();
  glMap1f(GL_MAP1_VERTEX_4, 0.0, 1.0, 4, 3, (const float*)(&pc[npc-3]));
  glMapGrid1f(lod,0,1); glEvalMesh1(GL_LINE,0,lod);
  check_gl_error();
  // curvas de Bezier de grado 3
  for (i=2;i<npc-3;i+=3){
	  glMap1f(GL_MAP1_VERTEX_4, 0.0, 1.0, 4, 4, (const float*)(&pc[i]));
	  glMapGrid1f(lod,0,1); glEvalMesh1(GL_LINE,0,lod);
	  check_gl_error();
  }
}

//============================================================
// callbacks

//------------------------------------------------------------
// redibuja cada vez que hace un redisplay
void Display_cb() { // Este tiene que estar
  if (mini) return;

  glClear(GL_COLOR_BUFFER_BIT);  // borra el buffer de pantalla
  dibuja();
  glutSwapBuffers();

  check_gl_error();
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto del screen
void Reshape_cb(int wi, int hi){
  w=wi;h=hi;

  if (!h||!w) {mini=true; return;}
  mini=false;

  glViewport(0,0,w,h); // region donde se dibuja

  // matriz de proyeccion
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();
  glOrtho(-w/2,w-w/2,h-h/2,-h/2,-1,1);

  glutPostRedisplay();
}

//------------------------------------------------------------
// Teclado y Mouse
// GLUT ACTIVE SHIFT Set if the Shift modifier or Caps Lock is active.
// GLUT ACTIVE CTRL  Set if the Ctrl modifier is active.
// GLUT ACTIVE ALT   Set if the Alt modifier is active.

short modifiers=0;  // ctrl, alt, shift (de GLUT)
inline short get_modifiers() {return modifiers=(short)glutGetModifiers();}

static void info(){
  cout << " - Metodo: " << metodo << "              \r";
  cout.flush();
}

// Maneja pulsaciones del teclado (ASCII keys)
// x,y posicion del mouse cuando se teclea
void Keyboard_cb(unsigned char key,int x=0,int y=0) {
  if (key==27){ // escape => exit
    get_modifiers();
    if (!modifiers) salir();
  }
  else if (key=='a'||key=='A'){ // Antialiasing
    antialias=!antialias;
    if (antialias){
      glEnable(GL_POINT_SMOOTH); glEnable(GL_LINE_SMOOTH);
      cout << "Antialiasing" << endl;
    }
    else {
      glDisable(GL_POINT_SMOOTH); glDisable(GL_LINE_SMOOTH);
      cout << "Sin Antialiasing" << endl;
    }
  }
  else if (key=='b'||key=='B'){ // Balance
    if (key=='B') metodo+=metodos-1; else  metodo++; 
    metodo%=metodos;
    info();
    recalc_all();
  }
  else if (key=='+'){ // lod
    lod++;
    cout << "Nivel de Detalle: " << lod  << "                          \r";
    cout.flush();
  }
  else if (key=='-'){ // lod
    if (lod==2) return;
    lod--;
    cout << "Nivel de Detalle: " << lod  << "                        \r";
    cout.flush();
  }
  else if (key==127){ // del => borra pt
    // @@@@ esta es la rutina mas complicada! analizar
    if (npc==0) return;
    if (pcsel==0) { // borra el primero
      if (npc==1) npc=0;
      else if (npc==2) {pc[0]=pc[1]; npc=1;}
      else { // desaparecen tambien el 1 y el 3
        npc-=3; pc[0]=pc[2];
        for (int i=1;i<npc;i++) pc[i]=pc[i+3];
      }
    }
    else if (pcsel==npc-1) { // borra el ultimo
      if (npc==2) npc=1;
      else { // desaparecen tambien el npc-2 y el npc-4
        pc[npc-4]=pc[npc-3]; npc-=3;
      }
      pcsel=npc-1;
    }
    else { // desaparecen tres intermedios: el anterior, este y el posterior
      npc-=3;
      for (int i=pcsel-1;i<npc;i++) pc[i]=pc[i+3];
      if (npc==2) pcsel=1; else if (pcsel!=2) pcsel-=3;
    }
    recalc(pcsel);
  }
  glutPostRedisplay();
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

// Menu
void Menu_cb(int value)
{
  switch (value){
    case 'a':
      Keyboard_cb('a');
      return;
    case 'b':
      Keyboard_cb('b');
      return;
    case 'd':
      Keyboard_cb('d');
      return;
    case 'g':
      Keyboard_cb('g');
      return;
    case '+':
      Keyboard_cb('+');
      return;
    case '-':
      Keyboard_cb('-');
      return;
    case 'i':
      Keyboard_cb('i');
      return;
    case 127: // del = borra
      Keyboard_cb(127);
      return;
    case 27: // esc = exit
      salir();
    }
}
//------------------------------------------------------------

// Movimientos del mouse
void Motion_cb(int x, int y){ // drag
  x-=w/2;y-=h/2;

  // mueve el punto
  // permite drag solo dentro del viewport
  // en caso contrario, sin zoom, no se pueden editar los que esten fuera
  if (x<-w/2) x=-w/2; else if (x>w-w/2-1) x=w-w/2-1;
  if (y<-h/2) y=-h/2; else if (y>h-h/2-1) y=h-h/2-1;
  p4f pm(x,y);
  // evita que se pegue con los vecinos inmediatos
  // posterior
  if (pcsel==0&&pm.cerca2D(pc[2],1)) return;
  else if (pcsel==npc-3&&pm.cerca2D(pc[npc-1],1)) return;
  else if (pcsel<npc-3&&pm.cerca2D(pc[pcsel+3],1)) return;
  // anterior
  if (pcsel==npc-1&&pm.cerca2D(pc[npc-3],1)) return;
  else if (pcsel==2&&pm.cerca2D(pc[0],1)) return;
  else if (pcsel>2&&pm.cerca2D(pc[pcsel-3],1)) return;

  pc[pcsel]=pm; recalc(pcsel);  // mueve y recalcula

  glutPostRedisplay();
}

// Clicks del mouse
// GLUT LEFT BUTTON, GLUT MIDDLE BUTTON, or GLUT RIGHT BUTTON
// The state parameter is either GLUT UP or GLUT DOWN
// glutGetModifiers may be called to determine the state of modifier keys
void Mouse_cb(int button, int state, int x, int y){
  x-=w/2;y-=h/2;
  if (button==GLUT_LEFT_BUTTON){
    if (state==GLUT_DOWN) {
      p4f pm(x,y);
      int i,cercano=-1;
      // verifica tolerancia
      if (pm.cerca2D(pc[0],tol)) cercano=0;
      else if (pm.cerca2D(pc[npc-1],tol)) cercano=npc-1;
      else for(i=2;i<npc;i+=3) if (pm.cerca2D(pc[i],tol)) {cercano=i; break;}
      if (cercano!=-1) {pcsel=cercano; glutMotionFunc(Motion_cb); return;}
      // no pico cerca de ninguno => agrega
      if (npc>=MAXPC-3) return;
      if (npc==0) {npc=1; pcsel=0;} // primero
      else if (npc==1) {npc=2; pcsel=1;} // segundo
      else { // cualquier otro
        npc+=3; // hace lugar para tres mas
        if (pcsel==npc-4) pc[pcsel+1]=pc[pcsel];// despues del ultimo
        else for (i=npc-1;i>pcsel+1;i--) pc[i]=pc[i-3];
        if (pcsel==0) pcsel=2; else pcsel+=3;
      }
      pc[pcsel]=pm; if (pcsel==npc-1) recalc(pcsel-2); else recalc(pcsel);
	  glutPostRedisplay();
      glutMotionFunc(Motion_cb);
    } // down
    else if (state==GLUT_UP){// fin del drag
      glutMotionFunc(0); // anula el callback para los drags
      modifiers=0;
    } // up
  } // left
}

//------------------------------------------------------------
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

  glutInitWindowSize(640,480); glutInitWindowPosition(250,100);

  glutCreateWindow("Interpolación con splines"); // crea el main window

  //declara los callbacks
  //los que no se usan no se declaran
  glutDisplayFunc(Display_cb); // redisplays
  glutReshapeFunc(Reshape_cb); // resize
  glutKeyboardFunc(Keyboard_cb); // teclado
  glutSpecialFunc(Special_cb); // ctrl alt y shift
  glutMouseFunc(Mouse_cb); // clicks

  glutCreateMenu(Menu_cb);
    glutAddMenuEntry("[Del] Borra Punto de Control", 127);
    glutAddMenuEntry("[+]   +Nivel de Detalle       ", '+');
    glutAddMenuEntry("[-]   -Nivel de Detalle       ", '-');
    glutAddMenuEntry("[b/B] Metodo de Balanceado    ", 'b');
    glutAddMenuEntry("[a]   Antialiasing ON/OFF     ", 'a');
    glutAddMenuEntry("----------------------------", 255);
    glutAddMenuEntry("[i] Info ON/OFF             ", 'i');
    glutAddMenuEntry("[Esc] Exit                  ", 27);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  // ========================
  // estado normal del OpenGL
  // ========================
  glClearColor(fondo[0],fondo[1],fondo[2],1);  // color de fondo
  glDrawBuffer(GL_BACK);
  glMatrixMode(GL_MODELVIEW); glLoadIdentity(); // constante
  glShadeModel(GL_FLAT); // no interpola color

  if (antialias){
    glEnable(GL_POINT_SMOOTH); glEnable(GL_LINE_SMOOTH);
  }
  else {
    glDisable(GL_POINT_SMOOTH); glDisable(GL_LINE_SMOOTH);
  }

  // ========================
  // info
  cout << "\n\n\nPuntos de control (Maximo " << MAXPC << "):\n"
     << "     Click izq -> Selecciona (tol. 10) o\n"
        "                    Agrega despues del seleccionado\n"
     << "      Drag izq -> Mueve el seleccionado\n"
     << "         <DEL> -> Borra el seleccionado\n"
     << endl;
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv);// inicializa glut
  initialize(); // condiciones iniciales de la ventana y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0; //solo para que el builder se quede tranquilo (aqui nunca llega)
}
