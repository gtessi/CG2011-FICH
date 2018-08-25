#include <iostream> // cout
#include <cmath> // sin cos
#include <GL/glut.h>
using namespace std;

//==========================================
// globales

extern int texmode;
extern bool wire,relleno,blend;
extern float line_color[];

//==========================================
void drawEsfera(float r, int nlat , int nlon){
  nlon*=2;
	float pi=4*atan(1.0),
        lat,lat1,dlat=pi/nlat,
        slat,clat,slat1,clat1,
        tlat, tlat1, dtlat=1.0/nlat,
        lon,dlon=2*pi/nlon,
        slon,clon,
        tlon, dtlon=1.0/nlon;
	int i,j;
  GLenum face;

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glFrontFace(GL_CW); 
  if (blend) {
    glDisable(GL_CULL_FACE);
    face=GL_FRONT_AND_BACK;
  }
  else {
    glEnable(GL_CULL_FACE); 
    face=GL_FRONT;
  }
  if (relleno){
    // caras
    glPolygonMode(face,GL_FILL);
		glColor3d(0.7,0.2,0.2);
    lat1=-pi/2; slat1=-1; clat1=0; tlat1=0;
		for(i=0;i<nlat;i++){
      lat=lat1; lat1+=dlat;
      slat=slat1; slat1=sin(lat1);
      clat=clat1; clat1=cos(lat1);
      tlat=tlat1; tlat1+=dtlat;
      lon=-pi; tlon=0; slon=0; clon=-1;
			glBegin(GL_QUAD_STRIP);
      glTexCoord2f(tlon,tlat );glNormal3f(clat *slon,clat *slon,slat );glVertex3f(r*clat *clon,r*clat *slon,r*slat );
      glTexCoord2f(tlon,tlat1);glNormal3f(clat1*slon,clat1*slon,slat1);glVertex3f(r*clat1*clon,r*clat1*slon,r*slat1);
			for(j=0;j<nlon;j++){
        lon+=dlon; slon=sin(lon); clon=cos(lon); tlon+=dtlon;
        glTexCoord2f(tlon,tlat );glNormal3f(clat *slon,clat *slon,slat );glVertex3f(r*clat *clon,r*clat *slon,r*slat );
        glTexCoord2f(tlon,tlat1);glNormal3f(clat1*slon,clat1*slon,slat1);glVertex3f(r*clat1*clon,r*clat1*slon,r*slat1);
      }
      glEnd();
		}
  }

  glDisable(GL_LIGHTING); glDisable(GL_TEXTURE_2D);
  if (wire){
    // aristas
		glColor3d(0.0,0.0,0.0);
    glPolygonMode(face,GL_LINE);
    lat1=-pi/2; slat1=-1; clat1=0; tlat1=0;
		for(i=0;i<nlat;i++){
      lat=lat1; lat1+=dlat;
      slat=slat1; slat1=sin(lat1);
      clat=clat1; clat1=cos(lat1);
      tlat=tlat1; tlat1+=dtlat;
      lon=-pi; tlon=0; slon=0; clon=-1;
			glBegin(GL_QUAD_STRIP);
      glVertex3f(r*clat *clon,r*clat *slon,r*slat );
      glVertex3f(r*clat1*clon,r*clat1*slon,r*slat1);
			for(j=0;j<nlon;j++){
        lon+=dlon; slon=sin(lon); clon=cos(lon); tlon+=dtlon;
        glVertex3f(r*clat *clon,r*clat *slon,r*slat );
        glVertex3f(r*clat1*clon,r*clat1*slon,r*slat1);
      }
      glEnd();
		}
  }

  // ejes
  glBegin(GL_LINES);
    glColor3d(1,0,0); glVertex3d(0,0,0); glVertex3d(1.5,0,0);
    glColor3d(0,1,0); glVertex3d(0,0,0); glVertex3d(0,1.5,0);
    glColor3d(0,0,1); glVertex3d(0,0,0); glVertex3d(0,0,1.5);
  glEnd();

  glPopAttrib();

}

