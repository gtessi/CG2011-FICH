#include <cstdlib> // exit
#include <iostream> // cout
#include <GL/glut.h>

#include "utiles/imagen.h"

using namespace std;

//------------------------------------------------------------
// variables globales y defaults

imagen img0,img1;

int
  w=480,h=360, // tamaño de la ventana
  wimg=0,himg=0, // rectangulo que englobaria las imagenes pegadas
  gap=5; // espacio entre imagenes y bordes
float
  zoom=1; // ventana/imagen

//------------------------------------------------------------
// muestra una imagen en pantalla, en x0,y0
void mostrar_imagen(const imagen& img, int x0,int y0){
  glRasterPos2i(x0,y0);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // nunca entendi muy bien que catzo es esto
  glPixelZoom(zoom,zoom);
  glDrawPixels(img.w,img.h,((img.color)? GL_RGB : GL_LUMINANCE), _GL_Px, img.buffer);
}

//------------------------------------------------------------
void Display_cb() {

  glClear(GL_COLOR_BUFFER_BIT); // borra

  if (img0) mostrar_imagen(img0,gap,gap);
  if (img1) mostrar_imagen(img1,int(img0.w*zoom+.5)+2*gap,gap);

  glutSwapBuffers();
#ifdef _DEBUG
  int errornum=glGetError();
  while(errornum!=GL_NO_ERROR){
         if(errornum==GL_INVALID_ENUM)
           cout << "GL_INVALID_ENUM" << endl;
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
#endif // _DEBUG
}


//------------------------------------------------------------
// Maneja cambios de ancho y alto del screen
void Reshape_cb(int width, int height){

  h=height; w=width;
  if (w==0||h==0) return;

  glViewport(0,0,w,h); // region donde se dibuja

  // matriz de proyeccion
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();
  glOrtho(0,w,0,h,-1,1);

  // calcula el zoom de las iagenes
  float wz=float(w-3*gap)/wimg, hz=float(h-2*gap)/himg;
  zoom=(wz<hz)? wz : hz;

  glutPostRedisplay();
}

//------------------------------------------------------------
// Teclado
void Special_cb(int key,int xm=0,int ym=0) {
  if (key==GLUT_KEY_F4 && glutGetModifiers()==GLUT_ACTIVE_ALT) // alt+f4 => exit
    exit(EXIT_SUCCESS);
  if (key==GLUT_KEY_F12) { // captura la ventana y la guarda en un jpg
	  imagen img(w,h,true);
	  glReadPixels(0,0,w,h,GL_RGB,GL_UNSIGNED_BYTE,img.buffer);
	  img.flip();
	  if (img.save_jpg("captura.jpg"))
		  cout << "Imagen guardada: captura.jpg" << endl;
	  else
		  cout << "No se pudo escribir la imagen" << endl;
  }
}

//------------------------------------------------------------
// Inicializa GLUT y OpenGL
void initialize() {
  glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);
  glutInitWindowSize(w,h); glutInitWindowPosition(50,50);
  glutCreateWindow("Imagenes"); // crea el main window

  //declara los callbacks
  glutDisplayFunc(Display_cb);
  glutReshapeFunc(Reshape_cb);
  glutSpecialFunc(Special_cb);

  // estado normal del OpenGL
  glClearColor(.9f,.85f,.85f,1.f); // color de fondo
  glMatrixMode(GL_MODELVIEW); glLoadIdentity(); // constante

}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  // lee las imagenes y las da vuelta (para OpenGL)
  if (argc>1){
    if (!img0.read(argv[1])){
      cout << "\nError, no se pudo leer la imagen: " << argv[1] << endl;
      cin.get();
      return 1;
    }
    else {
      img0.flip(); img0.gray(); // para usar algo: la convierte en gris
      wimg=img0.w; himg=img0.h;
    }
    if (argc>2){
      if (!img1.read(argv[2])){
        cout << "\nError, no se pudo leer la imagen: " << argv[2] << endl;
        cin.get();
        return 1;
      }
      else {
        img1.flip(); img1.resize(2);  // para usar algo: la duplica
        wimg+=img1.w; if (img1.h>himg) himg=img1.h;
      }
    }
  }
  else {
    cout << "\nTerminar esto (cualquier tecla) y luego: \n"
         << "\t ejecutar desde consola: imagen nombre1 nombre2\n"
         << "(formatos admitdos: png, bmp, tif o jpg)" << endl;
    cin.get();
    return 1;
  }

  // arma las ventanas y entra al loop
  glutInit(&argc,argv);// inicializa glut
  initialize(); // condiciones iniciales de la ventana y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0;
}
