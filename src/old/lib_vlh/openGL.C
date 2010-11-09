/*****************************/
/* many changes by A. Hagert */
/*****************************/

/* From the std. library: */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <qinputdialog.h>
#include <qmessagebox.h>

#include "openGL.h"
#include "VLTools.h"

#include <unistd.h>   /* Interprozesskommunikation */
#include <signal.h>

//#include <time.h>

VAttrList graph_list;

typedef struct MyStruct_short {   /* added by A.Hagert: struct for the short graph */
  VNodeBaseRec base;
  VShort type;
  VShort col;
  VShort row; 
  VShort band;
  VShort label;
  VImage image;
} MyNodeRec_short, *MyNode_short;

typedef struct MyNodeStruct_float {   /* added by A.Hagert: struct for the float graph */
  VNodeBaseRec base;
  VFloat type;
  VFloat col;
  VFloat row; 
  VFloat band;
  VFloat label;
  VImage image;
} MyNodeRec_float, *MyNode_float;

VGraph gsrc;
FILE *graph_file;
VAttrListPosn posn;
float zoomfactor = 1.1;
int **coltab, anz = 0;                /* added by A. Hagert: colortable - [3] - rgb, collabel - needed for "holes" in the table, anz=size of colortable */
float *collabel;
float maxcolor[4], mincolor[4];       /* added by A. Hagert: max. and min. label of the graph (=max. & min. color) - for up to 4 graphs */
QColor *pos_farbe, *neg_farbe;        /* added by A. Hagert: colortables */
double middle[3];
float lastx=0, lasty=0;
int add_band = 0, add_row = 0, add_col = 0;

static GLint fogMode;

/* globals */
static GLfloat tb_lastposition[3], current_position[3];

static GLfloat tb_angle = 0.0;//, tb_angle_alt = 0.0; /* for mouse_rotation */
static GLfloat tb_axis[3];//, tb_axis_alt[3];         /* for mouse_rotation */
float xRot_temp, yRot_temp, zRot_temp;
int button = 0, pos_last_x = 0, pos_last_y = 0, pos_start_x = 0, pos_start_y = 0;

float /*int*/ sX, sY;   /* added by A.Hagert: int->float for the float-graphs */
int number_of_nodes=0;

/* added by A. Hagert: intern colortable - used for colortable "8" - but replaced by "setHsv()" */
/*void hsi2rgb(double label, float minimum, float maximum, double saturation, double intensity, double *r, double *g, double *b)
{
  double h,s,v;
  double f,p,q,t;
  int i;

  h = 40 + 320 * (label-minimum) / (maximum-minimum);  // hue - leave the first 40 colors -> minium
  s = saturation;  v = intensity;
  if (s > 1) s = 1;  if (s < 0) s = 0;
  if (v > 1) v = 1;  if (v < 0) v = 0;
  *r = *g = *b = 0;
  if (s == 0) {
    if (h < 0 || h > 360) {
      *r = intensity;      *g = intensity;      *b = intensity;
    }
    return;
  }
  if (h == 360) h = 0;
  h /= (double) 60.0; 
  i = (int) floor(h); 
  f = h - i;  
  p = v * (1.0 - s);  //89= v * (1.0 - s * f);  
  t = v * (1.0 - (s * (1.0 - f)));

  switch (i) {
    case 0: *r = v;    *g = t;    *b = p;    break;
    case 1: *r = q;    *g = v;    *b = p;    break;
    case 2: *r = p;    *g = v;    *b = t;    break;
    case 3: *r = p;    *g = q;    *b = v;    break;
    case 4: *r = t;    *g = p;    *b = v;    break;
    case 5: *r = v;    *g = p;    *b = q;    break;
    default: fprintf(stderr," error in hsi2rgb\n");
  }
}*/

void create_colortables(int coltype) {

  pos_farbe = (QColor *) malloc(sizeof(QColor) * 256);
  neg_farbe = (QColor *) malloc(sizeof(QColor) * 256);  

  /* 10 colortables ... */
  if (coltype==0)
    for (int i=0; i<256; i++ ) {
      if (i<128) {
	pos_farbe[i]=qRgb(255,i*2,i);
	neg_farbe[255-i]=qRgb(i/2,i*2,255);
      }
      else {
	pos_farbe[i]=qRgb(255,255,i);	  
	neg_farbe[255-i]=qRgb(i*2,255,255);
      }
    }
  else if (coltype==1)
    for (int i=0; i<256; i++ ) {
      if (i<190) {
	pos_farbe[255-i]=qRgb(255,(int)rint(i*0.664921*2),(int)rint(i*0.664921)); // [255,    254..0, 127..0]
	neg_farbe[i]=qRgb((int)rint(i*0.664921),(int)rint(i*0.664921*2),255);     // [0..127, 0..254, 255   ]
      }
      else {
	pos_farbe[255-i]=qRgb(255,255,(int)rint(i*1.953846+-245.184601)); // [255, 255, 255..128]
	neg_farbe[i]=qRgb((int)rint(i*1.953846+-245.184601),255,255);     // [128..255, 255, 255]
      }
    }
  else if (coltype==2)
    for (int i=0; i<512; i++ ) {
      if (i<256) 
	pos_farbe[i].setHsv( int(i*0.5), 255,  255);
      else
	neg_farbe[i-256].setHsv( int(i*0.5), 255,  255);
    }
  else if (coltype==3)
    for (int i=0; i<512; i++ ) {
      if (i<256)
	pos_farbe[255-i].setHsv( int(i*0.47), 255,  255);
      else
	neg_farbe[255-(i-256)].setHsv( int(i*0.47), 255,  255);
    }
  else if (coltype==4)
    for (int i=0; i<256; i++ ) {
      /* "alte" Tabelle */
      /*if (i<256) {
	pos_farbe[i].setHsv( (double)(360/80) * (i*20/256), 255,  255);
      } else {
	neg_farbe[i-256].setHsv( (double)(360/80) * (i*20/256), 255,  255);
      }*/
      if (i<64) {
	pos_farbe[i]=qRgb(i*2,i*4,255);
	neg_farbe[255-i]=qRgb(i*2,i*4,255);
      }
      else
	if (i<128) {
	  pos_farbe[i]=qRgb(i*2,255,255);
	  neg_farbe[255-i]=qRgb(i*2,255,255);
	}
	else
	  if (i<192) {
	    pos_farbe[i]=qRgb(255,255,511-2*i);
	    neg_farbe[255-i]=qRgb(255,255,511-2*i);
	  }
	  else {
	    pos_farbe[i]=qRgb(255,1023-4*i,511-2*i);
	    neg_farbe[255-i]=qRgb(255,1023-4*i,511-2*i);
	  }
    }
  else if (coltype==5)
    for (int i=0; i<256/*512*/; i++ ) {
      /* "alte" Tabelle */
      /*if (i<256) {
	pos_farbe[255-i].setHsv( (double)(360/100) * (i*20/256), 255,  255);
      } else {
	neg_farbe[255-(i-256)].setHsv( (double)(360/100) * (i*20/256), 255,  255);
     }*/
     pos_farbe[255-i].setHsv( (int)(i*300/255), 255,  255); /* lasse 360-355 weg */
     neg_farbe[i].setHsv( (int)(i*300/255), 255,  255);
  }
  else if (coltype==6)
    for (int i=0; i<512; i++ ) {
      if (i<256)
	neg_farbe[255-i].setHsv(/*(360/20) * (i*20/256)*/ int(i*1.41), 255,  255);
      else
	pos_farbe[255-(i-256)].setHsv( /*(360/20) * (i*20/256)*/int(i*1.41), 255,  255);
    }
  else if (coltype==7)
    for (int i=0; i<512; i++ ) {
      if (i<256)
	pos_farbe[255-i].setHsv(/*(360/40) * (i*20/256)*/ int(i*0.706), 255,  255);
      else
	neg_farbe[255-(i-256)].setHsv(/*(360/40) * (i*20/256)*/int(i*0.706), 255,  255);
    }
  else if (coltype==8)
    for (int i=0; i<256; i++ ) {
      pos_farbe[i].setHsv( (int)40 + 320 * (i) / 255, 255,  255);
      neg_farbe[255-i].setHsv( (int)40 + 320 * (i) / 255, 255,  255);
    }
  else if (coltype==9) {
    int r, g, b;
    float j;
    for (int i=0; i<512; i++ ) {
      j = i*0.5;
      r=(int)j/32;
      g=(int)(j-r*32)/4;
      b=(int)(j-r*32-g*4);
      if (r>0) r=(int)((r+.5)*32);
      if (g>0) g=(int)((g+.5)*32);
      if (b>0) b=(int)((b+.5)*64);
      
      if (i<256) neg_farbe[i].setRgb( r, g, b );
      else pos_farbe[i-256].setRgb( r, g, b );
    }
  }
  else if (coltype==10)
    for (int i=0; i<512; i++ ) {
      if (i<256)
	pos_farbe[255-i].setHsv( /*(360/10) * (i*20/256)*/int(i*2.8125), 255,  255);
      else
	neg_farbe[255-(i-256)].setHsv( /*(360/10) * (i*20/256)*/int(i*2.8125), 255,  255);
    }
  else
    for (int i=0; i<512; i++ ) {
      if (i<256)
	//pos_farbe[255-i].setRgb(6.53*(i*20/256),6.53*(i*20/256),6.53*(i*20/256));
	pos_farbe[255-i].setRgb(int(0.5*i),int(0.5*i),int(0.5*i));
      else
	//neg_farbe[255-(i-256)].setRgb(6.53*(i*20/256),6.53*(i*20/256),6.53*(i*20/256));
	neg_farbe[255-(i-256)].setRgb(int(0.5*i),int(0.5*i),int(0.5*i));
    }
}

void read_extern_colortable(VString col_tab_file, int graphtype) {
  char c;
  FILE *f;
  int i, r, g, b, n;
  float k;

  f = fopen (col_tab_file, "r");
  if (!f) { printf("wrong filename of colortable, using intern ones\n"); col_tab_file = "";}

  anz = 0;
  while ( (c=fgetc(f))!=EOF )
    if (c=='\n') anz++;  /* counts the lines */
  fclose (f);

  collabel = new (float [anz]);

  f = fopen (col_tab_file, "r");
  char line[255];
  if (graphtype == 0) { /* short-graph */
    for (n=0; n<anz; n++) {
      fscanf (f, "%i	%i	%i	%i\n", &i, &r, &g, &b);
      collabel[n] = (float)i;
    }
  }
  else { /* float-graph */
    n=0;
    while ( fgets ( line, sizeof(line), f ) != NULL ) {
      sscanf ( line, "%f	%i	%i	%i\n", &k, &r, &g, &b );
      collabel[n] = k;
      n++;
    }
  }
  fclose (f);

  coltab = new (int* [anz]); /* array of pointer */
  for (n = 0; n < anz; n++)
    coltab[n] = new (int [3]); /* coltab -> rgb of the labels */

  for (n=0; n<anz; n++) /* "holes" are set to white */
  {
    coltab[n][0] = 127;
    coltab[n][1] = 127;
    coltab[n][2] = 127;
  }

  f = fopen (col_tab_file, "r");
  if (graphtype == 0)
    for (n=0; n<anz; n++)
    {
      fscanf(f, "%i	%i	%i	%i\n", &i, &r, &g, &b);

      coltab[n][0] = r;
      coltab[n][1] = g;
      coltab[n][2] = b;
    }
  else { /* float-graph */
    n=0;
    while ( fgets ( line, sizeof(line), f ) != NULL ) {
      sscanf ( line, "%f	%i	%i	%i\n", &k, &r, &g, &b );
      coltab[n][0] = r;
      coltab[n][1] = g;
      coltab[n][2] = b;
      n++;
    }
  }
  fclose (f);
}

void get_color_from_intern_colortable(float label, float minimum, float maximum, int coltype, double *r, double *g, double *b, int bg)     /* definition of the colortables */
{
  int value=0;
    
  if (minimum != maximum)
  {
    if (label >= 0) {
      if (minimum > 0) value = (int)rint(255*(label-minimum) / (maximum-minimum));
      else value = (int)rint(255*(label-0) / (maximum-0));
    }
    else {
      if (maximum < 0) value = (int)rint(255*(label-minimum) / (maximum-minimum));
      else value = (int)rint(255*(label-minimum) / (0-minimum));
    }
   
    if (label>0) 
    { 
      if (label>maximum && maximum<0) {
        *r = neg_farbe[255].red()/255.0;
        *g = neg_farbe[255].green()/255.0;
        *b = neg_farbe[255].blue()/255.0;
      }
      else if (label>maximum) {
             *r = pos_farbe[255].red()/255.0;
             *g = pos_farbe[255].green()/255.0;
             *b = pos_farbe[255].blue()/255.0;
           }
           else if (label<minimum) {
                  *r = pos_farbe[0].red()/255.0;
                  *g = pos_farbe[0].green()/255.0;
                  *b = pos_farbe[0].blue()/255.0;
                }
                else
                {
                  *r = pos_farbe[(int)rint(value)].red()/255.0;
                  *g = pos_farbe[(int)rint(value)].green()/255.0;
                  *b = pos_farbe[(int)rint(value)].blue()/255.0;
                }
    }
    else
    {
      if (label<minimum && minimum>0) {
        *r = pos_farbe[0].red()/255.0;
        *g = pos_farbe[0].green()/255.0;
        *b = pos_farbe[0].blue()/255.0;
      }
      else if (label>maximum) {
             *r = neg_farbe[255].red()/255.0;
             *g = neg_farbe[255].green()/255.0;
             *b = neg_farbe[255].blue()/255.0;
           }
           else if (label<minimum) {
                  *r = neg_farbe[0].red()/255.0;
                  *g = neg_farbe[0].green()/255.0;
                  *b = neg_farbe[0].blue()/255.0;
                }
                else
                {
                  *r = neg_farbe[(int)rint(value)].red()/255.0;
                  *g = neg_farbe[(int)rint(value)].green()/255.0;
                  *b = neg_farbe[(int)rint(value)].blue()/255.0;
                }
    }

    if (label == 0) {
      if (bg == 0) { *r = 1.0; *g = 1.0; *b = 1.0; }
      else { *r = 0.0; *g = 0.0; *b = 0.0; }
    }  // for all tables: 0 = white
  }
  else {
    if (bg == 0) { *r = 1.0; *g = 1.0; *b = 1.0; }
    else { *r = 0.0; *g = 0.0; *b = 0.0; }
  }
} /* end of definition of the colortables */

void get_color_from_extern_colortable(float label, double *r, double *g, double *b)     /* definition of the colortables */
{
  short ok = 0;
  for (int i = 0; i < anz; i++)
    if ((int)(label*100.0) == (int)(collabel[i]*100.0)) { /* Uebereinstimmung "nur" auf 2 Kommastellen */
      *r = (float)coltab[i][0];
      *g = (float)coltab[i][1];
      *b = (float)coltab[i][2];
      ok = 1;
      //printf("%f =? %f || %f, %f, %f (%i)\n", label, collabel[i], *r, *g, *b, i);
    }

  if (ok == 0) {
    *r = 127;
    *g = 127;
    *b = 127;
  }
  //printf("%f -> %f, %f, %f\n", label, *r, *g, *b);
}

/* functions */
   static void _tbPointToVector(float x, float y, int width, int height, float v[3]) /* activated by A.Hagert:  for mouse-ation */
   {
   float d, a;

   // project x, y onto a hemi-sphere centered within width, height.
   v[0] = (2.0 * x - width) / width;
   v[1] = (height - 2.0 * y) / height;
   d = sqrt(v[0] * v[0] + v[1] * v[1]);
   v[2] = cos((3.14159265 / 2.0) * ((d < 1.0) ? d : 1.0));
   a = 1.0 / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
   v[0] *= a;
   v[1] *= a;
   v[2] *= a;
   }
/**/

MyGLDrawer::MyGLDrawer( QWidget *parent, const char *name, VImage *src_, VImage *fnc_, prefs *pr_, double ppp, double nnn, double *ca_, double *cp_, double *extent_, int ifile )
  : QGLWidget(parent,name), src(src_), fnc(fnc_), pr(pr_) , ca(ca_), cp(cp_), extent(extent_), ifile_m(ifile)
{
  rows=VImageNRows(src[0]);
  columns=VImageNColumns(src[0]);
  bands=VImageNFrames(src[0]);

  if (pr->verbose==2) fprintf(stderr,"Starting MyGLDrawer...\n");
  pr->picture[4]=0;

  xRot = 0.0;
  yRot = 180.0;
  zRot = 180.0;		// default object rotation
  scale = 1.25;		// default object scale
  color_onoff = 1;	// A. Hagert - paramater to switch between colored and s/w view
  lines_onoff = 1;	// A. Hagert - parameter to switch between lined and pointed visualization
  polygons_onoff = 1;	// A. Hagert - parameter to turn on/off polygons
  fog_onoff = 0;	// A. Hagert - paramater to switch between colored and s/w view
  col_tab_file = NULL;  // A. Hagert - colortable for graph
  crosses_onoff = 1;    // A. Hagert - single points are crosses
  lastcolor[0]=-1; lastcolor[1]=-1; lastcolor[2]=-1;
  object = 0, object2_0 = 0, object2_1 = 0, object3 = 0, object_2nd_graph = 0;
  //double vgroesse=width();
  //double hgroesse=height();
  xa=ya=za=0.0;
  move_total_x = 0;
  move_total_y = 0;

  if (fnc[0]) {
    fnc_rows=VImageNRows(fnc[0]);
    fnc_columns=VImageNColumns(fnc[0]);
    fnc_bands=VImageNFrames(fnc[0]);
  } else {
    fnc_rows=0;
    fnc_columns=0;
    fnc_bands=0;
  }
  posfarbe = (QColor *) malloc(sizeof(QColor) * 21);
  negfarbe = (QColor *) malloc(sizeof(QColor) * 21);

  ppmax=ppp;
  nnmax=nnn;
}

void MyGLDrawer::clean()  /* added by A.Hagert: clears the memory by reload of the graph (options colors/links/crosses) */
{
  glDeleteLists( object, 1 );
  glDeleteLists( object_2nd_graph, 1 );
  if (pr->graph[ifile_m]) { glDeleteLists( object2_0, 1 ); glDeleteLists( object2_1, 1 ); }
  if (fnc[0]) glDeleteLists( object3, 1 );
  if (col_tab_file)  // array with the colortable
  {
    for (int n = 0; n < anz; n++)
      delete [] coltab[n];
    delete [] coltab;
    delete [] collabel;
  }
  delete [] pos_farbe;
  delete [] neg_farbe;
}

MyGLDrawer::~MyGLDrawer()
{
  glDeleteLists( object, 1 );
  glDeleteLists( object_2nd_graph, 1 );
  if (pr->graph[ifile_m]) { glDeleteLists( object2_0, 1 ); glDeleteLists( object2_1, 1 ); }
  if (fnc[0]) glDeleteLists( object3, 1 );

  /* added by A.Hagert: "new" things to clean */
  if (col_tab_file)  // array with the colortable
  {
    for (int n = 0; n < anz; n++)
      delete [] coltab[n];
    delete [] coltab;
    delete [] collabel;
  }
  delete [] pos_farbe;
  delete [] neg_farbe;
}

void MyGLDrawer::initializeGL() 
{
  // Set up the rendering context, define display lists etc.:
  if (pr->hgfarbe[0]==1) {
    qglClearColor( white ); 		// Let OpenGL clear to white
  } else {
    qglClearColor( black ); 		// Let OpenGL clear to black
  }
  //glShadeModel( GL_SMOOTH );
  
  
  fogMode=GL_LINEAR;

  GLfloat position[] = { 0.0, 0.0, 0.0, 0.0 };

  glEnable(GL_DEPTH_TEST);

  glLightfv(GL_LIGHT0, GL_POSITION, position);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  
  if (pr->fog==0)
    glDisable(GL_FOG);
  else
    glEnable(GL_FOG);

  {
    GLfloat fog2Color[4] = { 1.0, 1.0, 1.0, 0.0};
    GLfloat fogColor[4] = { 0.0, 0.0, 0.0, 0.0};
    //fogMode=GL_EXP;
    glFogi(GL_FOG_MODE, fogMode);

    if (pr->hgfarbe[0]==1)
      glFogfv(GL_FOG_COLOR, fog2Color);
    else
      glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.10);  // original 0.10
    glHint(GL_FOG_HINT, GL_DONT_CARE);
    //glHint (GL_FOG_HINT, GL_NICEST);  /*  per pixel   */
    /*glFogf(GL_FOG_START, 9.0);  // Originale Werte
    glFogf(GL_FOG_END, 10.5);*/
    glFogf(GL_FOG_START, 10.0);
    glFogf(GL_FOG_END, 11.0);
  }
  //glClearColor(0.0, 0.0, 0.0, 1.0);

  if (pr->verbose==2) fprintf(stderr,"Initializing MyGLDrawer...\n");

  if (col_tab_file)  // added by A. Hagert -> get the colortable
    read_extern_colortable(col_tab_file, graphtype);
  else
    create_colortables ( pr->gcoltype );
  if (pr->graph[ifile_m]) { 
    object2_0 = makeObject2(0);		// Generate an OpenGL display list
    object2_1 = makeObject2(1);
  }
  object = makeObject(/*0*/ifile_m);	// Generate an OpenGL display list
  if (pr->verbose==2) fprintf(stderr,"Successfully Created Graph Object...\n");

  //if (pr->graph[ifile_m]) { object2_0 = makeObject2(0);		// Generate an OpenGL display list
  //                          object2_1 = makeObject2(1); }
  if (fnc[0]) object3 = makeObject3();		// Generate an OpenGL display list
  //}


}

void MyGLDrawer::resizeGL( int w, int h ) 
{
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  /*  if (width()>height()) {
    //glViewport( 0, 0, (GLsizei)w, (GLsizei)h );
    glViewport( 0, 0, (GLsizei)height(), (GLsizei)height() );
  } else {
    glViewport( 0, 0, (GLsizei)width(), (GLsizei)width() );
  }
  */

  glViewport( 0, 0, (GLsizei)width(), (GLsizei)height() );
  zoom(0.0);
  //glFrustum( -zoomfactor, zoomfactor, -zoomfactor*(GLfloat)height()/(GLfloat)width(), zoomfactor*(GLfloat)height()/(GLfloat)width(), 5.0, 15.0 );				/* by A. Hagert */
  //glFrustum( -1.0, 1.0, -1.0*(GLfloat)h/(GLfloat)w, 1.0*(GLfloat)h/(GLfloat)w, 5.0, 15.0 );

  /*  if (w<=h)
    glOrtho (-1.0, 1.0, -1.0*(GLfloat)h/(GLfloat)w, 1.0*(GLfloat)h/(GLfloat)w, 5.0, 15.0);
  else
    glOrtho (-1.0*(GLfloat)w/(GLfloat)h, 1.0*(GLfloat)w/(GLfloat)h, -1.0, 1.0, 5.0, 15.0);
  */
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
}

void MyGLDrawer::paintGL()
{

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glLoadIdentity();
  glTranslatef( 0.0, 0.0, -10.0 );

  glRotatef( xRot, 1.0, 0.0, 0.0 ); 
  glRotatef( yRot, 0.0, 1.0, 0.0 );
  glRotatef( zRot, 0.0, 0.0, 1.0 );
  //glRotatef(tb_angle, tb_axis[0], tb_axis[1], tb_axis[2]); // for mouse - rotation
  glCallList( object ); // the graph
  //if (ifile_m == 1) glCallList(object_2nd_graph);
  if (pr->graph[ifile_m]) { 
    if (pr->crossize == 0) glCallList( object2_0 ); // the cross
    else glCallList( object2_1 );
  }
  if (fnc[0] && pr->picture[4]) glCallList( object3 ); // functional data
  glFlush();
}

void MyGLDrawer::crossChange()
{
  if (pr->showcross) { 
    object2_0 = makeObject2(0);		// Generate an OpenGL display list
    object2_1 = makeObject2(1); 
  }
  else { object2_0 = 0; object2_1 = 0; }
  updateGL();
}

/*!
  Generate an OpenGL display list for the object to be shown, i.e. the box

  read the vista graph file and write it to a openGL object
*/

GLuint MyGLDrawer::makeObject(int nr) {
  GLuint list;
  GLfloat mat[3];
  int counter = 0, firstnode=0;
  int max_band = 0, min_band = 0, max_row = 0, min_row = 0, max_col = 0, min_col = 0;

  /* falls wir Polygone haben ... */
  VImage p_image=NULL;
  int p_nr=0, p_points=0;  /* Anzahl der Polygone, groesse der Polygone (3, 4, ...?) */
  //short *p_coo;    /* dynamisches feld mit den nummern der punkte und label */

  //double timer = (double)clock(), timer1;
  //printf ("started loading at a total time of %f seconds\n", (double)timer/CLOCKS_PER_SEC);

  glPointSize( 1.5 );
  list = glGenLists( 1 );
  glNewList( list, GL_COMPILE );
  if (pr->hgfarbe[0]==1) {
    qglColor( black );		      // Shorthand for glColor3f or glIndex
    mat[0] = 0.0; mat[1] = 0.0; mat[2] = 0.0;
  } else {
    qglColor( white );
    mat[0] = 1.0; mat[1] = 1.0; mat[2] = 1.0;
  }
  //glMaterialfv(GL_FRONT, GL_EMISSION, mat);
  glLineWidth( 1.1 );    /* changed by A.Hagert: originally 2 */
  // Graph file einlesen
  int graph_objects=0;
  if (pr->graph[nr])
  {
    graph_file = VOpenInputFile (pr->graph[nr], TRUE);
    if (! (graph_list = VReadFile (graph_file, NULL))) exit(1);

    for (VFirstAttr (graph_list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VGraphRepn) continue;
      VGetAttrValue (& posn, NULL, VGraphRepn, & gsrc);
      graph_objects++;

      /* mal schauen, ob da noch ein Bild drin ist (-> Polygone statt Punkte/Linien ): */
      VGetAttr(VGraphAttrList(gsrc), "image", NULL, VImageRepn, & p_image);
      /* rows -> Anzahl der Polygone, columns -> welcher Punkt gehoert zum Polygon + label, bands -> keine Aussage */
      if (p_image != NULL && polygons_onoff == 1) {
	p_nr = VImageNRows(p_image);
	p_points = VImageNColumns(p_image);
      }
    }
    if (VNodeRepn(gsrc) == VFloatRepn) graphtype = 1;
    else graphtype = 0;

    if (graph_objects==0)
      VError("Specified sulcus file is not of type graph");

    MyNode_short node_s;
    MyNode_short node1_s;  /* added by A.Hagert: for short-graphs */
    MyNode_float node_f;
    MyNode_float node1_f;  /* added by A.Hagert: for float-graphs */
    VAdjacency neighb;
    VNodeBaseRec base;
    double r=0.0, g=0.0, b=0.0;  /* added by A.Hagert: r/g/b - red/green/blue */
    short *lonely_nodes;

    maxslist=gsrc->lastUsed;
    lonely_nodes = (short*) VMalloc(sizeof(short) * gsrc->lastUsed+1);
    //lonely_nodes = (short*) VMalloc(sizeof(short) * gsrc->lastUsed+1);
    slist1 = (GLfloat *) VMalloc(sizeof(GLfloat) * gsrc->lastUsed+1);
    slist2 = (GLfloat *) VMalloc(sizeof(GLfloat) * gsrc->lastUsed+1);
    slist3 = (GLfloat *) VMalloc(sizeof(GLfloat) * gsrc->lastUsed+1);
    slist4 = (GLfloat *) VMalloc(sizeof(GLfloat) * gsrc->lastUsed+1);
    for (int i = 0; i <= gsrc->lastUsed; i++) {
      lonely_nodes[i]=0;
      slist1[i] = -1234.123450;
      slist2[i] = -1234.123450;
      slist3[i] = -1234.123450;
      slist4[i] = -1234.123450;
    }
    number_of_nodes=gsrc->lastUsed;

    /* added by A.Hagert: find minimum and maximum label of the graph */
    if ((pr->col_min == 0) && (pr->col_max == 0)) {
      if (graphtype == 0)       /* short-graph */
        for (int i=1; i<=gsrc->lastUsed; i++) {
          node_s = (MyNode_short) VGraphGetNode (gsrc,i);
          if (node_s == NULL) continue;

          if (firstnode == 0) { firstnode = 1; mincolor[nr] = node_s->label; maxcolor[nr] = node_s->label; }
          if (node_s->label < mincolor[nr]) mincolor[nr] = node_s->label;
          if (node_s->label > maxcolor[nr]) maxcolor[nr] = node_s->label;

          if (max_band == 0 && min_band == 0 && max_row == 0 && min_row == 0 && max_col == 0 && min_col == 0) {
            max_band = node_s->band;
	    min_band = node_s->band;
	    max_row = node_s->row;
	    min_row = node_s->row;
	    max_col = node_s->col;
	    min_col = node_s->col;
          }

          if (node_s->band > max_band) max_band = node_s->band;
          if (node_s->row  > max_row)  max_row  = node_s->row;
          if (node_s->col  > max_col)  max_col  = node_s->col;
          if (node_s->band < min_band) min_band = node_s->band;
          if (node_s->row  < min_row)  min_row  = node_s->row;
          if (node_s->col  < min_col)  min_col  = node_s->col;
        }
      else                     /* float-graph */
        for (int i=1; i<=gsrc->lastUsed; i++) {
          node_f = (MyNode_float) VGraphGetNode (gsrc,i);
          if (node_f == NULL) continue;

          if (firstnode == 0) { firstnode = 1; mincolor[nr] = node_f->label; maxcolor[nr] = node_f->label; }
          if (node_f->label < mincolor[nr]) mincolor[nr] = node_f->label;
          if (node_f->label > maxcolor[nr]) maxcolor[nr] = node_f->label;
	  
	  if (max_band == 0 && min_band == 0 && max_row == 0 && min_row == 0 && max_col == 0 && min_col == 0) {
            max_band = (int)ceil(node_f->band);
	    min_band = (int)node_f->band;
	    max_row = (int)ceil(node_f->row);
	    min_row = (int)node_f->row;
	    max_col = (int)ceil(node_f->col);
	    min_col = (int)node_f->col;
          }

          if (node_f->band > max_band) max_band = (int)ceil(node_f->band);
          if (node_f->row  > max_row)  max_row  = (int)ceil(node_f->row);
          if (node_f->col  > max_col)  max_col  = (int)ceil(node_f->col);
          if (node_f->band < min_band) min_band = (int)node_f->band;
          if (node_f->row  < min_row)  min_row  = (int)node_f->row;
          if (node_f->col  < min_col)  min_col  = (int)node_f->col;
        }
	if (pr->infilenum_graph != 0) { /* also nur, wenn keine Anatomie geladen wird */
	  add_col = -1*min_col;
	  add_row = -1*min_row;
	  add_band = -1*min_band;
	}
    }
    else {
      for  (int i=0; i<=ifile_m; i++) {
        mincolor[i] = pr->col_min;
        maxcolor[i] = pr->col_max;
      }
      if (graphtype == 0)       /* short-graph */
        for (int i=1; i<=gsrc->lastUsed; i++) {
          node_s = (MyNode_short) VGraphGetNode (gsrc,i);
          if (node_s == NULL) continue;

          if (max_band == 0 && min_band == 0 && max_row == 0 && min_row == 0 && max_col == 0 && min_col == 0) {
            max_band = node_s->band;
	    min_band = node_s->band;
	    max_row = node_s->row;
	    min_row = node_s->row;
	    max_col = node_s->col;
	    min_col = node_s->col;
          }

          if (node_s->band > max_band) max_band = node_s->band;
          if (node_s->row  > max_row)  max_row  = node_s->row;
          if (node_s->col  > max_col)  max_col  = node_s->col;
          if (node_s->band < min_band) min_band = node_s->band;
          if (node_s->row  < min_row)  min_row  = node_s->row;
          if (node_s->col  < min_col)  min_col  = node_s->col;
        }
      else                     /* float-graph */
        for (int i=1; i<=gsrc->lastUsed; i++) {
          node_f = (MyNode_float) VGraphGetNode (gsrc,i);
          if (node_f == NULL) continue;

	  if (max_band == 0 && min_band == 0 && max_row == 0 && min_row == 0 && max_col == 0 && min_col == 0) {
            max_band = (int)ceil(node_f->band);
	    min_band = (int)node_f->band;
	    max_row = (int)ceil(node_f->row);
	    min_row = (int)node_f->row;
	    max_col = (int)ceil(node_f->col);
	    min_col = (int)node_f->col;
          }

          if (node_f->band > max_band) max_band = (int)ceil(node_f->band);
          if (node_f->row  > max_row)  max_row  = (int)ceil(node_f->row);
          if (node_f->col  > max_col)  max_col  = (int)ceil(node_f->col);
          if (node_f->band < min_band) min_band = (int)node_f->band;
          if (node_f->row  < min_row)  min_row  = (int)node_f->row;
          if (node_f->col  < min_col)  min_col  = (int)node_f->col;
        }
	if (pr->infilenum_graph != 0) { /* also nur, wenn keine Anatomie geladen wird */
	  add_col = -1*min_col;
	  add_row = -1*min_row;
	  add_band = -1*min_band;
	}
    }

    emit got_color_min_max(mincolor[0], maxcolor[0]);
    /* alle Punkte speichern ... */
    if (graphtype == 0)  /* added by A.Hagert: short */
      for (int i=1; i<=gsrc->lastUsed; i++)
      {
        node_s = (MyNode_short) VGraphGetNode (gsrc,i);
        if (node_s == NULL) continue;

        slist1[i]=node_s->col;
        slist2[i]=node_s->row;
        slist3[i]=node_s->band;
	slist4[i]=node_s->label;
      }
    else
      for (int i=1; i<=gsrc->lastUsed; i++)
      {
        node_f = (MyNode_float) VGraphGetNode (gsrc,i);
        if (node_f == NULL) continue;

        slist1[i]=node_f->col;
        slist2[i]=node_f->row;
        slist3[i]=node_f->band;
	slist4[i]=node_f->label;
      }


    if (graphtype == 0)  /* added by A.Hagert: short */
    {
      for (int i=1; i<=gsrc->lastUsed; i++)
      {
        node_s = (MyNode_short) VGraphGetNode (gsrc,i);
        if (node_s == NULL) continue;

        base   = node_s->base;
        neighb = base.head;

        if (neighb == NULL)  /* added by A.Hagert: nodes without links */
	//{
	  if (lonely_nodes[i]==0) lonely_nodes[i] = 1; /* marked as single point */
	  //printf("%i is lonely: marked as: %i\n", i, lonely_nodes[i]);
	//}

        counter = 0;
	if (lines_onoff == 1) glBegin( GL_LINE_LOOP );  /* added by A.Hagert: links are "on"  */
        while (neighb != NULL) {  /* nodes with links */
	  node1_s = (MyNode_short) VGraphGetNode(gsrc,neighb->id);
	  lonely_nodes[neighb->id] = 2;
	  //intf("masked %i (-> %i), (%f, %f, %f)\n", i, neighb->id, slist1[i], slist2[i], slist3[i]);
	  counter++;
	  if (lines_onoff == 1) {
	    if (p_image == NULL || polygons_onoff == 0) {
	      if (color_onoff == 1) {  /* added by A.Hagert: like above ... */
	        if (col_tab_file)
	        {
	          get_color_from_extern_colortable(node_s->label, &r, &g, &b);
	          mat[0] = (GLfloat) r/255.0; mat[1] = (GLfloat) g/255.0; mat[2] = (GLfloat) b/255.0;
	        }
	        else  /* added by A.Hagert: intern colortable */
	        {
	          get_color_from_intern_colortable(node_s->label, mincolor[nr], maxcolor[nr], pr->gcoltype, &r, &g, &b, pr->hgfarbe[0]);
	          mat[0] = (GLfloat) r; mat[1] = (GLfloat) g; mat[2] = (GLfloat) b;
	        }
	      }
	      glMaterialfv(GL_FRONT, GL_EMISSION, mat);
	      glVertex3f( (slist1[i]+add_col-((float)columns/2.0))/50.0, (slist2[i]+add_row-((float)rows/2.0))/50.0, (slist3[i]+add_band-((float)bands/2.0))/50.0 );  /* added by A.Hagert: "the point before" - if links are "on" */

	      if (color_onoff == 1) {  /* added by A.Hagert: like above ... */
	        if (col_tab_file)
	        {
	          get_color_from_extern_colortable(node1_s->label, &r, &g, &b);
	          mat[0] = (GLfloat) r/255.0; mat[1] = (GLfloat) g/255.0; mat[2] = (GLfloat) b/255.0;
	        }
	        else  /* added by A.Hagert: intern colortable */
	        {
	          get_color_from_intern_colortable(node1_s->label, mincolor[nr], maxcolor[nr], pr->gcoltype, &r, &g, &b, pr->hgfarbe[0]);
	          mat[0] = (GLfloat) r; mat[1] = (GLfloat) g; mat[2] = (GLfloat) b;
	        }
	      }
	      glMaterialfv(GL_FRONT, GL_EMISSION, mat);
	      glVertex3f( (node1_s->col+add_col-((float)columns/2.0))/50.0, (node1_s->row+add_row-((float)rows/2.0))/50.0, (node1_s->band+add_band-((float)bands/2.0))/50.0 );  /* added by A.Hagert: "the point before" - if links are "on" */
	    }
          }
	  neighb = neighb->next;
        }
	if (lines_onoff == 1) glEnd();
      }
      /* added by. A. Hagert: saemtliche Einzelpunkte (bzw. Kreuze) */
      for (int i=1; i<=gsrc->lastUsed; i++)
      {
	if ((lonely_nodes[i]==1) || (lines_onoff==0 && crosses_onoff) || (lines_onoff == 0)) {
          if (color_onoff == 1) {
	    if (col_tab_file)
	    {
	      get_color_from_extern_colortable(slist4[i], &r, &g, &b);
	      mat[0] = (GLfloat) r/255.0; mat[1] = (GLfloat) g/255.0; mat[2] = (GLfloat) b/255.0;
	    }
	    else
	    {
	      get_color_from_intern_colortable((float)slist4[i], mincolor[nr], maxcolor[nr], pr->gcoltype, &r, &g, &b, pr->hgfarbe[0]);
	      mat[0] = (GLfloat) r; mat[1] = (GLfloat) g; mat[2] = (GLfloat) b;
	    }
	  }
	  if ( (p_image == NULL || polygons_onoff == 0) &&
	       (!(slist1[i] < -1234.1233 && slist1[i] > -1234.1235 &&  /* "Luecken" werden nicht mitgemalt */
	          slist2[i] < -1234.1233 && slist2[i] > -1234.1235 &&
	          slist3[i] < -1234.1233 && slist3[i] > -1234.1235 &&
	          slist4[i] < -1234.1233 && slist4[i] > -1234.1235)) ) {
	    if (crosses_onoff) glBegin( GL_LINES );
	    else {
	      // Kugeln
	      if (pr->openvis==2) {
		GLUquadric *quad=gluNewQuadric();
		glPushMatrix();
		glTranslatef((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
		glMaterialfv(GL_FRONT, GL_EMISSION, mat);    
		gluSphere(quad,(float)pr->spheresize/1000.0,8,8);
		//gluSphere(quad,0.02,8,8); 
		gluDeleteQuadric(quad);
		glPopMatrix();
	      }
	      glBegin( GL_POINTS );
	    }
	    glMaterialfv(GL_FRONT, GL_EMISSION, mat);
            glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
	    //printf("printed %i: (%f, %f, %f) -> (%f, %f, %f)\n", i, slist1[i],slist2[i],slist3[i], mat[0], mat[1], mat[2]);
	    if (crosses_onoff) {
	      glMaterialfv(GL_FRONT, GL_EMISSION, mat);
    	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0+0.035,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0-0.035,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
  	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0+0.035,(slist3[i]+add_band-((float)bands/2))/50.0);
	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
  	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0-0.035,(slist3[i]+add_band-((float)bands/2))/50.0);
	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
  	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0+0.035);
	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
  	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0-0.035);
	    }
	    else {
	      glMaterialfv(GL_FRONT, GL_EMISSION, mat);    
	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
	    }
	    glEnd();
	  }
	  //printf("printed: %i: (%f, %f, %f) -> %f\n", i, slist1[i], slist2[i], slist3[i], slist4[i]);
	}
	//else
	  //printf("NOT printed %i: (%f, %f, %f) -> (%f, %f, %f)\n", i, slist1[i],slist2[i],slist3[i], mat[0], mat[1], mat[2]);
      }
    }
    else  /* added by A.Hagert: float-graph - just like short, only other types and casts */
    {
      for (int i=1; i<=gsrc->lastUsed; i++)
      {
        node_f = (MyNode_float) VGraphGetNode (gsrc,i);
        if (node_f == NULL) continue;

        base   = node_f->base;
        neighb = base.head;

        if (neighb == NULL)  /* added by A.Hagert: nodes without links */
	  if (lonely_nodes[i]==0) lonely_nodes[i] = 1; /* marked as single point */

        counter = 0;
	if (lines_onoff == 1 && neighb != NULL)  glBegin( GL_LINE_LOOP );  /* added by A.Hagert: links are "on"  */
        while (neighb != NULL) {  /* nodes with links */
	  node1_f = (MyNode_float) VGraphGetNode(gsrc,neighb->id);
	  lonely_nodes[neighb->id] = 2;
	  counter++;
	  if (lines_onoff == 1) {
	    if (p_image == NULL || polygons_onoff == 0) {
	      if (color_onoff == 1) {  /* added by A.Hagert: like above ... */
	        if (col_tab_file)
	        {
	          get_color_from_extern_colortable(node_f->label, &r, &g, &b);
	          mat[0] = (GLfloat) r/255.0; mat[1] = (GLfloat) g/255.0; mat[2] = (GLfloat) b/255.0;
	        }
	        else  /* added by A.Hagert: intern colortable */
	        {
	          get_color_from_intern_colortable(node_f->label, mincolor[nr], maxcolor[nr], pr->gcoltype, &r, &g, &b, pr->hgfarbe[0]);
	          mat[0] = (GLfloat) r; mat[1] = (GLfloat) g; mat[2] = (GLfloat) b;
	        }
	      }
	      glMaterialfv(GL_FRONT, GL_EMISSION, mat);
	      glVertex3f( (slist1[i]+add_col-((float)columns/2.0))/50.0, (slist2[i]+add_row-((float)rows/2.0))/50.0, (slist3[i]+add_band-((float)bands/2.0))/50.0 );  /* added by A.Hagert: "the point before" - if links are "on" */

	      if (color_onoff == 1) {  /* added by A.Hagert: like above ... */
	        if (col_tab_file)
	        {
	          get_color_from_extern_colortable(node1_f->label, &r, &g, &b);
	          mat[0] = (GLfloat) r/255.0; mat[1] = (GLfloat) g/255.0; mat[2] = (GLfloat) b/255.0;
	        }
	        else  /* added by A.Hagert: intern colortable */
	        {
	          get_color_from_intern_colortable(node1_f->label, mincolor[nr], maxcolor[nr], pr->gcoltype, &r, &g, &b, pr->hgfarbe[0]);
	          mat[0] = (GLfloat) r; mat[1] = (GLfloat) g; mat[2] = (GLfloat) b;
	        }
	      }
	      glMaterialfv(GL_FRONT, GL_EMISSION, mat);
	      glVertex3f( (node1_f->col+add_col-((float)columns/2.0))/50.0, (node1_f->row+add_row-((float)rows/2.0))/50.0, (node1_f->band+add_band-((float)bands/2.0))/50.0 );  /* added by A.Hagert: "the point before" - if links are "on" */
	    }
          }
	  neighb = neighb->next;
        }
	if (lines_onoff == 1) glEnd();
      }
      /* added by. A. Hagert: saemtliche Einzelpunkte (bzw. Kreuze) */
      for (int i=1; i<=gsrc->lastUsed; i++)
      {
	if ((lonely_nodes[i]==1) || (lines_onoff==0 && crosses_onoff) || (lines_onoff == 0)) {
          if (color_onoff == 1) { 
	    if (col_tab_file)
	    {
	      get_color_from_extern_colortable(slist4[i], &r, &g, &b);
	      mat[0] = (GLfloat) r/255.0; mat[1] = (GLfloat) g/255.0; mat[2] = (GLfloat) b/255.0;
	    }
	    else
	    {
	      get_color_from_intern_colortable((float)slist4[i], mincolor[nr], maxcolor[nr], pr->gcoltype, &r, &g, &b, pr->hgfarbe[0]);
	      mat[0] = (GLfloat) r; mat[1] = (GLfloat) g; mat[2] = (GLfloat) b;
	    }
	  }
	  if ( (p_image == NULL || polygons_onoff == 0) &&
	       (!(slist1[i] < -1234.1233 && slist1[i] > -1234.1235 &&  /* "Luecken" werden nicht mitgemalt */
	          slist2[i] < -1234.1233 && slist2[i] > -1234.1235 &&
	          slist3[i] < -1234.1233 && slist3[i] > -1234.1235 &&
	          slist4[i] < -1234.1233 && slist4[i] > -1234.1235)) ) {
	    if (crosses_onoff) glBegin( GL_LINES );
	    else glBegin( GL_POINTS );
	    glMaterialfv(GL_FRONT, GL_EMISSION, mat);
	    glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
	    //printf("printed %i: (%f, %f, %f) -> (%f, %f, %f)\n", i, slist1[i],slist2[i],slist3[i], mat[0], mat[1], mat[2]);
	    if (crosses_onoff) {
	      glMaterialfv(GL_FRONT, GL_EMISSION, mat);
	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0+0.035,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0-0.035,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
  	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0+0.035,(slist3[i]+add_band-((float)bands/2))/50.0);
	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
  	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0-0.035,(slist3[i]+add_band-((float)bands/2))/50.0);
	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
  	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0+0.035);
	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
  	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0-0.035);
	    }
	    else {
	      glMaterialfv(GL_FRONT, GL_EMISSION, mat);
	      glVertex3f((slist1[i]+add_col-((float)columns/2.0))/50.0,(slist2[i]+add_row-((float)rows/2.0))/50.0,(slist3[i]+add_band-((float)bands/2))/50.0);
	    }
	    glEnd();
	  }
	  //printf("printed: %i: (%f, %f, %f) -> %f\n", i, slist1[i], slist2[i], slist3[i], slist4[i]);
	}
      }
    }

    /* by A. Hagert: keine Linien/Punkte, sondern Polygone */
    if (p_image != NULL && polygons_onoff == 1) {
      /* Versuch, sinvoll Transparenz zu erreicen... (bisher gescheitert) */
      //glEnable(GL_BLEND);
      //glEnable (GL_ALPHA_TEST);
      //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      //glDisable (GL_DEPTH_TEST);
      /*glClearDepth(1.0f);
      glDepthFunc(GL_ALWAYS);
      glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/
      if (graphtype == 0) { /* short */
        for (int k = 0; k < p_nr; k++) {
          if (color_onoff == 1) {
	    if (col_tab_file)
	    {
	      get_color_from_extern_colortable(VGetPixel (p_image, 0, k, p_points-1), &r, &g, &b);
	      mat[0] = (GLfloat) r/255.0; mat[1] = (GLfloat) g/255.0; mat[2] = (GLfloat) b/255.0;
	    }
	    else
	    {
	      get_color_from_intern_colortable(VGetPixel (p_image, 0, k, p_points-1), mincolor[nr], maxcolor[nr], pr->gcoltype, &r, &g, &b, pr->hgfarbe[0]);
	      mat[0] = (GLfloat) r; mat[1] = (GLfloat) g; mat[2] = (GLfloat) b;
	    }
	  }
	  glBegin(GL_POLYGON);
	  //glColor4f(mat[0],mat[1],mat[2],k);
	  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat);
	  for (int j = 0; j < p_points-1; j++) {
	    node_s = (MyNode_short) VGraphGetNode(gsrc, (int)VGetPixel (p_image, 0, k, j)+1);
	    glVertex3f((node_s->col+add_col-((float)columns/2.0))/50.0,(node_s->row+add_row-((float)rows/2.0))/50.0,(node_s->band+add_band-((float)bands/2))/50.0);
	  }
          glEnd();
        }
      }
      else { /* float */
        for (int k = 0; k < p_nr; k++) {
          if (color_onoff == 1) {
	    if (col_tab_file)
	    {
	      get_color_from_extern_colortable(VGetPixel (p_image, 0, k, p_points-1), &r, &g, &b);
	      mat[0] = (GLfloat) r/255.0; mat[1] = (GLfloat) g/255.0; mat[2] = (GLfloat) b/255.0;
	    }
	    else
	    {
	      get_color_from_intern_colortable(VGetPixel (p_image, 0, k, p_points-1), mincolor[nr], maxcolor[nr], pr->gcoltype, &r, &g, &b, pr->hgfarbe[0]);
	      mat[0] = (GLfloat) r; mat[1] = (GLfloat) g; mat[2] = (GLfloat) b;
	    }
	  }
	  glBegin(GL_POLYGON);
	  //mat[3] = 1.0;
	  /*if (mat[0] == 1.0 && mat[1] == 1.0 && mat[2] == 1.0) {
	    mat[0] = (double)rand()/6000000000.0; mat[1] = (double)rand()/6000000000.0; mat[2] = (double)rand()/6000000000.0;
	  }*/
	  /*if (mat[0] == 1.0 && mat[1] == 1.0 && mat[2] == 1.0) {
	    mat[0] = 0.6;  mat[1] = 0.6;  mat[2] = 0.6;
	  }*/
	  //printf("%i: %f, %f, %f\n", k, mat[0], mat[1], mat[2]);
	  glMaterialfv(GL_FRONT, GL_EMISSION, mat);
	  for (int j = 0; j < p_points-1; j++) {
	    //glColor4f(mat[0],mat[1],mat[2], 1.0f);
	    glMaterialfv(GL_FRONT, GL_EMISSION, mat);
	    node_f = (MyNode_float) VGraphGetNode(gsrc, (int)VGetPixel (p_image, 0, k, j)+1);
	    //printf("(%f, %f, %f)\n", mat[0], mat[1], mat[2]);
	    glVertex3f((node_f->col+add_col-(columns/2.0))/50.0,(node_f->row+add_row-(rows/2.0))/50.0,(node_f->band+add_band-(bands/2))/50.0);
	  }
	  glEnd();
        }
      }
    }

    delete [] lonely_nodes;
    glEndList();
  }
  else {
    maxslist = 0;
    slist1 = (GLfloat *) VMalloc(sizeof(GLfloat) * 1);
    slist2 = (GLfloat *) VMalloc(sizeof(GLfloat) * 1);
    slist3 = (GLfloat *) VMalloc(sizeof(GLfloat) * 1);
    slist4 = (GLfloat *) VMalloc(sizeof(GLfloat) * 1);
    for (int i = 0; i <= maxslist; i++) {
      slist1[i] = -1234.123450;
      slist2[i] = -1234.123450;
      slist3[i] = -1234.123450;
      slist4[i] = -1234.123450;
    }
  }

  //timer1 = (double)clock();
  //printf ("finished loading at a total time of %f seconds\n", (double)timer1/CLOCKS_PER_SEC);
  //printf ("it took a time of %f seconds\n", (double)(timer1-timer)/CLOCKS_PER_SEC);

  return list;
}

/* Generate an OpenGL display list for the object to be shown, i.e. the box */
GLuint MyGLDrawer::makeObject2(int s)
{
  float size;
  GLuint list2;

  //if (middle[0] == 0 && middle[1] == 0 && middle[2] == 0) {  // covering the middle-point
    /*double winx, winy, winz;
    GLint viewport[4];
    GLdouble mvmatrix[16], projmatrix[16];
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glGetIntegerv (GL_VIEWPORT, viewport);
    glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
    glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
    gluProject(pr->cursorp[0], pr->cursorp[1], pr->cursorp[2], mvmatrix, projmatrix, viewport, &winx, &winy, &winz);*/
    //middle[0] = pr->cursorp[0];
    //middle[1] = pr->cursorp[1];
    //middle[2] = pr->cursorp[2];
    //glMatrixMode (GL_MODELVIEW);
    //printf("middle: (%f, %f, %f) -> (%f, %f, %f)\n", pr->cursorp[0], pr->cursorp[1], pr->cursorp[2], middle[0], middle[1], middle[2]);
  //}
  list2 = glGenLists( 1 );
  glNewList( list2, GL_COMPILE );
  if (s == 0) { size=0.15;		// little cross
                glLineWidth( 1.0 );
              }
  else { size=0.5;			// big cross
         glLineWidth( 2.5 );
       }
  //  glPointSize( 3.0 );
  //    glEnable( GL_POINT_SMOOTH );
  GLfloat mat[3];

  //if (hgfarbe[0]==0) {
  glColor3f((GLfloat)pr->crosscolor.red()/255.0, (GLfloat)pr->crosscolor.green()/255.0, (GLfloat)pr->crosscolor.blue()/255.0);
  mat[0] = (GLfloat)pr->crosscolor.red()/255.0; mat[1] = (GLfloat)pr->crosscolor.green()/255.0; mat[2] = (GLfloat)pr->crosscolor.blue()/255.0;
  /*  glColor3f(1.0, 1.0, 0.0);
      mat[0] = 1.0; mat[1] = 1.0; mat[2] = 0.0;
      } else {
      glColor3f(0.5, 0.5, 0.0);
      mat[0] = 0.5; mat[1] = 0.5; mat[2] = 0.0;
      }*/
  if (pr->hgfarbe[0]==1) { mat[0]=0.0; mat[1]=0.0; mat[2]=0.0;  }

  glMaterialfv(GL_FRONT, GL_EMISSION, mat);
  glBegin( GL_LINES );
  glVertex3f( (pr->cursorp[0]+add_col-(float)columns/2.0)/50.0-size,(pr->cursorp[1]+add_row-(float)rows/2.0)/50.0,(pr->cursorp[2]+add_band-(float)bands/2.0)/50.0 );
  glVertex3f( (pr->cursorp[0]+add_col-(float)columns/2.0)/50.0-0.05,(pr->cursorp[1]+add_row-(float)rows/2.0)/50.0,(pr->cursorp[2]+add_band-(float)bands/2.0)/50.0 );
  glVertex3f( (pr->cursorp[0]+add_col-(float)columns/2.0)/50.0+0.05,(pr->cursorp[1]+add_row-(float)rows/2.0)/50.0,(pr->cursorp[2]+add_band-(float)bands/2.0)/50.0 );
  glVertex3f( (pr->cursorp[0]+add_col-(float)columns/2.0)/50.0+size,(pr->cursorp[1]+add_row-(float)rows/2.0)/50.0,(pr->cursorp[2]+add_band-(float)bands/2.0)/50.0 );
  glVertex3f( (pr->cursorp[0]+add_col-(float)columns/2.0)/50.0,(pr->cursorp[1]+add_row-(float)rows/2.0)/50.0-size,(pr->cursorp[2]+add_band-(float)bands/2.0)/50.0 );
  glVertex3f( (pr->cursorp[0]+add_col-(float)columns/2.0)/50.0,(pr->cursorp[1]+add_row-(float)rows/2.0)/50.0-0.05,(pr->cursorp[2]+add_band-(float)bands/2.0)/50.0 );
  glVertex3f( (pr->cursorp[0]+add_col-(float)columns/2.0)/50.0,(pr->cursorp[1]+add_row-(float)rows/2.0)/50.0+0.05,(pr->cursorp[2]+add_band-(float)bands/2.0)/50.0 );
  glVertex3f( (pr->cursorp[0]+add_col-(float)columns/2.0)/50.0,(pr->cursorp[1]+add_row-(float)rows/2.0)/50.0+size,(pr->cursorp[2]+add_band-(float)bands/2.0)/50.0 );
  glVertex3f( (pr->cursorp[0]+add_col-(float)columns/2.0)/50.0,(pr->cursorp[1]+add_row-(float)rows/2.0)/50.0,(pr->cursorp[2]+add_band-(float)bands/2.0)/50.0-size );
  glVertex3f( (pr->cursorp[0]+add_col-(float)columns/2.0)/50.0,(pr->cursorp[1]+add_row-(float)rows/2.0)/50.0,(pr->cursorp[2]+add_band-(float)bands/2.0)/50.0-0.05 );
  glVertex3f( (pr->cursorp[0]+add_col-(float)columns/2.0)/50.0,(pr->cursorp[1]+add_row-(float)rows/2.0)/50.0,(pr->cursorp[2]+add_band-(float)bands/2.0)/50.0+0.05 );
  glVertex3f( (pr->cursorp[0]+add_col-(float)columns/2.0)/50.0,(pr->cursorp[1]+add_row-(float)rows/2.0)/50.0,(pr->cursorp[2]+add_band-(float)bands/2.0)/50.0+size );
  glEnd();
  glEndList();
  //GLfloat mat[3];

  return list2;
}

GLuint MyGLDrawer::makeObject3() {
  double r = 0.0, g = 0.0, b = 0.0;

  GLuint list3;
  list3 = glGenLists( 1 );
  GLfloat mat[3];
  glNewList( list3, GL_COMPILE );
  glPointSize( 3.0 );
  glBegin( GL_POINTS );
  int schrittweite=3;
  if (pr->oglzmapdense) schrittweite=1;
  for ( int i=0; i<columns-2; i=i+schrittweite ) {
    for ( int j=0; j<rows-2; j=j+schrittweite ) {
      for ( int k=0; k<bands-2; k=k+schrittweite ) {
	if (i<fnc_columns && j<fnc_rows && k<fnc_bands) {
	  if ( VPixel(fnc[0],k,j,i,VFloat) > ppmax*(pr->pmax/100) ) {
	    if ((int)(20/(pr->pmax-ppmax)*(VPixel(fnc[0],k,j,i,VFloat)-ppmax)) > 19) {
	      if (col_tab_file)
	      {
	        get_color_from_extern_colortable((float)VPixel(fnc[0],k,j,i,VFloat), &r, &g, &b);
	        mat[0] = (GLfloat) r/255.0; mat[1] = (GLfloat) g/255.0; mat[2] = (GLfloat) b/255.0;
	      }
	      else
	      {
       	        get_color_from_intern_colortable((float)VPixel(fnc[0],k,j,i,VFloat), ppmax*(pr->pmax/100), pr->pmax, pr->coltype, &r, &g, &b, pr->hgfarbe[0]);
	        mat[0] = (GLfloat) r; mat[1] = (GLfloat) g; mat[2] = (GLfloat) b;
	      }
	      glMaterialfv(GL_FRONT, GL_EMISSION, mat);
	    } else {
	      if (col_tab_file)
	      {
	        get_color_from_extern_colortable((float)VPixel(fnc[0],k,j,i,VFloat), &r, &g, &b);
	        mat[0] = (GLfloat) r/255.0; mat[1] = (GLfloat) g/255.0; mat[2] = (GLfloat) b/255.0;
	      }
	      else
	      {
	        get_color_from_intern_colortable((float)VPixel(fnc[0],k,j,i,VFloat), ppmax*(pr->pmax/100), pr->pmax, pr->coltype, &r, &g, &b, pr->hgfarbe[0]);
	        mat[0] = (GLfloat) r; mat[1] = (GLfloat) g; mat[2] = (GLfloat) b;
	      }
	      glMaterialfv(GL_FRONT, GL_EMISSION, mat);
	    }
	    glVertex3f( ((float)i+add_col-(float)columns/2.0)/50.0,((float)j+add_row-(float)rows/2.0)/50.0,((float)k+add_band-(float)bands/2.0)/50.0 );
	  }
	  if ( VPixel(fnc[0],k,j,i,VFloat) < -nnmax*(pr->nmax/100)/*-nnmax*/ ) {
	    if ((int)(20/(pr->nmax-nnmax)*(-VPixel(fnc[0],k,j,i,VFloat)-nnmax)) > 19) {
	      if (col_tab_file)
	      {
	        get_color_from_extern_colortable(VPixel(fnc[0],k,j,i,VFloat), &r, &g, &b);
	        mat[0] = (GLfloat) r/255.0; mat[1] = (GLfloat) g/255.0; mat[2] = (GLfloat) b/255.0;
	      }
	      else
	      {
	        get_color_from_intern_colortable(VPixel(fnc[0],k,j,i,VFloat), nnmax*(pr->nmax/100), pr->nmax, pr->coltype, &r, &g, &b, pr->hgfarbe[0]);
	        mat[0] = (GLfloat) r; mat[1] = (GLfloat) g; mat[2] = (GLfloat) b;
	      }
	      glMaterialfv(GL_FRONT, GL_EMISSION, mat);
	    } else {
	      if (col_tab_file)
	      {
	        get_color_from_extern_colortable(VPixel(fnc[0],k,j,i,VFloat), &r, &g, &b);
	        mat[0] = (GLfloat) r/255.0; mat[1] = (GLfloat) g/255.0; mat[2] = (GLfloat) b/255.0;
	      }
	      else
	      {
	        get_color_from_intern_colortable(VPixel(fnc[0],k,j,i,VFloat), nnmax*(pr->nmax/100), pr->nmax, pr->coltype, &r, &g, &b, pr->hgfarbe[0]);
	        mat[0] = (GLfloat) r; mat[1] = (GLfloat) g; mat[2] = (GLfloat) b;
	      }
	      glMaterialfv(GL_FRONT, GL_EMISSION, mat);
	    }
	    glVertex3f( ((float)i+add_col-(float)columns/2.0)/50.0,((float)j+add_row-(float)rows/2.0)/50.0,((float)k+add_band-(float)bands/2.0)/50.0 );
	  }
	}
      }
    }
  }
  glPointSize( 1.5 );
  glEnd();  
  glEndList();
  
  return list3;
}


/*!
  Set the rotation angle of the object to \e degrees around the X axis.
*/

void MyGLDrawer::setXRotation( int degrees ) {
  xRot = (GLfloat)(degrees % 360);
  tb_axis[0] = (GLfloat)degrees/1000;  /* activated by A. Hagert: for mouse - rotation */
  talCross( pr->cursorp[0], pr->cursorp[1], pr->cursorp[2]);
  updateGL();
}


/*!
  Set the rotation angle of the object to \e degrees around the Y axis.
*/

void MyGLDrawer::setYRotation( int degrees ) {
  yRot = (GLfloat)(degrees % 360);
  tb_axis[1] = (GLfloat)degrees/1000;  /* activated by A. Hagert: for mouse - rotation */
  talCross( pr->cursorp[0], pr->cursorp[1], pr->cursorp[2]);
  updateGL();
}


/*!
  Set the rotation angle of the object to \e degrees around the Z axis.
*/

void MyGLDrawer::setZRotation( int degrees ) {
  zRot = (GLfloat)(degrees % 360);
  tb_axis[2] = (GLfloat)degrees/1000;  /* activated by A. Hagert: for mouse - rotation */
  talCross( pr->cursorp[0], pr->cursorp[1], pr->cursorp[2]);
  updateGL();
}

/* handling of the mouseMove event */
void MyGLDrawer::mouseMoveEvent( QMouseEvent *e ) {  /* activated by A. Hagert: for mouse - rotation */
  GLfloat dx, dy, dz;
  float /*int*/ x, y;  /* changed by A. Hagert: for float-graphs */
  int move_diff_x = 0, move_diff_y = 0;

  y=e->pos().y();
  x=e->pos().x();

  //trying to rotate the graph with the mouse
  _tbPointToVector(x, y, width(), height(), current_position);

  // calculate the angle to rotate by (directly proportional to the
  // length of the mouse movement
  dx = current_position[0] - tb_lastposition[0];
  dy = current_position[1] - tb_lastposition[1];
  dz = current_position[2] - tb_lastposition[2];

  tb_angle = 90.0 * sqrt(dx * dx + dy * dy + dz * dz);

  // calculate the axis of rotation (cross product)
  tb_axis[0] = tb_lastposition[1] * current_position[2] - tb_lastposition[2] * current_position[1];
  if ( tb_lastposition[1] * current_position[2] - tb_lastposition[2] * current_position[1] > 1.0) {
    tb_axis[0] = tb_axis[0] - 2;
  } else if ( tb_lastposition[1] * current_position[2] - tb_lastposition[2] * current_position[1] < -1.0) {
    tb_axis[0] = tb_axis[0] + 2;
  }
  tb_axis[1] = tb_lastposition[2] * current_position[0] - tb_lastposition[0] * current_position[2];
  if ( tb_lastposition[2] * current_position[0] - tb_lastposition[0] * current_position[2] > 1.0) {
    tb_axis[1] = tb_axis[1] - 2;
  } else if ( tb_lastposition[2] * current_position[0] - tb_lastposition[0] * current_position[2] < -1.0) {
    tb_axis[1] = tb_axis[1] + 2;
  }
  tb_axis[2] = tb_lastposition[0] * current_position[1] - tb_lastposition[1] * current_position[0];
  if ( tb_lastposition[0] * current_position[1] - tb_lastposition[1] * current_position[0] > 1.0) {
    tb_axis[2] = tb_axis[2] - 2;
  } else if ( tb_lastposition[0] * current_position[1] - tb_lastposition[1] * current_position[0] < -1.0) {
    tb_axis[2] = tb_axis[2] + 2;
  }

  if (button == 0) {  /* drehen nur mit linker Maustaste */
    xRot = (int)(xRot_temp + (GLfloat)tb_axis[0]*tb_angle) % 360;
    yRot = (int)(yRot_temp + (GLfloat)tb_axis[1]*tb_angle) % 360;
    zRot = (int)(zRot_temp + (GLfloat)tb_axis[2]*tb_angle) % 360;
  }
  if (button == 1) {  /* verschieben mit rechter Maustaste */
    move_diff_x = -(int)rint(pos_start_x - x);
    move_diff_y = (int)rint(pos_start_y - y);
    move((float)(move_diff_x-pos_last_x)/100.0, (float)(move_diff_y-pos_last_y)/100.0, 0.0);
    move_total_x += move_diff_x-pos_last_x;
    move_total_y += move_diff_y-pos_last_y;
    pos_last_x = move_diff_x;
    pos_last_y = move_diff_y;
  }
  if (button == 2) {  /* zoom mit mittlerer Maustaste */
    move_diff_x = -(int)rint(pos_start_x - x);
    move_diff_y = (int)rint(pos_start_y - y);
    if (move_diff_y-pos_last_y != 0.0)
      zoom(+0.1*(move_diff_y-pos_last_y));
    pos_last_x = move_diff_x;
    pos_last_y = move_diff_y;
  }
    //while (xRot > 180) xRot = xRot - 360;
    //while (xRot < -180) xRot = xRot + 360;
    //while (yRot > 180) yRot = yRot - 360;
    //while (yRot < -180) yRot = yRot + 360;
    //while (zRot > 180) zRot = zRot - 360;
    //while (zRot < -180) zRot = zRot + 360;

    talCross( pr->cursorp[0], pr->cursorp[1], pr->cursorp[2]);
    updateGL();

}

/* event to handle the mousepress */
void MyGLDrawer::mousePressEvent( QMouseEvent *e ) {
  int y=e->pos().y();
  int x=e->pos().x();
  sX=x;
  sY=y;

  if (e->button() == LeftButton) {   /* rotation with mouse */
    button=0;
    /* added by A. Hagert: for mouse - rotation: saving last position */
    _tbPointToVector(x, y, width(), height(), tb_lastposition);
    xRot_temp = xRot;
    yRot_temp = yRot;
    zRot_temp = zRot;
  }
  if (e->button() == RightButton) {   /* translation with mouse */
    button = 1;
    pos_start_x = x;
    pos_start_y = y;
  }
  if (e->button() == MidButton) {
    button=2;
    pos_start_x = x;
    pos_start_y = y;
  }
}

/* only move the cross on mouse release */
void MyGLDrawer::mouseReleaseEvent( QMouseEvent *e ) {
  int y=e->pos().y();
  int x=e->pos().x();

  if (button == 0) {
    if (x<sX+2 && x>sX-2 && y>sY-2 && y<sY+2)
      bewegeKreuz(x, y);
    emit zeichneOGL();
    /* added by A. Hagert: for mouse - rotation: signal to the widget -> vgview.C */
    emit mouseRotated ((int)xRot, (int)yRot, (int)zRot);
    //talCross( pr->cursorp[0], pr->cursorp[1], pr->cursorp[2]); /* for synchronized IPC */
    //printf("6****\n");
  }

  if (button == 1) {  /* verschieben mit rechter Maustaste */
    pos_last_x = 0;
    pos_last_y = 0;

    if (pos_start_x == x && pos_start_y == y) {  /* zuruecksetzen auf Mitte */
      move((float)-move_total_x/100.0, (float)-move_total_y/100.0, 0.0);
      move_total_x = 0;
      move_total_y = 0;
      updateGL();
    }
  }

  if (button == 2) {
    if (pos_start_x == x && pos_start_y == y) {  /* zuruecksetzen auf Normalzoom */
      zoom (0.0);
    }
    pos_last_x = 0;
    pos_last_y = 0;
    updateGL();
  }

  emit sendtoserver();
}

/* event to move the cross */
void MyGLDrawer::bewegeKreuz( int x, int y )
{
  GLdouble wx, wy, wz;     // by A. Hagert
  GLint viewport[4], realy;
  GLdouble mvmatrix[16], projmatrix[16];
  int best = 0;                                  /* added by A. Hagert:  */
  float abstand = 0.0, best_abstand = 10000.0;   /* added by A. Hagert:  */
  float start = 0.0, ende = 0.0, step = 0.0;     /* added by A. Hagert:  */

  //  qDebug("x=%d,y=%d",x,y);
  glGetIntegerv (GL_VIEWPORT, viewport);
  glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
  glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);

  realy = viewport[3] - (GLint) y - 1;
  int found=0;
  if ((fog_onoff == 1) && (pr->exact == 0)) { start = 0.0; ende = 0.80; step = 0.001; }	/* added by A. Hagert: Nebel an:  Sichtbarkeitsschwelle 0.55-0.75 */
  else  if (fog_onoff == 1) { start = 0.0; ende = 1.0; step = 0.001; }				/* added by A. Hagert: Nebel an:  Sichtbarkeitsschwelle 1.00-0.00 */
        else { start = 0.0; ende = 1.0; step = 0.01; }						/* added by A. Hagert: Nebel aus: Sichtbarkeitsschwelle 1.00-0.00 */
  for (GLdouble i=start;i<ende;i=i+step) {
    //printf("project: (%f, %f, %f)\n", (GLdouble) x, (GLdouble) realy, i);
    gluUnProject ((GLdouble) x, (GLdouble) realy, i, mvmatrix, projmatrix, viewport, &wx, &wy, &wz);
    if (pr->exact == 0)  /* added by A. Hagert: "old" clicking routine" */
    {
      for (int j=1;j<=(int)maxslist;j++) {
        if ((int)slist1[j]+add_col>=(int)(50*wx+((float)columns/2.0)-2) && (int)slist1[j]+add_col<=(int)(50*wx+((float)columns/2.0+2))) {
          if ((int)slist2[j]+add_row>=(int)(50*wy+((float)rows/2.0)-2) && (int)slist2[j]+add_row<=(int)(50*wy+((float)rows/2.0+2))) {
	    if ((int)slist3[j]+add_band>=(int)(50*wz+((float)bands/2.0)-2) && (int)slist3[j]+add_band<=(int)(50*wz+((float)bands/2.0+2))) {
	      found=j;
	      break;
  	    }
	  }
        }
      }
      if (found) break;
    }
    else  /* added by A. Hagert: "new" clicking routine - searches in ALL nodes */
    {
      for (int j=1;j<=(int)maxslist;j++) 
      {
        //printf ("(%f, %f, %f) - (%f, %f, %f)\n", wx, wy, wz, slist1[j], slist2[j], slist3[j]);
        abstand = sqrt ( pow(slist1[j]+add_col-(50*wx+((float)columns/2.0)), 2) +  pow(slist2[j]+add_row-(50*wy+((float)rows/2.0)), 2) + pow(slist3[j]+add_band-(50*wz+((float)bands/2.0)), 2 ) );
	//abstand = sqrt ( pow(slist1[j]-(50*wx+((float)columns/2.0)), 2) +  pow(slist2[j]-(50*wy+((float)rows/2.0)), 2) );
        if ( best_abstand >= abstand )
        {
          best_abstand = abstand;
	  best = j;
        }
      }
      found = best;
    }
  }
  //printf("%f -> %i (%i, %i)\n", best_abstand, best, x, y);

  if (found) 
  {
    pr->cursorp[0]=/*(int)rint*/(slist1[found]);
    pr->cursorp[1]=/*(int)rint*/(slist2[found]);
    pr->cursorp[2]=/*(int)rint*/(slist3[found]);
    //printf("* %i -> (%f, %f, %f)\n", ifile_m, pr->cursorp[0], pr->cursorp[1], pr->cursorp[2]);
    if (fnc[0])
    {
      if (pr->cursorp[2]<fnc_bands && pr->cursorp[1]<fnc_rows && pr->cursorp[0]<fnc_columns)
	emit z2Wert(VPixel(fnc[0],(int)rint(pr->cursorp[2]),(int)rint(pr->cursorp[1]),(int)rint(pr->cursorp[0]),VFloat));  // rint by A.Hagert
    }
    else if (pr->cursorp[2]<bands && pr->cursorp[1]<rows && pr->cursorp[0]<columns)
    {
      emit z2Wert(VPixel(src[0],(int)rint(pr->cursorp[2]),(int)rint(pr->cursorp[1]),(int)rint(pr->cursorp[0]),VUByte));    // rint by A.Hagert
    }
    talCross( pr->cursorp[0], pr->cursorp[1], pr->cursorp[2]);
    crossChange();
    emit kreuzBewegt();
  }
}

void MyGLDrawer::posChanged( int pos )
{
  ppmax=(double)pos/10;
}

void MyGLDrawer::negChanged( int neg )
{
  nnmax=(double)neg/10;
}

void MyGLDrawer::posChanged( )
{
  object3 = makeObject3();		// Generate an OpenGL display list
  updateGL();
}

void MyGLDrawer::negChanged( )
{
  object3 = makeObject3();		// Generate an OpenGL display list
  updateGL();
}

void MyGLDrawer::optionsOnOff(QWidget *parent) {

  //  for (int i=0;i<files;i++) {
    if (pr->picture[4]) {
      //object3 = makeObject3();		// Generate an OpenGL display list
      updateGL();
    }else {
      //object3 = makeObject3();		// Generate an OpenGL display list
      updateGL();
    }
    //}
}

void MyGLDrawer::talCross( float /*int*/ col, float /*int*/ row, float /*int*/ band ) {
  float x=col, y=row, z=band;
  int files=0;

  MyNode_short node_s, node1_s;  /* added by A. Hagert */
  MyNode_float node_f, node1_f;  /* added by A. Hagert */
  VAdjacency neighb;             /* added by A. Hagert */
  VNodeBaseRec base;             /* added by A. Hagert */
  float wertXYZ;                 /* added by A. Hagert */
  double r, g, b;		 /* added by A. Hagert */

  if (pr->infilenum>pr->zmapfilenum)
    files=pr->infilenum;
  else 
    files=pr->zmapfilenum;
  if (pr->talairach==1 && pr->talairachoff==0) {
    
    double XXt = (double)x;
    double YYt = (double)y;
    double ZZt = (double)z;
    VLTools mytools;

    mytools.VPixel3Tal(XXt,YYt,ZZt, extent, ca, cp, (int)files, pr->pixelmult);

    emit crossPosit((float)XXt,(float)YYt,(float)ZZt,"t");
  } else
    if (pr->pixelco==1)  {
      emit crossPosit((float)x,(float)y,(float)z,"a");
    }
    else if (pr->pixelco==2)
      /* changed by A. Hagert: int->float for float graphs */
      emit crossPosit((float)(x*(pr->pixelmult[0])/(pr->pixelm2[0])),(float)(y*(pr->pixelmult[1])/(pr->pixelm2[1])),(float)(z*(pr->pixelmult[2])/(pr->pixelm2[2])),"z");
    else
      /* changed by A. Hagert: int->float for float graphs */
      emit crossPosit((float)(x*(pr->pixelmult[0])),(float)(y*(pr->pixelmult[1])),(float)(z*(pr->pixelmult[2])),"m");


  /* added by A. Hagert ... */
  wertXYZ = 0.0;
  maxslist=gsrc->lastUsed;
  if (graphtype == 0)			/* graph of type short */
    for (int i=1; i<=gsrc->lastUsed; i++) 
      {
	node_s = (MyNode_short) VGraphGetNode (gsrc,i);
	if (node_s == NULL) continue;
	
	if ((node_s->band == rint(z)) && (node_s->row == rint(y)) && (node_s->col == rint(x)))
	  {
	    wertXYZ = node_s->label;
	    if (col_tab_file)
	      {
		/* added by A. Hagert: lastcolor -> rgb for background of the label-LCD-display */
		get_color_from_extern_colortable((float)node_s->label, &r, &g, &b);
		lastcolor[0] = (int) r; lastcolor[1] = (int) g; lastcolor[2] = (int) b;
	      }
	    else
	      {
		get_color_from_intern_colortable((float)node_s->label, mincolor[ifile_m], maxcolor[ifile_m], pr->gcoltype, &r, &g, &b, pr->hgfarbe[0]);
		/* added by A. Hagert: lastcolor -> rgb for background of the label-LCD-display */
		lastcolor[0] = (int) (255*r); lastcolor[1] = (int) (255*g); lastcolor[2] = (int) (255*b);
	      }
	  }
	
	base   = node_s->base;
	neighb = base.head;
	
	while ((neighb != NULL)) {
	  node1_s = (MyNode_short) VGraphGetNode(gsrc,neighb->id);
	  if ((node1_s->band == rint(z)) && (node1_s->row == rint(y)) && (node1_s->col == rint(x))) wertXYZ = node1_s->label;
	  neighb = neighb->next;
	}
      }
  else					/* graph of type float */
    for (int i=1; i<=gsrc->lastUsed; i++) 
      {
	node_f = (MyNode_float) VGraphGetNode (gsrc,i);
	if (node_f == NULL) continue;
	
	if ((node_f->band == z) && (node_f->row == y) && (node_f->col == x)) 
	  {
	    wertXYZ = node_f->label;
	    if (col_tab_file)
	      {
		/* added by A. Hagert: lastcolor -> rgb for background of the label-LCD-display */
		get_color_from_extern_colortable((float)node_f->label, &r, &g, &b);
		lastcolor[0] = (int) r; lastcolor[1] = (int) g; lastcolor[2] = (int) b;
	      }
	    else
	      {
		get_color_from_intern_colortable(node_f->label, mincolor[ifile_m], maxcolor[ifile_m], pr->gcoltype, &r, &g, &b, pr->hgfarbe[0]);
		/* added by A. Hagert: lastcolor -> rgb for background of the label-LCD-display */
		lastcolor[0] = (int) (255*r); lastcolor[1] = (int) (255*g); lastcolor[2] = (int) (255*b);
	      }
	  }
	
	base   = node_f->base;
	neighb = base.head;
	
	while ((neighb != NULL)) {
	  node1_f = (MyNode_float) VGraphGetNode(gsrc,neighb->id);
	  if ((node1_f->band == z) && (node1_f->row == y) && (node1_f->col == x)) wertXYZ = node1_f->label;
	  neighb = neighb->next;
	}
      }
  emit crossLabel((double)wertXYZ);
  lastx=x;lasty=y;
  /* end addition */ 
}

/* added by A. Hagert */
void MyGLDrawer::zoom (float z)
{
  /*GLint viewport[4];
  GLdouble modelMatrix[16], projMatrix[16];
  double winx, winy, winz, winx_m, winy_m, winz_m, objx, objy, objz;
  int rx=0, ry=0, rz=0;*/
  zoomfactor -= z*0.05;
  if (zoomfactor < 0.01) zoomfactor = 0.01;
  if (zoomfactor > 8.0) zoomfactor = 8.0;
  if (z == 0) zoomfactor = 1.1;
  glViewport(0, 0, (GLsizei)width(), (GLsizei)height());
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  //glFrustum( -zoomfactor, zoomfactor, -zoomfactor*(GLfloat)height()/(GLfloat)width(), zoomfactor*(GLfloat)height()/(GLfloat)width(), 5.0, 15.0 );				/* by A. Hagert */
  gluPerspective(21.0*zoomfactor, 1.0, 5.0, 15.0);
  if (move_total_x != 0 && move_total_y != 0)
    move((float)move_total_x/100.0, (float)move_total_y/100.0, 0.0);
  glMatrixMode (GL_MODELVIEW);
  /*if (z != 0) {
    glLoadIdentity ();
    gluLookAt(30.0, 30.0, 30.0, 10.0, 10.0, 10.0, 0.0, 1.0, 0.0);
    glClearColor(0.2, 0.2, 0.2, 0.0);
  }*/
  /*glGetIntegerv (GL_VIEWPORT, viewport);
  glGetDoublev (GL_MODELVIEW_MATRIX, modelMatrix);
  glGetDoublev (GL_PROJECTION_MATRIX, projMatrix);
  objx = pr->cursorp[0]; objy = pr->cursorp[1]; objz = pr->cursorp[2];
  gluProject(objx, objy, objz, modelMatrix, projMatrix, viewport, &winx, &winy, &winz);
  gluProject(middle[0], middle[1], middle[2], modelMatrix, projMatrix, viewport, &winx_m, &winy_m, &winz_m);
  if (winx > winx_m) rx = 1;
  else rx = -1;
  printf("winx_m = %f, winx = %f, rx = %i\n", winx_m, winx, rx);
  for (int i = 0; i < 5; i++) {
    if (winx > winx_m) glTranslatef (-0.1, 0.0, 0.0);
    else glTranslatef (0.1, 0.0, 0.0);
    glGetIntegerv (GL_VIEWPORT, viewport);
    glGetDoublev (GL_MODELVIEW_MATRIX, modelMatrix);
    glGetDoublev (GL_PROJECTION_MATRIX, projMatrix);
    gluProject(objx, objy, objz, modelMatrix, projMatrix, viewport, &winx, &winy, &winz);
    printf("winx_m = %f, winx = %f, rx = %i\n", winx_m, winx, rx);
    if (winx > winx_m) { if (rx == -1) rx = 2; }
    else { if (rx == 1) rx = 2; }
  } //while (rx != 2);
  printf("*** winx_m = %f, winx = %f, rx = %i\n", winx_m, winx, rx);
  glMatrixMode (GL_MODELVIEW);*/

  //if (z != 0) {
  //  double xfact = /*(2.0*asin(1))*//*1.0;
  //  double yfact = /*-(0.5*asin(1))*//*0.0;
  //  double zfact = /*(2.0*asin(1))*//*0.0;
  //  printf("middle: %f, %f, %f\n", middle[0], middle[1], middle[2]);
  //  printf("cursor: %f, %f, %f\n", pr->cursorp[0], pr->cursorp[1], pr->cursorp[2]);
  //  printf("moved to: %f, %f, %f\n", (pr->cursorp[0]-middle[0])/50, (pr->cursorp[1]-middle[1])/50, (pr->cursorp[2]-middle[2])/50);
  //  if ( (xRot == 0.0 && yRot == 0.0 && zRot == 0.0) || (fabs(xRot) == 180.0 && fabs(yRot) == 180.0 && fabs(zRot) == 180.0) )
  //         move((middle[0]-pr->cursorp[0])/50, (middle[1]-pr->cursorp[1])/50, 0.0);
  //  else if (xRot == 0.0 && fabs(yRot) == 180.0 && fabs(zRot) == 180.0)
  //         move((middle[0]-pr->cursorp[0])/50, -(middle[1]-pr->cursorp[1])/50, 0.0);
  //  else if (fabs(xRot) == 180.0 && fabs(yRot) == 180.0 && zRot == 0.0)
  //         move(-(middle[0]-pr->cursorp[0])/50, -(middle[1]-pr->cursorp[1])/50, 0.0);
  //  else if (fabs(xRot) == 180.0 && yRot == 0.0 && fabs(zRot) == 180.0)
  //         move(-(middle[0]-pr->cursorp[0])/50, (middle[1]-pr->cursorp[1])/50, 0.0);
  //  else if (fabs(xRot) == 0.0 && yRot == 0.0 && fabs(zRot) == 180.0)
  //         move(-(middle[0]-pr->cursorp[0])/50, -(middle[1]-pr->cursorp[1])/50, 0.0);
  //  else if (fabs(xRot) == 0.0 && yRot == 180.0 && fabs(zRot) == 0.0)
  //         move(-(middle[0]-pr->cursorp[0])/50, (middle[1]-pr->cursorp[1])/50, 0.0);
  //  else if (fabs(xRot) == 180.0 && yRot == 0.0 && fabs(zRot) == 0.0)
  //         move((middle[0]-pr->cursorp[0])/50, -(middle[1]-pr->cursorp[1])/50, 0.0);
  //  else move(xfact*(middle[0]-pr->cursorp[0])/50, yfact*(middle[1]-pr->cursorp[1])/50, zfact*(middle[2]-pr->cursorp[2]));
  //  printf("%f, %f, %f\n", xRot, yRot, zRot);
  //}
}

void MyGLDrawer::Disable_fog ()
{
  glDisable(GL_FOG);
}

void MyGLDrawer::Enable_fog ()
{
  glEnable(GL_FOG);
}

/* added by A. Hagert -> sets the viewpoint */
void MyGLDrawer::move( float x, float y, float z) {
  glMatrixMode (GL_PROJECTION);
  glTranslatef (x, y, z);
  glMatrixMode (GL_MODELVIEW);
}

/* added by A. Hagert (anaolog wie in BilderCW.C): findet locale Extrema */
void MyGLDrawer::findMinMaxZ(int vorz)
{
  float maxwert=0.0, abstand=0.0, best_abstand=10000.0;
  int found = 0;

  /* aktuellen Wert (label, slist4) bestimmen */
  for (int i = 1; i <= number_of_nodes; i++) {
    //printf("%i, (%f, %f, %f) -> %f : (%f, %f, %f)\n", i, pr->cursorp[0], pr->cursorp[1], pr->cursorp[2], slist1[i], slist2[i], slist3[i]);
    if (pr->cursorp[0]==slist1[i] && pr->cursorp[1]==slist2[i] && pr->cursorp[2]==slist3[i]) {
      //printf("found\n");
      found   = i;
      maxwert = slist4[i];
      break;
    }
  }

  if (!found) 
    if (vorz == 1) maxwert = 0.0;
    else maxwert = 1000.0;

  /* im "radius" nach groesstem/kleinstem suchen */
  for (int j = 1; j <= number_of_nodes; j++) {
    if ((int)slist1[j]>=(int)pr->cursorp[0]-pr->radius && (int)slist1[j]<=(int)pr->cursorp[0]+pr->radius) {
      if ((int)slist2[j]>=(int)pr->cursorp[1]-pr->radius && (int)slist2[j]<=(int)pr->cursorp[1]+pr->radius) {
	if ((int)slist3[j]>=(int)pr->cursorp[2]-pr->radius && (int)slist3[j]<=(int)pr->cursorp[2]+pr->radius) {
	  if (vorz*slist4[j] > vorz*maxwert) { 
	    best_abstand = sqrt ( pow(slist1[j]-pr->cursorp[0], 2) +  pow(slist2[j]-pr->cursorp[1], 2) + pow(slist3[j]-pr->cursorp[2], 2 ) );
	    found=j;
	    maxwert = slist4[j];
	  }
	  else if (vorz*slist4[j] == vorz*maxwert) { 
	    abstand = sqrt ( pow(slist1[j]-pr->cursorp[0], 2) +  pow(slist2[j]-pr->cursorp[1], 2) + pow(slist3[j]-pr->cursorp[2], 2 ) );
	    if (abstand < best_abstand) {
	      best_abstand = abstand;
	      found=j;
	      maxwert = slist4[j];
	    }
	  }
        }
      }
    }
  }
  
  if (found != 0) {
    pr->cursorp[0]=/*(int)rint*/(slist1[found]);
    pr->cursorp[1]=/*(int)rint*/(slist2[found]);
    pr->cursorp[2]=/*(int)rint*/(slist3[found]);
      
    if (fnc[0]) 
    {
      if (pr->cursorp[2]<fnc_bands && pr->cursorp[1]<fnc_rows && pr->cursorp[0]<fnc_columns)
        emit z2Wert(VPixel(fnc[0],(int)rint(pr->cursorp[2]),(int)rint(pr->cursorp[1]),(int)rint(pr->cursorp[0]),VFloat));  // rint by A.Hagert
    } 
    else if (pr->cursorp[2]<bands && pr->cursorp[1]<rows && pr->cursorp[0]<columns)
    {
      emit z2Wert(VPixel(src[0],(int)rint(pr->cursorp[2]),(int)rint(pr->cursorp[1]),(int)rint(pr->cursorp[0]),VUByte));    // rint by A.Hagert
    }
      
    talCross( pr->cursorp[0], pr->cursorp[1], pr->cursorp[2]);
    crossChange();
    emit kreuzBewegt();
  }
}

/* added by A. Hagert (anaolog wie in BilderCW.C): findet locales Minimum */
void MyGLDrawer::findMinZ()
{
  findMinMaxZ(-1);
}

/* added by A. Hagert (anaolog wie in BilderCW.C): findet locales Maximum */
void MyGLDrawer::findMaxZ() 
{
  findMinMaxZ(1);
}

void MyGLDrawer::coordIN() {
  bool ok = FALSE;
  char *token=NULL;
  int x=(int)rint(pr->cursorp[0]),y=(int)rint(pr->cursorp[1]),z=(int)rint(pr->cursorp[2]); // rint added by A. Hagert (for float-graphs)

  int arr[10];
  QString s;
  int files=0;

  if (pr->infilenum>pr->zmapfilenum)
    files=pr->infilenum;
  else 
    files=pr->zmapfilenum;

  if (pr->talairach==1 && pr->atlas==1) {
 
    s = QInputDialog::getText( tr( "Talairach coordinates [ X Y Z ]" ), tr( "Please enter Talairach coordinates (separated with spaces)" ), 
			       QLineEdit::Normal, QString::null, &ok, this );
  } else {
    if (pr->pixelco==1)
	    s = QInputDialog::getText( tr( "Anatomical voxel coordinates [ X Y Z ]" ), 
				       tr( "Please enter anatomical voxel coordinates (separated with spaces)" ),
				       QLineEdit::Normal, QString::null, &ok, this );
    else if (pr->pixelco==2 && pr->sw2)
      s = QInputDialog::getText( tr( "Zmap voxel coordinates [ X Y Z ]" ), tr( "Please enter zmap voxel coordinates (separated with spaces)" ), 
				 QLineEdit::Normal,QString::null, &ok, this );
    else
      s = QInputDialog::getText( tr( "Pixel coordinates in mm [ X Y Z ]" ), tr( "Please enter the coordinates in mm (separated with spaces)" ), 
				 QLineEdit::Normal, QString::null, &ok, this );
  }
  if ( ok && !s.isEmpty() ) {
    // user entered something and pressed ok
    char *ons = (char*)strdup(s);
    int i=0;
    token = strtok(ons, " ");
    if (token != NULL) {
      arr[i++] = atoi(token);
      
      while ((token = strtok(NULL, " ")) != NULL) { 
	arr[i++]=atoi(token);
      }
    }
    if (i==3) {
      x=arr[0];
      y=arr[1];
      z=arr[2];
    } else {
      QMessageBox::warning( this, "Warning",
      "Please specify 3 coordinates\nseparated with spaces");
      return; 
    }
  } else return;
  if (pr->talairach==1 && pr->atlas==1) {
    VLTools mytools;
    mytools.VTal3Pixel(x,y,z, pr->voxel, extent, ca, (int)files, pr->pixelmult );
  } else {
    if (pr->pixelco==1) {
      //die koordinaten bleiben wie sie sind
    }
    else if (pr->pixelco==2) {
      x=(int)rint((double)x/pr->pixelmult[0]*pr->pixelm2[0]);
      y=(int)rint((double)y/pr->pixelmult[1]*pr->pixelm2[1]);
      z=(int)rint((double)z/pr->pixelmult[2]*pr->pixelm2[2]);
    }
    else {
      x=(int)rint((double)x/pr->pixelmult[0]);
      y=(int)rint((double)y/pr->pixelmult[1]);
      z=(int)rint((double)z/pr->pixelmult[2]);
    }
  }
  if (z<0 || z>bands || y<0 || y>rows || x<0 || x>columns) {
    QMessageBox::warning( this, "Warning",
    "Illegal coordinates specified");
    return;  
  }

  pr->cursorp[0]=x;pr->cursorp[1]=y;pr->cursorp[2]=z;
  /*for (int j=0;j<files;j++) {
    bild1[j]->repaintf();
    bild2[j]->repaintf();
    bild3[j]->repaintf();
  }*/
  //talCross(pr->cursorp[0],pr->cursorp[1],pr->cursorp[2]);
  
  if (fnc[0]) {
    if (pr->cursorp[2]<fnc_bands && pr->cursorp[1]<fnc_rows && pr->cursorp[0]<fnc_columns)
      emit z2Wert(VPixel(fnc[pr->active],(int)rint(pr->cursorp[2]),(int)rint(pr->cursorp[1]),(int)rint(pr->cursorp[0]),VFloat));  // rint added by A. Hagert (for float-graphs)
  } else if (pr->cursorp[2]<bands && pr->cursorp[1]<rows && pr->cursorp[0]<columns) {
    emit z2Wert(VPixel(src[pr->active],(int)rint(pr->cursorp[2]),(int)rint(pr->cursorp[1]),(int)rint(pr->cursorp[0]),VUByte));    // rint added by A. Hagert (for float-graphs)
  }
  
  talCross( pr->cursorp[0], pr->cursorp[1], pr->cursorp[2]);
  crossChange();
  emit kreuzBewegt();
  emit sendtoserver();
}

void MyGLDrawer::move_cross() {
  pr->fog += 10;
  //emit z2Wert(VPixel(src[0],(int)rint(pr->cursorp[0]),(int)rint(pr->cursorp[1]),(int)rint(pr->cursorp[2]),VUByte));    // rint by A.Hagert
  talCross( pr->cursorp[0], pr->cursorp[1], pr->cursorp[2]);
  crossChange();
  emit kreuzBewegt();
}
