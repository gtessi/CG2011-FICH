#include <cstring>
#include <fstream>  // salida a debug

#include <cstdlib>
using namespace std;

#include "utiles.h" // swap
#include "case.h" // minusc
#include "imagen.h"

imagen::imagen(): buffer(0),w(0),h(0),color(true) {
  strcpy(nombre,"noname");
}

imagen::imagen(int wi,int hi,bool ci,int r,_Px g,_Px b): buffer(0),w(0),h(0),color(true) {
  strcpy(nombre,"noname");
  makebuffer(wi,hi,ci);
  if (r!=-1) clear((_Px)r,g,b);
}

imagen::imagen(const char *filename): buffer(0),w(0),h(0),color(true) {
  nombre[0]=0;
  read(filename);
}

_Px *imagen::makebuffer(int wi,int hi,bool colori){
  int
    size=w*h*((color)? 3: 1),
    newsize=wi*hi*((colori)? 3: 1);

  if (size!=newsize){
    _Px *newbuffer=0;
    if (newsize) {
      newbuffer=new _Px[newsize];
      if (!newbuffer) return buffer;
    }
    delete [] buffer;
    buffer=newbuffer;
  }
  //cambio
  w=wi; h=hi; color=colori;
  return buffer;
}

imagen& imagen::roba(imagen &o){ // le roba todos los datos
  delete [] buffer;
  buffer=o.buffer;
  strcpy(nombre,o.nombre);
  w=o.w; h=o.h; color=o.color;
  o.buffer=0; o.w=o.h=0; o.color=true;
  strcpy(o.nombre,"noname");
  return *this;
}

void imagen::swap(imagen &o){ // intercambia todos los datos
  Swap(buffer,o.buffer);
  char tmp[_max_file_len];
  strcpy(tmp,nombre); strcpy(o.nombre,nombre); strcpy(nombre,tmp);
  Swap(w,o.w); Swap(h,o.h); Swap(color,o.color);
}

char* imagen::set_nombre(const char* c){
  return strcpy(nombre,c);
}

void imagen::ini(){
  delete[] buffer; buffer=0;
  w=h=0;
  color=true;
  strcpy(nombre,"noname");
}

imagen& imagen::clear(_Px r,_Px g,_Px b){
  if(!buffer) return *this;
  int i,len=w*h,ix=0;
  if (!color||(r==g&&g==b)) memset(buffer,r,len*((color)? 3 : 1)*SZPx);
  else for (i=0;i<len;i++) {
    buffer[ix++]=r; buffer[ix++]=g; buffer[ix++]=b;
  }
  return *this;
}

bool imagen::read(const char* filename){
  if (filename) strcpy(nombre,filename);
  const char *exti=ext_begin(nombre); if (!exti) return false;
  if (!exti) strcat(nombre,".tif");
  if (!exti[0]) strcat(nombre,"tif");
  char ext[_max_ext_len]; strcpy(ext,exti); minusc(ext);
#ifdef _USE_LIBJPEG
  if (!strcmp(ext,"jpg")||!strcmp(ext,"jpeg")) return read_jpeg(filename);
#endif
#ifdef _USE_LIBPNG
  if (!memcmp(ext,"png",3)) return read_png(filename);
#endif
#ifdef _USE_LIBTIFF
  if (!memcmp(ext,"tif",3)) return read_tiff(filename);
#endif
#ifdef _USE_EASYBMP
  if (!strcmp(ext,"bmp")) return read_bmp(filename);
#endif
  return false;
}

bool imagen::write(const char* filename){
  if (!buffer) return false;
  if (filename) strcpy(nombre,filename);
  const char *exti=ext_begin(nombre);
  if (!exti) strcat(nombre,".tif");
  if (!exti[0]) strcat(nombre,"tif");
  char ext[_max_ext_len]; strcpy(ext,exti); minusc(ext);
#ifdef _USE_LIBJPEG
  if (!strcmp(ext,"jpg")||!strcmp(ext,"jpeg")) return write_jpeg(filename);
#endif
#ifdef _USE_LIBPNG
  if (!strcmp(ext,"png")) return write_png(filename);
#endif
#ifdef _USE_LIBTIFF
  if (!memcmp(ext,"tif",3)) return write_tiff(filename);
#endif
#ifdef _USE_EASYBMP
  if (!strcmp(ext,"bmp")) return write_bmp(filename);
#endif
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////
//                                grayscale
///////////////////////////////////////////////////////////////////////////////////////
//  Converting color to grayscale:
//  the human eye is much better at seeing some colors than others.
//  Appropriate coefficients are 0.299 for red, 0.587 for green, and 0.114 for blue.
imagen& imagen::gray(double r,double g,double b){
  if (!color) return *this;
  if (!buffer) {color=false; return *this;}

  double suma=r+g+b; if (suma==0) return clear();
  r/=suma; g/=suma; b/=suma;

  int ig,ic,size=w*h;

  _Px *nbuffer=new _Px[size];
  if (!nbuffer) return *this;

  for (ig=ic=0;ig<size;(ig++, ic+=3))
    nbuffer[ig]=(_Px) (buffer[ic]*r+buffer[ic+1]*g+buffer[ic+2]*b);

  color=false;
  delete [] buffer; buffer=nbuffer;
  return *this;
}

imagen& imagen::minrgb(){
  if (!color) return *this;
  if (!buffer) {color=false; return *this;}
  int lix,cix,blen=h*w;
  _Px *nbuffer=new _Px[blen];
  if (!nbuffer) return *this;
  _Px r,g,b,l;

  for (lix=0,cix=0;lix<blen;lix++) {
    r=buffer[cix++];g=buffer[cix++];b=buffer[cix++];
    l=((r<g)? r: g); if (b<l) l=b;
    nbuffer[lix]=l;
  }

  delete [] buffer; buffer=nbuffer;
  color=false;
  return *this;
}

imagen& imagen::mk_color(double r,double g,double b){
  if (color) return *this;
  if (!buffer) {color=true; return *this;}

  int blen=h*w,lix=blen,cix=3*blen;
  _Px *nbuffer=new _Px[3*blen];
  if (!nbuffer) return *this;
  _Px l;

  while (--lix){
    l=buffer[lix];
    nbuffer[--cix]=(_Px)(b*l); nbuffer[--cix]=(_Px)(g*l); nbuffer[--cix]=(_Px)(r*l);
  }

  delete [] buffer; buffer=nbuffer;
  color=true;
  return *this;
}

///////////////////////////////////////////////////////////////////////////////////////
//                                textura
///////////////////////////////////////////////////////////////////////////////////////
// para texturas w y h deben ser 2^n y no mayores que maxt
bool imagen::make_texture(int maxt){ // ya veremos donde meto la imagen
  int nw=1,nh=1,f, ch=((color)? 3 : 1);

  double r=double(maxt)/((h>w)? h: w); if (r<1) resize(r);

  while (nw<w) nw*=2;
  while (nh<h) nh*=2;
  if (w==nw&&h==nh) return true;
  _Px *nbuffer=new _Px[nw*nh];
  if (!nbuffer) return false;
//  memset(nbuffer,255,nw*nh*((color)? 3 : 1)*SZPx); //blanco???
  memset(nbuffer,0,ch*nw*nh*SZPx); // negro
  for (f=0;f<h;f++) memcpy(&nbuffer[ch*nw*f],&buffer[ch*w*f],ch*w*SZPx);
  delete [] buffer; buffer=nbuffer;
  w=nw; h=nh;
  return true;
}


///////////////////////////////////////////////////////////////////////////////////////
//                                resize
///////////////////////////////////////////////////////////////////////////////////////
bool imagen::resize(double fx,double fy){
  return (fx*fy<1)? resize_acumulado(fx,fy) : resize_bilinear(fx,fy);
}

// esto es un suavizador bilineal para ampliar, para reducir es una cagada
// al reducir, si un grupo de viejos pixels queda atrapado por un nuevo pixel
// solo se toma el x,y (el "central")  sin promediar
bool imagen::resize_bilinear(double fx, double fy){
  if (!buffer||!w||!h) return false;
  if (fy==-1e64) fy=fx;
  if (fx*fy==0) return false;
  bool xmirror=false, ymirror=false;
  if (fx<0) {xmirror=true; fx=-fx;}
  if (fy<0) {ymirror=true; fy=-fy;}

  // calcula w y h enteros
  int nw=int(w*fx); int nh=int(h*fy);

  // para que el 0 sea el 0 y el w-1 sea el nw-1, se calcula el "verdadero factor"
  fx=double(nw-1)/(w-1), fy=double(nh-1)/(h-1);

  //(f,c) son coordenadas enteras, (x,y) no
  int f,oldf,c,oldc,ix=0,oldix00,oldix10,oldix01,oldix11;
  double x,y,dx,dy;
  short i,ch=((color)? 3: 1);

  _Px* nbuffer=new _Px[nw*nh*ch];
  if (!nbuffer) return false;

  for(f=0;f<nh;f++){
    // asumo que los reales se truncan al convertirlos a enteros
    y=f/fy; oldf=int(y); dy=y-oldf;
    for(c=0;c<nw;c++){
      x=c/fx; oldc=int(x); dx=x-oldc;
      oldix00=(oldf*w+oldc)*ch;     oldix10=oldix00+ch;
      oldix01=((oldf+1)*w+oldc)*ch; oldix11=oldix01+ch;
      for(i=0;i<ch;i++){
        nbuffer[ix++]=(_Px) (
          buffer[oldix00]*(1-dx)*(1-dy)+
          buffer[oldix10]*(  dx)*(1-dy)+
          buffer[oldix01]*(1-dx)*(  dy)+
          buffer[oldix11]*(  dx)*(  dy));
        oldix00++; oldix10++; oldix01++; oldix11++;
      }
    }
  }

  w=nw; h=nh; delete [] buffer; buffer=nbuffer;

  //mirror
  if (xmirror&&!mirror()) return false;
  if (ymirror&&!flip()) return false;
  return true;
}
/*
bool imagen::resize_bicubic(double fx, double fy){
  implementar!!!
}
*/
// asumo que los reales se truncan al convertirlos a enteros
bool imagen::resize_acumulado(double fx, double fy){
  if (!buffer||!w||!h) return false;
  if (fy==-1e64) fy=fx;
  if (fx*fy==0) return false;
  bool xmirror=false, ymirror=false;
  if (fx<0) {xmirror=true; fx=-fx;}
  if (fy<0) {ymirror=true; fy=-fy;}

  // calcula w y h enteros y poner bien fx y fy
  int nw=int(w*fx); fx=double(nw)/w;
  int nh=int(h*fy); fy=double(nh)/h;

  //(f,c) son coordenadas enteras, (x,y) no
  double x,y,dx,dy,sumw;
  short i,ch=((color)? 3: 1);
  int f,oldf,c,oldc,lix=0,cix,nix=0,chnw=ch*nw;

  _Px* nbuffer=new _Px[nw*nh*ch];
  if (!nbuffer) return false;
  _Px* lbuffer=new _Px[nw*h*ch];
  if (!lbuffer) {delete [] nbuffer; return false;}
  double* cbuffer=new double[nw*ch];
  if (!cbuffer) {delete [] nbuffer; delete [] lbuffer; return false;}
  double pixel[3];

  // arma las h filas (h*nw) de pixels acumulados
  for(f=0;f<h;f++){
    for(c=0;c<nw;c++){
      for(i=0;i<ch;i++) pixel[i]=0;
      // primer pedazo
      x=c/fx; oldc=int(x); dx=x-oldc; sumw=0;
      if (dx>0){
        for(i=0;i<ch;i++) pixel[i]+=(1-dx)*buffer[(f*w+oldc)*ch+i];
        sumw+=1-dx; oldc++;
      }
      // enteros intermedios
      while(oldc<int((c+1)/fx)){
        for(i=0;i<ch;i++) pixel[i]+=buffer[(f*w+oldc)*ch+i];
        sumw+=1; oldc++;
      }
      // pedazo final
      dx=(c+1)/fx-oldc;
      if (dx>0){
        for(i=0;i<ch;i++) pixel[i]+=dx*buffer[(f*w+oldc)*ch+i];
        sumw+=dx;
      }
      for(i=0;i<ch;i++)
        lbuffer[lix++]=(_Px) (pixel[i]/sumw);
    }
  }

  // acumula filas
  for(f=0;f<nh;f++){
    y=f/fy; oldf=int(y); dy=y-oldf; sumw=0;
    for(c=0;c<chnw;c++) cbuffer[c]=0;
    // primer pedazo
    if (dy>0){
      cix=oldf*chnw;
      for(c=0;c<chnw;c++) cbuffer[c]+=(1-dy)*lbuffer[cix+c];
      sumw+=1-dy; oldf++;
    }
    // enteros intermedios
    while(oldf<int((f+1)/fy)){
      cix=oldf*chnw;
      for(c=0;c<chnw;c++) cbuffer[c]+=lbuffer[cix+c];
      sumw+=1; oldf++;
    }
    // pedazo final
    dy=(f+1)/fy-oldf;
    if (dy>0){
      cix=oldf*chnw;
      for(c=0;c<chnw;c++) cbuffer[c]+=dy*lbuffer[cix+c];
      sumw+=dy;
    }
    for(c=0;c<chnw;c++)
      nbuffer[nix++]=(_Px)(cbuffer[c]/sumw);
  }

  delete[] lbuffer; delete [] cbuffer;

  w=nw; h=nh; delete [] buffer; buffer=nbuffer;

  //mirror
  if (xmirror&&!mirror()) return false;
  if (ymirror&&!flip()) return false;

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////
//                                operadores
///////////////////////////////////////////////////////////////////////////////////////

imagen& imagen::operator=(const imagen &img){
  if (&img==this) return *this; // evita esta=esta
  strcpy(nombre,img.nombre);

  int
    size=w*h*((color)? 3: 1),
    newsize=img.w*img.h*((img.color)? 3: 1);

  if (size!=newsize){
    delete [] buffer;
    if (!newsize) buffer=0;
    else buffer=new _Px[newsize];
  }

  //cambio
  w=img.w; h=img.h; color=img.color;
  if (newsize) memcpy(buffer,img.buffer,newsize*SZPx);
  return *this;
}

bool imagen::negative(){
  _Px *b=buffer+h*w*((color)? 3: 1)*SZPx;
  while ((--b)>=buffer) *b=~(*b);
  return true;
}

#define _MKOPE1(op) bool imagen::operator op(const imagen &img){\
  if (h!=img.h||w!=img.w||(color^img.color)) return false;\
  int ix=h*w*((color)? 3: 1);\
  while (ix--) buffer[ix] op img.buffer[ix];\
  return true;\
}

_MKOPE1(|=)
_MKOPE1(&=)
_MKOPE1(^=)

#undef _MKOPE1


// suma, combinacion o promedio... es lo mismo
// a*(1-f)+b*f
// si f<0 es: a*(1-f)+(255-b)*f
// si f>1 es: (255-a)*(1-f)+b*f
imagen& imagen::combine(const imagen& a,const imagen& b,double f){
  if (h!=b.h||w!=b.w) return *this;

  if (f==0) return *this=a;
  else if (f==1) return *this=b;

  if (a.color&&!b.color) {
    (*this)=b; mk_color();
    return combine(a,(*this),f);
  }
  if (!a.color&&b.color) {
    (*this)=a; mk_color();
    return combine((*this),b,f);
  }
  if (a.color&&b.color&&!color) mk_color();
  if (!buffer||w!=a.w||h!=a.h||color!=a.color)
    makebuffer(a.w,a.h,a.color);

  int ix=h*w*((color)? 3: 1);
  if (f<0){
    while (ix--) buffer[ix]=(_Px)(double(a.buffer[ix])*(1-f)+double(255-b.buffer[ix])*f);
  }
  else if (f<1){
    while (ix--) buffer[ix]=(_Px)(double(a.buffer[ix])*(1-f)+double(b.buffer[ix])*f);
  }
  else if (f>1){
    while (ix--) buffer[ix]=(_Px)(double(255-a.buffer[ix])*(1-f)+double(b.buffer[ix])*f);
  }
  return *this;
}

// abs(diferencia)
bool imagen::abs_diff(const imagen &img){
  if (h!=img.h||w!=img.w||(color^img.color)) return false;
  int ix=h*w*((color)? 3: 1);
  while (ix--)
    buffer[ix]=(_Px)(abs(buffer[ix]-img.buffer[ix]));
  return true;
}

// el menor o el mayor
bool imagen::lightest(const imagen &img){
  if (h!=img.h||w!=img.w||(color^img.color)) return false;
  int ix=h*w*((color)? 3: 1);
  while (ix--) buffer[ix]=Max(buffer[ix],img.buffer[ix]);
  return true;
}

bool imagen::darkest(const imagen &img){
  if (h!=img.h||w!=img.w||(color^img.color)) return false;
  int ix=h*w*((color)? 3: 1);
  while (ix--) buffer[ix]=Min(buffer[ix],img.buffer[ix]);
  return true;
}


// f [0,1]
// x*(1-f)
bool imagen::dark(double f){
  _Px *b=buffer+h*w*((color)? 3: 1)*SZPx;
  while ((--b)>=buffer) (*b)=(_Px)((*b)*(1-f)); // ojo la conversion
  return true;
}
// x+(255-x)*f
bool imagen::light(double f){
  _Px *b=buffer+h*w*((color)? 3: 1)*SZPx;
  while ((--b)>=buffer) *b+=(_Px)((255-*b)*f);
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////
//                                mirror
///////////////////////////////////////////////////////////////////////////////////////

bool imagen::mirror(){
  if (!buffer) return false;
  short i,ch=((color)? 3: 1);
  int f,c,wch=w*ch;
  _Px *linea=0;
  for (f=0;f<h;f++){
    linea=&buffer[f*wch];
    for(c=0;c<w/2;c++)for(i=0;i<ch;i++) Swap(linea[c*ch+i],linea[(w-1-c)*ch+i]);
  }
  return true;
}

bool imagen::flip(){
  if (!buffer) return false;
  int f,wch=w*((color)? 3: 1),sz=wch*SZPx;
  _Px *linea=new _Px[wch];
  for (f=0;f<h/2;f++){
    memcpy(linea,&buffer[f*wch],sz);
    memcpy(&buffer[f*wch],&buffer[(h-1-f)*wch],sz);
    memcpy(&buffer[(h-1-f)*wch],linea,sz);
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////
//                            rotacion de 90 grados
///////////////////////////////////////////////////////////////////////////////////////

bool imagen::right(){
  if (!buffer) return false;
  short ch=((color)? 3: 1),sz=ch*SZPx;
  int f,c,wch=w*ch,hch=h*ch,size=h*wch;
  _Px *nbuffer=new _Px[size],*linea;
  if (!nbuffer) return false;
  for (f=0;f<h;f++){
    linea=&buffer[f*wch];
    for(c=0;c<w;c++) memcpy(&nbuffer[c*hch+(h-f-1)*ch],&linea[c*ch],sz);
  }
  delete [] buffer; buffer=nbuffer; Swap(w,h);
  return true;
}

bool imagen::left(){
  if (!buffer) return false;
  short ch=((color)? 3: 1),sz=ch*SZPx;
  int f,c,wch=w*ch,hch=h*ch,size=h*wch;
  _Px *nbuffer=new _Px[size],*linea;
  if (!nbuffer) return false;
  for (f=0;f<h;f++){
    linea=&buffer[f*wch];
    for(c=0;c<w;c++) memcpy(&nbuffer[(w-c-1)*hch+f*ch],&linea[c*ch],sz);
  }
  delete [] buffer; buffer=nbuffer; Swap(w,h);
  return true;
}



///////////////////////////////////////////////////////////////////////////////////////
//                            linea de bresenham en la imagen
///////////////////////////////////////////////////////////////////////////////////////
void imagen::line (int x1, int y1, int x2, int y2, _Px cr, _Px cg, _Px cb)
{
  int d, dx, dy;
  int aa, ax, ay;
  int sy;
  int x, y;
  int ch=(color)? 3 : 1, wch=w*ch;
  if (!color&&(cg||cb)) cr=(_Px)(0.299*cr+0.587*cg+0.114*cb);
  _Px *point, *endpoint;
  if (x1 > x2) { // x creciente
    Swap(x1,x2); Swap(y1,y2);
  }
  // clipping
  if (x1<0) {
    if (x2<0) return;
    y1=y1-(y2-y1)/(x2-x1)*x1;
    x1=0;
  }
  if (x2>=w) {
    if (x1>=w) return;
    y2=y1+(y2-y1)/(x2-x1)*(w-1-x1);
    x2=w-1;
  }
  if (y1<0){
    if (y2<0) return;
    x1=x1-(x2-x1)/(y2-y1)*y1;
    y1=0;
  }
  if (y2<0){
    x2=x1-(x2-x1)/(y2-y1)*y1;
    y2=0;
  }
  if (y1>=h){
    if (y2>=h) return;
    x1=x1+(x2-x1)/(y2-y1)*(h-1-y1);
    y1=h-1;
  }
  if (y2>=h){
    x2=x1+(x2-x1)/(y2-y1)*(h-1-y1);
    y2=h-1;
  }

  dx = x2 - x1;
  dy = y2 - y1;
  ax = dx << 1;
  ay = abs(dy) << 1;
  sy = (dy >= 0) ? 1 : -1;
  x = x1;
  y = y1;

  point = buffer + ch*x + wch*y;
  endpoint = buffer + ch*x2 + wch*y2;
  if (ax > ay){
    d = ay - (ax >> 1);
    aa = ay;
    if (sy > 0) {
      while (point != endpoint){
        *point = cr;
        if (color) {
          *(point+1)=cg;
          *(point+2)=cb;
        }
        if (d >= 0) {
          point+=wch;
          d -= ax;
        }
        point++;
        d+=aa;
      }
    }
    else{
      while (point != endpoint){
        *point = cr;
        if (color) {
          *(point+1)=cg;
          *(point+2)=cb;
        }
        if (d >= 0){
          point-=wch;
          d-=ax;
        }
        point++;
        d+=aa;
      }
    }
  }
  else{
    d=ax-(ay >> 1);
    aa=ax;
    if (sy > 0){
      while (point != endpoint){
        *point = cr;
        if (color) {
          *(point+1)=cg;
          *(point+2)=cb;
        }
        if (d > 0){
          point++;
          d-=ay;
        }
        point+=wch;
        d+=aa;
      }
    }
    else{
      while (point != endpoint){
        *point = cr;
        if (color) {
          *(point+1)=cg;
          *(point+2)=cb;
        }
        if (d > 0){
          point++;
          d-=ay;
        }
        point-=wch;
        d+=aa;
      }
    }
  }

  *point = cr;
  if (color) {
    *(point+1)=cg;
    *(point+2)=cb;
  }
}

