// imagenes

#ifndef IMAGEN_H
#define IMAGEN_H

#include "filename.h"
#include "utiles.h"

// si se define alguna(s) de estas macros, se utiliza(n) para saber que libreria(s)
// utilizar y que formato(s) leer, sino se utiliza solo JPEG por defecto
// las macros son:
//    _USE_LIBJPEG para JPGs (http://www.ijg.org/)
//    _USE_EASYBMP para la BMPSs (http://easybmp.sourceforge.net/)
//    _USE_LIBTIFF para la TIFs (www.libtiff.org)
//    _USE_LIBPNG para la PNGs (http://www.libpng.org/pub/png/libpng.html)
#if !defined(_USE_LIBTIFF)&&!defined(_USE_LIBJPEG)&&!defined(_USE_EASYBMP)&&!defined(_USE_LIBPNG)
  #define _USE_LIBJPEG
#endif

#define _Px unsigned char
#define _GL_Px GL_UNSIGNED_BYTE

const short SZPx=sizeof(_Px);

class imagen{
public:
  _Px *buffer;
  int w, h;
  bool color; // 1 o 3 canales (escala de grises o color)
  char nombre[_max_file_len];

  imagen(); // w=0 h=0 color=true nombre=noname buffer vacio
  imagen(int wi,int hi,bool ci=true,int r=-1,_Px g=0,_Px b=0); // inicializado si hay r

  imagen(const imagen &o){*this=o;}
  imagen(const char *filename);
 ~imagen() {delete [] buffer;}

  void ini(); // borra el buffer
  imagen& clear(_Px r=0,_Px g=0,_Px b=0);
  _Px *makebuffer(int wi,int hi,bool ci=true); // genera el buffer

  imagen& roba(imagen &o); // le roba todos los datos
  void swap(imagen &o); // intercambia todos los datos
 
  char* set_nombre(const char*);
  char* rename(const char* newname){return set_nombre(newname);}

  //  the human eye is much better at seeing some colors than others.
  //  Appropriate coefficients are 0.299 for red, 0.587 for green, and 0.114 for blue.
  // El algoritmo da L=r*R+g*G+b*B; sin alpha
  imagen& gray(double r=0.299,double g=0.587,double b=0.114);
  imagen& minrgb(); // pasa a gris con el minmo entre r,g y b
  imagen& mk_color(double r=1,double g=1,double b=1); // pasa a color

  // Para ampliar el bilineal funciona mejor que el bicubic de PSP
  // No implemente el bicubico
  // Para reducir el acumulado es el mejor sin vueltas
  bool resize_bilinear(double fx, double fy=-1e64);
  // bool resize_bicubic(double fx, double fy=-1e64); implementar!!
  bool resize_acumulado(double fx, double fy=-1e64);
  bool resize(double fx, double fy=-1e64); // bilin/acumulado

  bool mirror();
  bool flip();
  bool left();
  bool right();

  // operadores
  imagen& operator=(const imagen &);
  bool negative();
  bool operator ~(){return negative();} // 255-buffer

  bool operator |=(const imagen&);
  bool operator &=(const imagen&);
  bool operator ^=(const imagen&);

  operator _Px *() {return buffer;}
  _Px& operator()(int columna,int fila,int channel=0){
    _revienta(fila<0||fila>=h||columna<0||columna>=w);
    return buffer[((color)? 3 : 1)*(w*fila+columna)+channel];
  }
  _Px operator()(int columna,int fila,int channel=0) const{
    _revienta(fila<0||fila>=h||columna<0||columna>=w);
    return buffer[((color)? 3 : 1)*(w*fila+columna)+channel];
  }

  // suma, combinacion o promedio... es lo mismo
  // a*(1-f)+b*f 
  // si f<0 es: a*(1-f)+(255-b)*f
  // si f>1 es: (255-a)*(1-f)+b*f
  imagen& combine(const imagen &a, const imagen &b, double f=.5);
  imagen& operator +(const imagen &o) {return combine(*this,o,.5);}
  imagen& operator -(const imagen &o) {return combine(*this,o,-.5);}

  bool abs_diff(const imagen&); // abs(-)

  bool darkest(const imagen&);
  bool lightest(const imagen&);

  // factor [0,1]
  bool dark(double f=.5); // x*(1-f)
  bool light(double f=.5); // 255-(255-x)*(1-f) o x+(255-x)*f

  // io
  bool read(const char *filename=0);
  bool write(const char *filename=0);
  bool inline open(const char *filename=0) {return read(filename);}
  bool inline save(const char *filename=0) {return write(filename);}

  bool read_jpeg (const char *filename=0);
  bool write_jpeg (const char *filename=0, int quality=100);
  bool inline read_jpg(const char *filename=0) {return read_jpeg(filename);}
  bool inline write_jpg(const char *filename=0, int quality=100) {return write_jpeg(filename,quality);}
  bool inline open_jpeg(const char *filename=0) {return read_jpeg(filename);}
  bool inline save_jpeg(const char *filename=0, int quality=100) {return write_jpeg(filename,quality);}
  bool inline open_jpg(const char *filename=0) {return read_jpeg(filename);}
  bool inline save_jpg(const char *filename=0, int quality=100) {return write_jpeg(filename,quality);}

  bool read_tiff (const char *filename=0);
  bool write_tiff (const char *filename=0);
  bool inline read_tif (const char *filename=0) {return read_tiff(filename);}
  bool inline write_tif (const char *filename=0) {return write_tiff(filename);}
  bool inline open_tiff (const char *filename=0) {return read_tiff(filename);}
  bool inline save_tiff (const char *filename=0) {return write_tiff(filename);}
  bool inline open_tif (const char *filename=0) {return read_tiff(filename);}
  bool inline save_tif (const char *filename=0) {return write_tiff(filename);}

  bool read_bmp (const char *filename=0);
  bool write_bmp (const char *filename=0);
  bool inline open_bmp (const char *filename=0) {return read_bmp(filename);}
  bool inline save_bmp (const char *filename=0) {return write_bmp(filename);}

  bool read_png (const char *filename=0);
  bool write_png (const char *filename=0);
  bool inline open_png (const char *filename=0) {return read_png(filename);}
  bool inline save_png (const char *filename=0) {return write_png(filename);}

 
  // para texturas w y h deben ser 2^n
  bool make_texture(int max_tex_sz);

  // bresenham
  void line (int x1, int y1, int x2, int y2, _Px cr=255,_Px cg=0, _Px cb=0);

};

#endif
