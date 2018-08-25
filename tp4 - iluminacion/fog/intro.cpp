#include <cmath> // sqrt
#include <cstdlib> // exit
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

GLfloat lig_ambient [] = { 0.7, 0.7, 0.7, 1.0 };
GLfloat lig_diffuse [] = { 0.8, 0.8, 0.8, 1.0 };
GLfloat lig_specular [] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat mat_ambient [] = { 0.24725, 0.1995, 0.0745, 1.0 };
GLfloat mat_diffuse [] = { 0.75164, 0.60648, 0.22648, 1.0 };
GLfloat mat_specular [] = { 0.628281, 0.555802, 0.366065, 1.0 };
GLfloat mat_shininess [] = { 51.2 };

GLfloat fogColor[4] = {0.5, 0.5, 0.5, 1.0};
GLint FogMode[3] = {GL_LINEAR, GL_EXP, GL_EXP2};
char* FogModeString[3] ={"Linear", "Exponencial", "Exponencial al cuadrado"};
GLint FogModeInd=0;

GLshort modifier;
GLfloat
  x_size=640,
  y_size=480,
  lx, ly, lz,
  f_density=0.01;
bool fog=false,
  color=false,
  smooth=true,
  local=false;




///////////////////////////////////////////////////////////////////////////////

void draw_cb(void) {
  static int anterior=0;
  int tiempo=glutGet(GLUT_ELAPSED_TIME);
  if (tiempo-anterior<30) return;
  anterior=tiempo;
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPointSize(10);
  glColorMaterial(GL_FRONT, GL_AMBIENT);
  glEnable(GL_COLOR_MATERIAL);
  glColor3f(1,1,0);
  glBegin(GL_POINTS);
    glVertex3f(lx,ly,lz);
  glEnd();
  glColor4fv(mat_ambient);
  int i,j;
  for(i=-1;i<=1;i++){
    for(j=-1;j<=1;j++){
      glPushMatrix();
       glTranslatef((float)i*3.0,(float)j*2.0,-(float)j*3.0);
       glutSolidTeapot(1);
      glPopMatrix();
    }
  }
  glutSwapBuffers();
}

//------------------------------------------------------------

void Motion_cb(int x, int y) {
    if (modifier==GLUT_ACTIVE_ALT){
    }
    else {
         if (modifier==GLUT_ACTIVE_SHIFT){
           lz = ((float)(y - y_size/2))/45;
         }
         else {
           lx = ((float)(x-x_size/2))/45;
           ly = ((float)(y_size/2-y))/45;
         }
         GLfloat light_position[] = { lx, ly, lz, 1.0 };
         glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    }
    glutPostRedisplay();
}

void Mouse_cb(int button, int state, int x, int y) {
  if (button==GLUT_LEFT_BUTTON){
    if (state==GLUT_DOWN) {
      modifier = (short) glutGetModifiers();
      glutMotionFunc(Motion_cb); // callback para los drags
    }
  }
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto de la ventana
void reshape_cb(int w, int h) {
  x_size=w;
  y_size=h;
  glViewport(0,0,w,h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60,(GLfloat)w / (GLfloat)h,1.0,20.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0,0,8, 0,0,0, 0,1,0);
}

void Keyboard_cb(unsigned char key,int x=0,int y=0) {
  switch (key){
    case 'f': case 'F':
      fog=!fog;
      if (fog) glEnable(GL_FOG); else glDisable(GL_FOG);
      cout << ((fog)? "Fog" : "Sin Fog") << endl;
      break;
    case 'c': case 'C': // color/material
          color=!color;
          if (color) glDisable(GL_LIGHTING); else glEnable(GL_LIGHTING);
          cout << ((color)? "Color" : "Material") << endl;
      break;
    case 'a': case 'A':
          f_density+=0.01;
          glFogf (GL_FOG_DENSITY, f_density);
          glFogf (GL_FOG_END, 50-50*f_density);
      break;
    case 'z': case 'Z':
          f_density-=0.01;
          if(f_density<0) f_density=0;
          glFogf (GL_FOG_DENSITY, f_density);
          glFogf (GL_FOG_END, 50-50*f_density);
      break;
    case 'm': case 'M':
          FogModeInd++; FogModeInd=FogModeInd%3;
          glFogi (GL_FOG_MODE, FogMode[FogModeInd]);
          cout<<"Modo de neblina: "<<FogModeString[FogModeInd]<<endl;
      break;
    case 's': case 'S':
      smooth=!smooth;
      glShadeModel((smooth) ? GL_SMOOTH : GL_FLAT);
      cout << ((smooth)? "Sombreado de Gouraud" : "Sombreado plano") << endl;
      break;
    case 'l': case 'L':
      local=!local;
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, (local) ? 1 : 0);
      cout << ((local)? "Local viewer activado" : "Local viewer desactivado") << endl;
      break;
  }
  draw_cb();
}


//------------------------------------------------------------
// Inicializa GLUT y OpenGL
void inicializa(void) {
  GLfloat light_position[] = { 10.0, 10.0, 10.0, 1.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lig_diffuse);
  glLightfv(GL_LIGHT0, GL_AMBIENT, lig_ambient);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lig_specular);

  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

  glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);

  glEnable(GL_FOG);
  glFogi (GL_FOG_MODE, GL_EXP);
  glFogf (GL_FOG_DENSITY, f_density);
  glFogfv (GL_FOG_COLOR, fogColor);
  glHint (GL_FOG_HINT, GL_NICEST);
  glClearColor(fogColor[0], fogColor[1], fogColor[2], fogColor[3]);
  glFogf (GL_FOG_START, 1);
}

//------------------------------------------------------------
// main
int main(int argc, char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  glutInitWindowSize (640, 480);
  glutInitWindowPosition (0, 0);
  glutCreateWindow ("Iluminacion - Fog");
  inicializa();
  glutReshapeFunc(reshape_cb);
  glutDisplayFunc(draw_cb);
  glutMouseFunc(Mouse_cb);
  glutKeyboardFunc(Keyboard_cb);
  glutMainLoop();
  return 0;
}
