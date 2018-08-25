#include "imagen.h"
#ifdef _USE_EASYBMP

#include "bmp/EasyBMP.h"

bool imagen::read_bmp(const char *filename){
  if (filename) strcpy(nombre,filename);
  char *ext=ext_begin(nombre);
  if (!ext) strcat(nombre,".bmp"); else strcpy(ext,"bmp");

  BMP img;
  if (!img.ReadFromFile(nombre))
    return false;
  if (!makebuffer(img.TellWidth(),img.TellHeight(),true))
    {ini(); return false;}

  int i,j,ix;
  _Px r,g,b;
  for(color=false,ix=j=0;j<h;j++){
    for(i=0;i<w;i++){
      r=img(i,j)->Red;   buffer[ix++]=r;
      g=img(i,j)->Green; buffer[ix++]=g;
      b=img(i,j)->Blue;  buffer[ix++]=b;
      if (!color&&(r!=g||g!=b)) color=true;
    }
  }
  if (!color) gray(1,0,0);
  return true;
}

bool imagen::write_bmp(const char *filename){
  if (filename) strcpy(nombre,filename);
  char *ext=ext_begin(nombre);
  if (!ext) strcat(nombre,".bmp"); else strcpy(ext,"bmp");

  BMP img;
  if (!img.SetBitDepth((color)? 24 : 8) || !img.SetSize(w,h))
    return false;

  int i=0,j=0,ix=0,size=w*h; if (color) size*=3;
  if (!color) {
    while (ix<size){
      img(i,j)->Red=img(i,j)->Green=img(i,j)->Blue=buffer[ix++];
      i++; if (i==w) {i=0; j++;}
    }
    CreateGrayscaleColorTable(img);
  }
  else {
    while (ix<size){
      img(i,j)->Red=buffer[ix++];
      img(i,j)->Green=buffer[ix++];
      img(i,j)->Blue=buffer[ix++];
      i++; if (i==w) {i=0; j++;}
    }
  }

  // write the output file
  if (!img.WriteToFile(nombre))
    return false;

  return true;
}

#endif
