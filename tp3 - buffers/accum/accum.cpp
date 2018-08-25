// Test de Acumulaci\ufffdn
// Analizar:
//   El funcionamiento del bufer de acumulacion
//   El funcionamiento de glClear() en display y en regen

#include <cmath> // sqrt
#include <cstdlib> // exit
#include <iostream> // cout
#include <GL/glut.h>

using namespace std;

//------------------------------------------------------------
// variables globales y defaults

int
	w=480,h=360, // tama\ufffdo de la ventana
	lod=8, // nivel de detalle (subdivisiones de lineas y superficies parametricas)
	boton=-1, // boton del mouse clickeado
	xclick,yclick; // x e y cuando clickeo un boton
float // luces y colores en float
	lambient[]={.5f,.5f,.5f,1}, // luz ambiente
	ldiffuse[]={.7f,.7f,.7f,1}, // luz difusa
	fondo[]={1.f,1.f,1.f,1.f},   // color de fondo
	face_color[]={.6f,.5f,.2f,.5f}, // color de caras
	line_color[]={.3f,.2f,.1f,.5f}, // color de lineas
	escala=100,escala0, // escala de los objetos window/modelo pixeles/unidad
	eye[]={0,0,5}, target[]={0,0,0}, up[]={0,1,0}, // camara, mirando hacia y vertical
	znear=2, zfar=8, //clipping planes cercano y alejado de la camara (en 5 => veo de 3 a -3)
	lpos[]={2,1,5,1}, // posicion luz, l[4]: 0 => direccional -- 1 => posicional
	decay=.9; // factor de decaimiento

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
	
	// ponemos en el buffer de color un decay*lo_que_habia+(1-decay)*render nuevo
  glClear(GL_ACCUM_BUFFER_BIT); // 10% (1-decay) de color de fondo
	glAccum(GL_ACCUM,decay); // transfiere 90% (decay) de lo que habia en el buffer de color
	glAccum(GL_RETURN,1); // transfiere el resultado al buffer de color
  // dibuja en el buffer de color
	
	glClear(GL_DEPTH_BUFFER_BIT);
	
	static int a=0;
	if (rota) a=(a+2)%360;
	
	// tetera
	glPushMatrix();
	glRotatef (a, 0.0, 1.0, 0.0);
	glRotatef (15, 1.0, 0.0, 1.0);
	glFrontFace(GL_CW); 
	if (wire){
		if (!color) glDisable(GL_LIGHTING); // lineas sin material (siempre en color)
		glColor4fv(line_color); // color de lineas
		glutWireTeapot(.75);
		if (!color) glEnable(GL_LIGHTING); // vuelve a habilitar material
	}
	if (relleno){
		glEnable(GL_CULL_FACE);
		glColor4fv(face_color); // color de caras
		glutSolidTeapot(.75);
		glDisable(GL_CULL_FACE);
	}
	glPopMatrix();
	
	//linea
	glPushMatrix();
	glTranslatef (1.5, 1.0, 0.0);
	glRotatef (a, 0.0, 0.0, -1.0);
	if (!color) glDisable(GL_LIGHTING); // lineas sin material (siempre en color)
	glColor3ub(255,0,0);
	glBegin(GL_LINES);
	glVertex3f(0.f,0.f,0.f);
	glVertex3f(0.f,.75f,0.f);
	glEnd();
	if (!color) glEnable(GL_LIGHTING); // vuelve a habilitar material
	glPopMatrix();
	
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
	
	glClear(GL_COLOR_BUFFER_BIT|GL_ACCUM_BUFFER_BIT);
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
	// (int!!?? en seguida se hace >MAX_INT pero ... funciona)
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
      if (ym>h) ym=h; if (ym<1) ym=1;
      decay=pow(float(ym)/h,.5f);
			cout << "decay: " << decay << "       \r"; cout.flush();
      float d=1-decay;
	    glClearAccum(d*fondo[0],d*fondo[1],d*fondo[2],d*fondo[3]);
			Display_cb();
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
	glutInitDisplayMode(GLUT_DEPTH|GLUT_RGBA|GLUT_DOUBLE|GLUT_ACCUM);
	
	glutInitWindowSize(w,h); glutInitWindowPosition(50,50);
	
	glutCreateWindow("Prueba de Accumulation"); // crea el main window
	
	//declara los callbacks
	//los que no se usan no se declaran
	glutDisplayFunc(Display_cb); // redisplays
	glutReshapeFunc(Reshape_cb); // cambio de alto y ancho
	glutKeyboardFunc(Keyboard_cb); // teclado
	glutSpecialFunc(Special_cb); // teclas especiales
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
	
	// color de fondo (debe ser negro??)
	glClearColor(fondo[0],fondo[1],fondo[2],fondo[3]);
  float d=1-decay;
	glClearAccum(d*fondo[0],d*fondo[1],d*fondo[2],d*fondo[3]);
	
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
