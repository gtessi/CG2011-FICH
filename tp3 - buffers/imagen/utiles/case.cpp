#include <cctype> // upper/lower case
#include <cstring> // strlen
#include "case.h"

//using namespace std; ??????

char* mayusc(char *s){
  if (s) for(char *i=s;i<s+strlen(s);i++) if (islower(*i)) *i=toupper(*i);
  return s;
}

char* minusc(char *s){
  if (s) for(char *i=s;i<s+strlen(s);i++) if (isupper(*i)) *i=tolower(*i);
  return s;
}

///////////////////////////////////////////////////////////////////////////////////
// arregla acentos y enies
///////////////////////////////////////////////////////////////////////////////////
//Terminal <-> ISO 8859-1 o Unicode
char *T2U(char *txt){
  static const int t2u[]={
      0, 63, 63, 63, 63, 63, 63, 63, 63, 63, 10, 63, 63, 13, 63, 63, // 000-015
     63, 63, 63, 63,182,167, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, // 016-031
     32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, // 032-047
     48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, // 048-063
     64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, // 064-079
     80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, // 080-095
     96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111, // 096-111
    112,113,114,115,116,117,118,119,120,121,122,123,124,125,126, 63, // 112-127
    199,252,233,226,228,224,229,231,234,235,232,239,238,236,196,197, // 128-143
    201,230,198,244,246,242,251,249,255,214,220,248,163,216,215,131, // 144-159
    225,237,243,250,241,209,170,186,191,174,172,189,188,161,171,187, // 160-175
     63, 63, 63,124, 63,193,194,192,169, 63, 63, 63, 63,162,165, 63, // 176-191
     63, 63, 63, 63,151, 43,227,195, 63, 63, 63, 63, 63, 61, 63,164, // 192-207
    240,208,202,203,200, 39,205,206,207, 63, 63, 63, 63,166,204, 63, // 208-223
    211,223,212,210,245,213,181,222,254,218,219,217,253,221,175,180, // 224-239
    173,177, 61,190,182,167,247,184,176,168,183,185,179,178, 63,160};// 240-255

  char *c=txt-1; while (*(++c)){
    if (*c<0) *c=t2u[256+*c]; else *c=t2u[int(*c)];
  }
  return txt;
}

char *U2T(char *txt){
  static const int u2t[]={
      0, 63, 63, 63, 63, 63, 63, 63, 63, 63, 10, 63, 63, 13, 63, 63, // 000-015
     63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, // 016-031
     32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, // 032-047
     48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, // 048-063
     64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, // 064-079
     80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, // 080-095
     96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111, // 096-111
    112,113,114,115,116,117,118,119,120,121,122,123,124,125,126, 63, // 112-127
     69, 63, 44,159, 34, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, // 128-143
     63, 96, 44, 34, 34, 63, 45,196,126, 63, 63, 63, 63, 63, 63, 63, // 144-159
    255,173,189,156,207,190,221,245,249,184,166,174,170,240,169,238, // 160-175
    248,241,253,252,239,230,244,250,247,251,167,175,172,171,243,168, // 176-191
    183,181,182,199,142,143,146,128,212,144,210,211,222,214,215,216, // 192-207
    209,165,227,224,226,229,153,158,157,235,233,234,154,237,231,225, // 208-223
    133,160,131,198,132,134,145,135,138,130,136,137,141,161,140,139, // 224-239
    208,164,149,162,147,228,148,246,155,151,163,150,129,236,232,152};// 240-255

  char *c=txt-1; while (*(++c)){
    if (*c<0) *c=u2t[256+*c]; else *c=u2t[int(*c)];
  }
  return txt;
}
///////////////////////////////////////////////////////////////////////////////////