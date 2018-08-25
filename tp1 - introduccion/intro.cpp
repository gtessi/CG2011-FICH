// Programa introductorio
// triángulo con tres puntos editables

#include <iostream> // cout
#include <cstdlib> // exit
#include <cmath> // fabs
#include <GL/glut.h>

using namespace std;

//------------------------------------------------------------
// variables globales
int
  w=480,h=360, // tamaño inicial de la ventana
  npuntos=0, // cantidad de puntos
  // pt[6], // los puntos (hasta 3: x0,y0,x1,y1,x2,y2)
  ep=-1, // índice del punto a editar
  
  // CONSIGNA 4
  pt[8]; // los puntos (hasta 4: x0,y0,x1,y1,x2,y2,c3,y3)

  // CONSIGNA 5
unsigned char
  color[3]; // arreglo con datos de un color
int
  xold=0, // coordenada x vieja
  yold=0; // coordenada y vieja

//============================================================
// callbacks

//------------------------------------------------------------

// arma un un nuevo buffer (back) y reemplaza el framebuffer
void Display_cb() {
  //static int counter=0; cout << "display: " << counter++ << endl;

  // arma el back-buffer
  glClear(GL_COLOR_BUFFER_BIT);// rellena con color de fondo

  if (!npuntos){ // no hay nada
    glutSwapBuffers(); // manda al monitor la pantalla vacía
    return;
  }

  // dibuja
  int i;
  // triángulo (sólo si ya están los tres puntos)
/*  if (npuntos==3) {
  // glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); glLineWidth(2);
    glColor3f(.4f,.6f,.8f);
    glBegin(GL_TRIANGLES);
      for(i=0;i<npuntos;i++) glVertex2i(pt[2*i],pt[2*i+1]);
    glEnd();
  }
  */
  
  // CONSIGNA 4
  // cuadrado (sólo si ya están los cuatro puntos)
  if (npuntos==4) {
    glColor3f(.4f,.6f,.8f);
    glBegin(GL_QUADS);
      for(i=0;i<npuntos;i++) glVertex2i(pt[2*i],pt[2*i+1]);
    glEnd();
  }
  
  // CONSIGNA 3
  // borde
  //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  glLineWidth(2);
  glColor3f(.9f,.1f,.1f);
  
  // dibuja los bordes a medida que se crean los puntos
  if(npuntos<4){
    glBegin(GL_LINE_STRIP);    
      for(i=0;i<npuntos;i++) glVertex2i(pt[2*i],pt[2*i+1]);
    glEnd();    
  } else {
    glBegin(GL_LINE_LOOP);    
      for(i=0;i<npuntos;i++) glVertex2i(pt[2*i],pt[2*i+1]);
    glEnd();
  }
  

  // puntos (después del triángulo, para que se vean encima)
  glColor3f(.1f,.2f,.3f);
  glPointSize(5.0); // punto grueso
  glBegin(GL_POINTS);
    for(i=0;i<npuntos;i++) glVertex2i(pt[2*i],pt[2*i+1]);
  glEnd();

  glutSwapBuffers(); // lo manda al monitor
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto de la ventana
void Reshape_cb(int width, int height){
  // cout << "reshape " << width << "x" << height << endl;
  if (!width||!height) return; // minimizado ==> nada
  w=width; h=height;
  glViewport(0,0,w,h); // región donde se dibuja (toda la ventana)
  // rehace la matriz de proyección (la porcion de espacio visible)
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();
  glOrtho(0,w,0,h,-1,1); // unidades = pixeles
  glutPostRedisplay(); // avisa que se debe redibujar
}

//------------------------------------------------------------
// Mouse

// Seguimiento del cursor
void PassiveMotion_cb(int x, int y){
  y=h-y; // el 0 está arriba
  cout << x << "," << y << "                    \r" << flush;
}

// Drag (movimiento con algun boton apretado)
void Motion_cb(int x, int y){
  y=h-y; // el 0 está arriba
  //cout << x << "," << y << endl;

  // CONSIGNA 2
  // controla que los puntos no salgan de los límites 
  if(x<0) x=0;
  if(x>w) x=w;
  if(y<0) y=0;
  if(y>h) y=h;
  
  pt[2*ep]=x;pt[2*ep+1]=y; // fija el punto editado en x,y
  glutPostRedisplay(); // avisa que se debe redibujar
}

// CONSIGNA 5
// callback para mover la figura
void Drag_cb(int x, int y){
  y=h-y; // el 0 está arriba
  //cout << x << "," << y << endl;
  
  // calculo los desplazamientos en x e y
  int
    difx=x-xold, dify=y-yold,
    i=0;
  
  // verifico que los puntos actualizados queden
  // dentro de la ventana
  for(i=0;i<npuntos;i++){
    if(pt[i*2]+difx<0 || pt[i*2]+difx>w) return;
    if(pt[i*2+1]+dify<0 || pt[i*2+1]+dify>h) return;

  }
  
  // si pasó los controles, actualizo cada
  // punto con los desplazamientos
  for(i=0;i<npuntos;i++){
    pt[i*2]+=difx;
    pt[i*2+1]+=dify;

  }
  
  // actualizo el punto donde se picó
  xold=x;
  yold=y;

  glutPostRedisplay(); // avisa que se debe redibujar
}

// Botones picados o soltados
void Mouse_cb(int button, int state, int x, int y){  
  cout << x << "," << y << "," << button << "," << state << endl;
  if (button==GLUT_LEFT_BUTTON){ // boton izquierdo
    if (state==GLUT_DOWN) { // clickeado
      y=h-y; // el 0 está arriba
      //cout << x << "," << y << endl;
      ep=-1;
      // verifica si picó a menos de 5 pixeles de algún punto previo
      for (int i=0;i<npuntos;i++){
        int d=abs(x-pt[2*i])+abs(y-pt[2*i+1]); // distancia simple (métrica del rombo)
        if (d>5) continue; // lejos
        ep=i; // edita el punto i
        glutMotionFunc(Motion_cb); // define el callback para los drags
        return;
      }
      
      // CONSIGNA 5
      // si pico en el cuadrilatero
      glReadBuffer(GL_FRONT);
      glReadPixels(x,y,1,1,GL_RGB,GL_BYTE,color);
      
      // muestra por consola el color que picó
      //cout << "COLOR RGB: " << int(color[0]) << " " << int(color[1]) << " " << int(color[2]) << endl;
      
      // si el color coincide, pasa el punto donde se picó
      // y llama al callback para mover la figura
      if(color[0]==51 && color[1]==76 && color[2]==102){
        xold=x;
        yold=y;
        glutMotionFunc(Drag_cb);
      }
      
      // no pico cerca de otro
      //if (npuntos==3) return; // ya hay 3 ==> no hace nada
      
      // CONSIGNA 4
      if (npuntos==4) return; // ya hay 4 ==> no hace nada
      
      // agrega un punto
      pt[2*npuntos]=x; pt[2*npuntos+1]=y; npuntos++;
      Display_cb(); // Redibuja
      // y queda listo para editarlo hasta que suelte el botón izquierdo
      ep=npuntos-1; // edita el ultimo punto
      glutMotionFunc(Motion_cb); // define el callback para los drags
    } // fin clickeado
    else if (state==GLUT_UP) // soltado
      glutMotionFunc(0); // anula el callback para los drags
  } // fin botón izquierdo
}

//------------------------------------------------------------
// Teclado

// Maneja pulsaciones del teclado (ASCII keys)
// x,y posicion del mouse cuando se teclea (aqui no importan)
void Keyboard_cb(unsigned char key,int x,int y) {
  if (key==127||key==8){ // DEL o Backspace ==> borra el punto ep (el editable)
    if (ep==-1||ep==npuntos) return;
    // corre los siguientes hacia atras
    for(int i=ep;i<npuntos-1;i++) {pt[2*i]=pt[2*i+2];pt[2*i+1]=pt[2*i+3];}
    npuntos--;
    glutPostRedisplay(); // avisa que se debe redibujar
    // Backspace borra para atras, DEL borra para adelante
    if (key==8) ep--; // ep pasa a ser el anterior
  }
  
  // CONSIGNA 1
  // ESC borra todos los puntos
  if (key==27){
    ep=-1; // define que no hay punto para editar
    npuntos=0; // no hay puntos
    glutPostRedisplay(); // avisa que se debe redibujar
  }
}

// Special keys (non-ASCII)
// teclas de funcion, flechas, page up/dn, home/end, insert
void Special_cb(int key,int xm=0,int ym=0) {
  if (key==GLUT_KEY_F4 && glutGetModifiers()==GLUT_ACTIVE_ALT) // alt+f4 => exit
    exit(EXIT_SUCCESS);
}

//------------------------------------------------------------
void inicializa() {
  // GLUT
  glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);// pide color RGB y double buffering
  glutInitWindowSize(w,h); glutInitWindowPosition(50,50);
  glutCreateWindow("introducción"); // crea el main window

  //declara los callbacks, los que (aun) no se usan (aun) no se declaran
  glutDisplayFunc(Display_cb);
  glutReshapeFunc(Reshape_cb);
  glutKeyboardFunc(Keyboard_cb);
  glutSpecialFunc(Special_cb);
  glutMouseFunc(Mouse_cb);
  glutPassiveMotionFunc(PassiveMotion_cb);

  // OpenGL
  glClearColor(0.85f,0.9f,0.95f,1.f); // color de fondo
  glMatrixMode(GL_MODELVIEW); glLoadIdentity(); // constante
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv); // inicialización interna de GLUT
  inicializa(); // define el estado inicial de GLUT y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0; // nunca se llega acá, es sólo para evitar un warning
}
