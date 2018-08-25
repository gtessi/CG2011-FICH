#include <iostream> // cout
#include <cmath> // sin cos
#include <cstdlib> // rand
#include <GL/glut.h>
using namespace std;

//==========================================
// globales

extern int lod,w,h;
extern bool wire,relleno,smooth,cl_info,blend;
extern float face_color[],line_color[],control_color[],point_color[],fondo[];

//==========================================

static void randomize_color(){
  static const float f=1.0/256;
  face_color[0]=f*(rand()%256);
  face_color[1]=f*(rand()%256);
  face_color[2]=f*(rand()%256);
  cout << "color: " << face_color[0] << ',' << face_color[1] << ',' << face_color[2] << ')' << endl;
} 

//==========================================
// algunos objetos
//==========================================

void drawBezierSurface(){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();

  static const GLfloat ctrlpoints[4][4][3]={
    {{-1.5,-1.5, 4.0},{-0.5,-1.5, 2.0},{0.5,-1.5,-1.0},{1.5,-1.5, 2.0}},
    {{-1.5,-0.5, 1.0},{-0.5,-0.5, 3.0},{0.5,-0.5, 0.0},{1.5,-0.5,-1.0}},
    {{-1.5, 0.5, 4.0},{-0.5, 0.5, 0.0},{0.5, 0.5, 3.0},{1.5, 0.5, 4.0}},
    {{-1.5, 1.5,-2.0},{-0.5, 1.5,-2.0},{0.5, 1.5, 0.0},{1.5, 1.5,-1.0}}
  };

  glMap2f(GL_MAP2_VERTEX_3,
    0.0, 1.0,  3, 4,
    0.0, 1.0, 12, 4,
    &ctrlpoints[0][0][0]);
  glEnable(GL_MAP2_VERTEX_3);

  // 1/2 del original
  glScalef(.5,.5,.5);

  // lod+1 puntos o lod segmentos en cada direccion
  glMapGrid2f(lod,0,1,lod,0,1);

  // si se dibuja 1ro el relleno no hay transparencia contra las lineas
  if (relleno){
    glColor4fv(face_color);
    glFrontFace(GL_CW);// estan definidos al reves?
    glEnable(GL_AUTO_NORMAL); glEnable(GL_NORMALIZE); // si no lo renderiza mal
    glEvalMesh2(GL_FILL,0,lod,0,lod);
  }

  if (wire){
    // grilla de lineas
    glDisable(GL_NORMALIZE); glDisable(GL_AUTO_NORMAL);
    glDisable(GL_LIGHTING);
    glColor4fv(line_color);
    glEvalMesh2(GL_LINE,0,lod,0,lod);

    // poliedro y puntos de control
    int i,j;
    for (i=0;i<3;i++) for (j=0;j<3;j++){
      glColor4fv(control_color);
      glBegin(GL_LINE_STRIP);
        glVertex3fv(&ctrlpoints[i+1][j  ][0]);
        glVertex3fv(&ctrlpoints[i  ][j  ][0]);
        glVertex3fv(&ctrlpoints[i  ][j+1][0]);
      glEnd();

      glColor4fv(point_color);
      glBegin(GL_POINTS);
        glVertex3fv(&ctrlpoints[i  ][j  ][0]);
        if (j==2)
          glVertex3fv(&ctrlpoints[i  ][j+1][0]);
        if (i==2){
          glVertex3fv(&ctrlpoints[i+1][j  ][0]);
          if (j==2)
            glVertex3fv(&ctrlpoints[i+1][j+1][0]);
        }
      glEnd();
    }
    glColor4fv(control_color);
    glBegin(GL_LINE_STRIP);
      for (j=0;j<4;j++)
        glVertex3fv(&ctrlpoints[3][j][0]);
      for (i=2;i>=0;i--)
        glVertex3fv(&ctrlpoints[i][3][0]);
    glEnd();
  }

  glPopAttrib();
  glPopMatrix();
}

void drawNurbsSurface(){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();

  static GLfloat ctlpoints[4][4][3];
  static bool first=true;
  if (first){ // define los puntos de control (solo la primera vez que entra aqui)
    first=false;
    int u,v;
    for (u=0;u<4;u++) {
      for (v=0;v<4;v++) {
        ctlpoints[u][v][0] = 2.0f*(u-1.5f);
        ctlpoints[u][v][1] = 2.0f*(v-1.5f);

      if ((u==1||u==2)&&(v==1||v==2))
        ctlpoints[u][v][2] = 3.0;
      else
        ctlpoints[u][v][2] = -3.0;
      }
    }
  }

  static GLfloat knots[8] =
    {0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0};

  glScalef (0.3f, 0.3f, 0.3f);

  GLUnurbsObj *theNurb=gluNewNurbsRenderer();
  //GLU_SAMPLING_METHOD: (ojo: algunos dependen de la vista => incompatibles con display lists)
  //  GLU PATH LENGTH, (DEFAULT!!)
  //    GLU_SAMPLING_TOLERANCE es max length pixels edges of the tessellation polygons
  //  GLU OBJECT PATH LENGTH idem GLU PATH LENGTH object space
  //  GLU PARAMETRIC ERROR
  //    GLU PARAMETRIC TOLERANCE max dist pixels polygons/surfaces
  //  GLU OBJECT PARAMETRIC ERROR idem GLU PARAMETRIC ERROR object space
  //  GLU DOMAIN DISTANCE parametric sample points per unit length in u,v
  gluNurbsProperty(theNurb,GLU_SAMPLING_METHOD,GLU_DOMAIN_DISTANCE); // para usar lod
  gluNurbsProperty(theNurb,GLU_U_STEP,float(lod)); gluNurbsProperty(theNurb,GLU_V_STEP,float(lod));
//  gluNurbsProperty(theNurb,GLU_SAMPLING_METHOD,GLU_OBJECT_PATH_LENGTH); // no esta????
//  gluNurbsProperty(theNurb,GLU_SAMPLING_TOLERANCE,1.0/(lod+1));

  gluNurbsProperty(theNurb,GLU_CULLING,GL_TRUE); // si esta fuera del viewport
  if (relleno){
    gluNurbsProperty(theNurb,GLU_DISPLAY_MODE,GLU_FILL);

    glColor4fv(face_color);
    glEnable(GL_AUTO_NORMAL); glEnable(GL_NORMALIZE);

    gluBeginSurface(theNurb);
      gluNurbsSurface(theNurb,
          8, knots,
          8, knots,
          4 * 3,
          3,
          &ctlpoints[0][0][0],
          4, 4,
          GL_MAP2_VERTEX_3);
    gluEndSurface(theNurb);
  }
  if (wire){
    // grilla de lineas
    glDisable(GL_LIGHTING);
    glDisable(GL_NORMALIZE); glDisable(GL_AUTO_NORMAL);
    glColor4fv(line_color);
    gluNurbsProperty(theNurb,GLU_DISPLAY_MODE,GLU_OUTLINE_POLYGON);
    gluBeginSurface(theNurb);
      gluNurbsSurface(theNurb,
          8, knots,
          8, knots,
          4 * 3,
          3,
          &ctlpoints[0][0][0],
          4, 4,
          GL_MAP2_VERTEX_3);
    gluEndSurface(theNurb);
  }
  gluDeleteNurbsRenderer(theNurb);
  glPopAttrib();
  glPopMatrix();
}

enum cuadrica{cono,esfera,cilindro,disco};
static void drawQuadric(cuadrica tipo){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();

  GLdouble baseRadius=1, topRadius=.2, height=1.5;
  GLint slices=lod, stacks=lod;

  // preposiciona
  if (tipo!=disco) glRotated(90,-1,0,0);
  if (tipo==cono||tipo==cilindro) glTranslated(0,0,-height/2);

  GLUquadricObj *q=gluNewQuadric();

  if (relleno){
    glColor4fv(face_color);
    gluQuadricNormals(q,(smooth)? GLU_SMOOTH : GLU_FLAT);
    gluQuadricOrientation(q,GLU_OUTSIDE);
    //GLU FILL, GLU LINE, GLU POINT or GLU SILHOUETTE
    gluQuadricDrawStyle(q,GLU_FILL);
    if (tipo==cono)
      gluCylinder(q,baseRadius,topRadius,height,2*slices,stacks);
    else if (tipo==cilindro)
      gluCylinder(q,baseRadius,baseRadius,height,4*slices,stacks);
    else if (tipo==esfera){
      glEnable(GL_CULL_FACE);
      gluSphere(q,baseRadius,2*slices,stacks);
      glDisable(GL_CULL_FACE);
    }
    else if (tipo==disco)
      gluDisk(q,0,baseRadius,4*slices,stacks);
  }
  if (wire){
    // grilla de lineas
    glDisable(GL_LIGHTING);
    glColor4fv(line_color);
    gluQuadricNormals(q,GLU_NONE);
//    gluQuadricDrawStyle(q,GLU_SILHOUETTE);
    gluQuadricDrawStyle(q,GLU_LINE);
    if (tipo==cono)
      gluCylinder(q,baseRadius,topRadius,height,2*slices,stacks);
    else if (tipo==cilindro)
      gluCylinder(q,baseRadius,baseRadius,height,4*slices,stacks);
    else if (tipo==esfera)
      gluSphere(q,baseRadius,2*slices,stacks);
    else if (tipo==disco)
      gluDisk(q,0,baseRadius,4*slices,stacks);
  }
  gluDeleteQuadric(q);
  glPopAttrib();
  glPopMatrix();
}
void drawCone(){drawQuadric(cono);}
void drawCylinder(){drawQuadric(cilindro);}
void drawSphere(){drawQuadric(esfera);}
void drawDisk(){drawQuadric(disco);}

void drawTeapot(){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  // tetera
  glFrontFace(GL_CW); // esta definida al reves de lo estandar
  if (relleno){    
    // las backfaces estan visibles a proposito
    glColor4fv(face_color);
    glutSolidTeapot(1);
  }
  if (wire){
    glEnable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glColor4fv(line_color);
    glutWireTeapot(1);
  }
  glPopAttrib();
}

void drawTorus(){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glEnable(GL_CULL_FACE);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,0);
  if (relleno){
    glColor4fv(face_color);
    glutSolidTorus (0.25, 1, lod, 4*lod);
  }
  if (wire){
    glDisable(GL_LIGHTING); // lineas sin material (siempre en color)
    glColor4fv(line_color); // color de lineas
    glutWireTorus (0.25, 1,  lod, 4*lod);
  }
  glPopAttrib();
}

void drawObjects(int change=0){
  static int which=7;
  static const int nobj=8; // objetos
  which=(which+change)%nobj;

  static bool primera_vez=true;
  if (change!=0||primera_vez) {
    primera_vez=false;
//    randomize_color();// colores aleatorios
    // pasteles
/*    float m=(face_color[0]+face_color[1]+face_color[2])/3;
    face_color[0]=(face_color[0]+3*m)/4;
    face_color[1]=(face_color[1]+3*m)/4;
    face_color[2]=(face_color[2]+3*m)/4;*/
  }
  switch (which){
    case 0:
      drawTeapot();
      if (cl_info&&change) cout << "Teapot" << endl;
      break;
    case 1:
      drawBezierSurface();
      if (cl_info&&change) cout << "BezierSurface" << endl;
      break;
    case 2:
      drawNurbsSurface();
      if (cl_info&&change) cout << "NurbsSurface" << endl;
      break;
    case 3:
      drawSphere();
      if (cl_info&&change) cout << "Esfera" << endl;
      break;
    case 4:
      drawDisk();
      if (cl_info&&change) cout << "Disco" << endl;
      break;
    case 5:
      drawCone();
      if (cl_info&&change) cout << "Cono" << endl;
      break;
    case 6:
      drawCylinder();
      if (cl_info&&change) cout << "Cilindro" << endl;
      break;
    case 7:
      drawTorus();
      if (cl_info&&change) cout << "Cilindro" << endl;
      break;
  }
}

