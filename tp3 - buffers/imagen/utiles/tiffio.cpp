#include "imagen.h"
#ifdef _USE_LIBTIFF

/*#if (defined(_MSC_VER)&&(_MSC_VER>=1400))
//(swprintf da error, ver el help en vs2005)
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif  se resolvio con USE_WIN32_FILEIO  */

#ifdef _WIN32
#define LIBTIFF_STATIC
#define USE_WIN32_FILEIO
#include "tiff/tiffio.h"
#else
#include <tiffio.h>
#endif
#include "imagen.h"

//using namespace std;

bool imagen::read_tiff (const char *filename){

  if (filename) strcpy(nombre,filename);
  char *ext=ext_begin(nombre);
  if (!ext) strcat(nombre,".tif"); else strcpy(ext,"tif");

  TIFF* tif = TIFFOpen(nombre, "r");
  if (!tif) return false;

  uint32 nw, nh;
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &nw);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &nh);
  size_t npixels= nw * nh;

  uint16 nc;
  TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &nc);

  // TIFFReadRGBAImage always deliveres 3 or 4 samples per pixel images  (siempre 4 (nestor))
  // (RGB or RGBA, see below). Cut-off possibly present channels (additional
  // alpha channels) from e.g. Photoshop. Any CMYK(A..) is now treated as RGB,
  // any additional alpha channel on RGB(AA..) is lost on conversion to RGB(A)
  // http://www.koders.com/cpp/fid9F3FAD3AC91595B252E40978C6DB10E57A38589C.aspx?s=tiff
  // PluginTIFF.cpp
  // sea como sea se lee como color

  uint32* raster = (uint32*) _TIFFmalloc(tsize_t(npixels * sizeof (uint32)));
  if (!raster) return false;

  if (!TIFFReadRGBAImage(tif, nw, nh, raster, 0)) return false;

  // transfiere RGBA de raster a RGB o L de buffer
  size_t f=nh,c,i3=0;
  uint32 pixel;
  unsigned char *nbuffer=new unsigned char [npixels*nc]; if (!nbuffer) return false;
  while (f--) { // las filas estan invertidas y f>0 por ser unsigned
    for(c=0; c<nw; c++){
      pixel=raster[f*nw+c];
      nbuffer[i3++]=(unsigned char) pixel&255;   pixel/=256; // R o L
      if (nc==1) continue;
      nbuffer[i3++]=(unsigned char) pixel&255; pixel/=256;   // G
      nbuffer[i3++]=(unsigned char) pixel&255;               // B
    }
  }

  _TIFFfree(raster);
  TIFFClose(tif);

  if (buffer) delete [] buffer; buffer=nbuffer;
  w=nw; h=nh; color=(nc==3);
  return true;
}


bool imagen::write_tiff (const char *filename){

  if (filename) strcpy(nombre,filename);
  char *ext=ext_begin(nombre);
  if (!ext) strcat(nombre,".tif"); else strcpy(ext,"tif");

  // http://www-106.ibm.com/developerworks/linux/library/l-libtiff/
  // http://www-106.ibm.com/developerworks/linux/library/l-libtiff2/

  // Open the TIFF file
  TIFF *image = TIFFOpen(nombre, "w"); if (!image) return false;

  TIFFSetField(image, TIFFTAG_PHOTOMETRIC, (color)? PHOTOMETRIC_RGB : PHOTOMETRIC_MINISBLACK);
  TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
  TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

  TIFFSetField(image, TIFFTAG_IMAGEWIDTH, w);
  TIFFSetField(image, TIFFTAG_IMAGELENGTH, h);
  TIFFSetField(image, TIFFTAG_ROWSPERSTRIP, h); // una tira con toda la imagen
  TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, 8);
  short nc=((color)? 3: 1);
  TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, nc);

  bool retval=(
    TIFFWriteEncodedStrip(image,0,&(buffer[0]),w*h*nc)
    ==-1)? true : false;

  // Close the file
  TIFFClose(image);
  return retval;
}

#endif
