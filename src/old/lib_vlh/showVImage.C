/****************************************************************
**
** Implementation showVImage
** (c) 2002 by Heiko Mentzel
**
****************************************************************/

#include <qfont.h> 
#include <qimage.h>
#include <qpainter.h>
#include <qstring.h>

//#include <qinputdialog.h> 

#include "showVImage.h"
//#include "colortab.xpm"
//#include "onset.xpm"
//#include "coordEin.h"

#define NP -32768
#define MFT 65535


void VLShow::vlhColorMap( QColor *&rgbfarbe, QColor *&rgbfarbeoverlay, int acoltype, int coltype, VImage src, VImage *fnc, prefs *pr) {

  int mycol, p;
  for (int i=0; i<MFT; i++ ) {
    if (acoltype==0) {
      mycol=(int)rint((double)((float)(i+NP - pr->anamean)*pr->anaalpha));
      if (mycol>255) mycol=255;
      if (mycol<0) mycol=0;
      rgbfarbe[i].setRgb( mycol, mycol, mycol );
    } else {
      if (acoltype==1) {
      rgbfarbe[i].setHsv( int((360.0/(pr->maxwert - pr->minwert)) * i), 255,  255);
      
      //scrambled colors
      } else {
	p = ((i + NP - pr->minwert + pr->shift) % (pr->maxwert - pr->minwert))  - NP + pr->minwert;
	rgbfarbe[i].setHsv( int((360.0/(double)(pr->maxwert - pr->minwert + 1)*((double)pr->spread/10.0)) * (double)p), 255,  255); 
      }
      // scrambled colors
      
    }
  }


  //if (fnc[0])
   if (coltype==0)
      for (int i=0; i<128; i++ ) {
	if (i<64) {
	  rgbfarbeoverlay[255-(127-i)].setRgb(255,i*4,i*2);
	  rgbfarbeoverlay[255-(i+128)].setRgb(i*2,i*4,255);
	}
	else {
	  rgbfarbeoverlay[255-(127-i)].setRgb(255,255,i*2);	  
	  rgbfarbeoverlay[255-(i+128)].setRgb(i*2,255,255);
	}
      }
    else if (coltype==1)
      for (int i=0; i<128; i++ ) {
	if (i<95) {
	  rgbfarbeoverlay[255-(i)].setRgb(255, (int)rint(i*0.664921*4), (int)rint(i*0.664921*2));
	  rgbfarbeoverlay[255-(127-i+128)].setRgb((int)rint(i*0.664921*2), (int)rint(i*0.664921*4), 255);
	}
	else {
	  rgbfarbeoverlay[255-(i)].setRgb(255, 255, int(i*1.953846*2-245.184601));
	  rgbfarbeoverlay[255-(127-i+128)].setRgb(int(i*1.953846*2-245.184601), 255, 255);
	}
      }
    else if (coltype==2)
      for (int i=0; i<256; i++ ) {
	if (i<128) {
	  rgbfarbeoverlay[255-(127-i)].setHsv( int((360/60) * (i/6.4)), 255,  255);
	} else {
	  rgbfarbeoverlay[255-(383-i)].setHsv( int((360/60) * (i/6.4)), 255,  255);
	}
      }
    else if (coltype==3)
      for (int i=0; i<256; i++ ) {
	if (i<128) {
	  rgbfarbeoverlay[255-i].setHsv( int((360/60) * (i/6.4)), 255,  255);
	} else {
	  rgbfarbeoverlay[255-((i-128)+128)].setHsv( int((360/60) * (i/6.4)), 255,  255);
	}
      }
    else if (coltype==4)
      for (int i=0; i</*256*/128; i++ ) {
        if (i<32) {
	  rgbfarbeoverlay[128+i]=qRgb(i*4,i*8,255);
	  rgbfarbeoverlay[255-(128+i)]=qRgb(i*4,i*8,255);
	}
	else
	  if (i<64) {
	    rgbfarbeoverlay[128+i]=qRgb(i*4,255,255);
	    rgbfarbeoverlay[255-(128+i)]=qRgb(i*4,255,255);
	  }
	  else 
	    if (i<96) {
	      rgbfarbeoverlay[128+i]=qRgb(255,255,1023-4*i);
	      rgbfarbeoverlay[255-(128+i)]=qRgb(255,255,1023-4*i);
	    }
	    else {
	      rgbfarbeoverlay[128+i]=qRgb(255,2047-8*i,1023-4*i);
	      rgbfarbeoverlay[255-(128+i)]=qRgb(255,2047-8*i,1023-4*i);
	    }
      }
    else if (coltype==5)
      for (int i=0; i<128; i++ ) {
        rgbfarbeoverlay[255-i].setHsv( (int)(i*300/127), 255,  255);
	rgbfarbeoverlay[i].setHsv( (int)(i*300/127), 255,  255);
      }
    else if (coltype==6)
      for (int i=0; i<256; i++ ) {
	  rgbfarbeoverlay[255-i].setHsv(int((360/20) * (i/6.4)), 255,  255);
      }
    else if (coltype==7)
      for (int i=0; i<256; i++ ) {
	  rgbfarbeoverlay[255-i].setHsv( int((360/40) * (i/6.4)), 255,  255);
      }
    else if (coltype==8)
      for (int i=0; i<256; i++ ) {
        if (i<128)
          rgbfarbeoverlay[127-i].setHsv( 40 + int(320 * i / 127), 255,  255);
	else
	  rgbfarbeoverlay[i].setHsv( 40 + int(320 * (i-128) / 127), 255,  255);
      }


	else if (coltype==9)
	  for (int i=0; i<256; i++ )
	    rgbfarbeoverlay[255-i].setRgb( 255-i, 255-i, 255-i );


	  /* added by A. Anwander for diffusion tensor direction map
	     for (int i=0; i<256; i++ ) {
	     int r,g,b, code;
	     r=i/32;
	     g=(i-r*32)/4;
	     b=(i-r*32-g*4);
	     if (r>0) r=(int)((r+.5)*32);
	     if (g>0) g=(int)((g+.5)*32);
	     if (b>0) b=(int)((b+.5)*64);
	     code = i;
	     
	     // swap the zero point ( -128 ) to code zero
	     if (code == -128) code = 0;
	     else if (code == 0 ) code = -128;
	     
	     if (code<0) code=0;
	     rgbfarbeoverlay[code].setRgb( r, g, b );
	     } */


    else if (coltype==10)
      for (int i=0; i<256; i++ ) {
        rgbfarbeoverlay[255-i].setHsv( int((360/10) * (i/6.4)), 255,  255);
      }
    else if (coltype==11)
      for (int i=0; i<256; i++ ) {
	srand((unsigned int)i);
	int m1 = (int) (256.0*rand()/(RAND_MAX+1.0));
	srand((unsigned int)i+100);
	int m2 = (int) (256.0*rand()/(RAND_MAX+1.0));
	srand((unsigned int)i+200);
	int m3 = (int) (256.0*rand()/(RAND_MAX+1.0));
	rgbfarbeoverlay[i].setRgb(m1,m2,m3);

      }
    else if (coltype==12) {
//      // [TS] our attempt to make a proper color table ...
//      for(int cIndex = 0; cIndex < 127; cIndex++) {
//		// [TS] first we fill the lower half of the table with values from blue to green
//		rgbfarbeoverlay[cIndex].setRgb(0, 2 * cIndex, 255 - (2 * cIndex));
//		// [TS] the upper half of the table starts with yellow and goes to red
//		rgbfarbeoverlay[128 + cIndex].setRgb(255, 255 - (2 * cIndex), 0);
//      }
//      // [TS] the last table entry is not covered by our loop, so we set it manually to red
//      rgbfarbeoverlay[255].setRgb(255, 0, 0);
//      // [TS] the 'central' value is some yellowish thing
//      rgbfarbeoverlay[127].setRgb(255, 255, 135); 

    	rgbfarbeoverlay[0].setRgb(0, 0, 131);
    	rgbfarbeoverlay[1].setRgb(0, 0, 135);
    	rgbfarbeoverlay[2].setRgb(0, 0, 139);
    	rgbfarbeoverlay[3].setRgb(0, 0, 143);
    	rgbfarbeoverlay[4].setRgb(0, 0, 147);
    	rgbfarbeoverlay[5].setRgb(0, 0, 151);
    	rgbfarbeoverlay[6].setRgb(0, 0, 155);
    	rgbfarbeoverlay[7].setRgb(0, 0, 159);
    	rgbfarbeoverlay[8].setRgb(0, 0, 163);
    	rgbfarbeoverlay[9].setRgb(0, 0, 167);
    	rgbfarbeoverlay[10].setRgb(0, 0, 171);
    	rgbfarbeoverlay[11].setRgb(0, 0, 175);
    	rgbfarbeoverlay[12].setRgb(0, 0, 179);
    	rgbfarbeoverlay[13].setRgb(0, 0, 183);
    	rgbfarbeoverlay[14].setRgb(0, 0, 187);
    	rgbfarbeoverlay[15].setRgb(0, 0, 191);
    	rgbfarbeoverlay[16].setRgb(0, 0, 195);
    	rgbfarbeoverlay[17].setRgb(0, 0, 199);
    	rgbfarbeoverlay[18].setRgb(0, 0, 203);
    	rgbfarbeoverlay[19].setRgb(0, 0, 207);
    	rgbfarbeoverlay[20].setRgb(0, 0, 211);
    	rgbfarbeoverlay[21].setRgb(0, 0, 215);
    	rgbfarbeoverlay[22].setRgb(0, 0, 219);
    	rgbfarbeoverlay[23].setRgb(0, 0, 223);
    	rgbfarbeoverlay[24].setRgb(0, 0, 227);
    	rgbfarbeoverlay[25].setRgb(0, 0, 231);
    	rgbfarbeoverlay[26].setRgb(0, 0, 235);
    	rgbfarbeoverlay[27].setRgb(0, 0, 239);
    	rgbfarbeoverlay[28].setRgb(0, 0, 243);
    	rgbfarbeoverlay[29].setRgb(0, 0, 247);
    	rgbfarbeoverlay[30].setRgb(0, 0, 251);
    	rgbfarbeoverlay[31].setRgb(0, 0, 255);
    	rgbfarbeoverlay[32].setRgb(0, 4, 255);
    	rgbfarbeoverlay[33].setRgb(0, 8, 255);
    	rgbfarbeoverlay[34].setRgb(0, 12, 255);
    	rgbfarbeoverlay[35].setRgb(0, 16, 255);
    	rgbfarbeoverlay[36].setRgb(0, 20, 255);
    	rgbfarbeoverlay[37].setRgb(0, 24, 255);
    	rgbfarbeoverlay[38].setRgb(0, 28, 255);
    	rgbfarbeoverlay[39].setRgb(0, 32, 255);
    	rgbfarbeoverlay[40].setRgb(0, 36, 255);
    	rgbfarbeoverlay[41].setRgb(0, 40, 255);
    	rgbfarbeoverlay[42].setRgb(0, 44, 255);
    	rgbfarbeoverlay[43].setRgb(0, 48, 255);
    	rgbfarbeoverlay[44].setRgb(0, 52, 255);
    	rgbfarbeoverlay[45].setRgb(0, 56, 255);
    	rgbfarbeoverlay[46].setRgb(0, 60, 255);
    	rgbfarbeoverlay[47].setRgb(0, 64, 255);
    	rgbfarbeoverlay[48].setRgb(0, 68, 255);
    	rgbfarbeoverlay[49].setRgb(0, 72, 255);
    	rgbfarbeoverlay[50].setRgb(0, 76, 255);
    	rgbfarbeoverlay[51].setRgb(0, 80, 255);
    	rgbfarbeoverlay[52].setRgb(0, 84, 255);
    	rgbfarbeoverlay[53].setRgb(0, 88, 255);
    	rgbfarbeoverlay[54].setRgb(0, 92, 255);
    	rgbfarbeoverlay[55].setRgb(0, 96, 255);
    	rgbfarbeoverlay[56].setRgb(0, 100, 255);
    	rgbfarbeoverlay[57].setRgb(0, 104, 255);
    	rgbfarbeoverlay[58].setRgb(0, 108, 255);
    	rgbfarbeoverlay[59].setRgb(0, 112, 255);
    	rgbfarbeoverlay[60].setRgb(0, 116, 255);
    	rgbfarbeoverlay[61].setRgb(0, 120, 255);
    	rgbfarbeoverlay[62].setRgb(0, 124, 255);
    	rgbfarbeoverlay[63].setRgb(0, 128, 255);
    	rgbfarbeoverlay[64].setRgb(0, 131, 255);
    	rgbfarbeoverlay[65].setRgb(0, 135, 255);
    	rgbfarbeoverlay[66].setRgb(0, 139, 255);
    	rgbfarbeoverlay[67].setRgb(0, 143, 255);
    	rgbfarbeoverlay[68].setRgb(0, 147, 255);
    	rgbfarbeoverlay[69].setRgb(0, 151, 255);
    	rgbfarbeoverlay[70].setRgb(0, 155, 255);
    	rgbfarbeoverlay[71].setRgb(0, 159, 255);
    	rgbfarbeoverlay[72].setRgb(0, 163, 255);
    	rgbfarbeoverlay[73].setRgb(0, 167, 255);
    	rgbfarbeoverlay[74].setRgb(0, 171, 255);
    	rgbfarbeoverlay[75].setRgb(0, 175, 255);
    	rgbfarbeoverlay[76].setRgb(0, 179, 255);
    	rgbfarbeoverlay[77].setRgb(0, 183, 255);
    	rgbfarbeoverlay[78].setRgb(0, 187, 255);
    	rgbfarbeoverlay[79].setRgb(0, 191, 255);
    	rgbfarbeoverlay[80].setRgb(0, 195, 255);
    	rgbfarbeoverlay[81].setRgb(0, 199, 255);
    	rgbfarbeoverlay[82].setRgb(0, 203, 255);
    	rgbfarbeoverlay[83].setRgb(0, 207, 255);
    	rgbfarbeoverlay[84].setRgb(0, 211, 255);
    	rgbfarbeoverlay[85].setRgb(0, 215, 255);
    	rgbfarbeoverlay[86].setRgb(0, 219, 255);
    	rgbfarbeoverlay[87].setRgb(0, 223, 255);
    	rgbfarbeoverlay[88].setRgb(0, 227, 255);
    	rgbfarbeoverlay[89].setRgb(0, 231, 255);
    	rgbfarbeoverlay[90].setRgb(0, 235, 255);
    	rgbfarbeoverlay[91].setRgb(0, 239, 255);
    	rgbfarbeoverlay[92].setRgb(0, 243, 255);
    	rgbfarbeoverlay[93].setRgb(0, 247, 255);
    	rgbfarbeoverlay[94].setRgb(0, 251, 255);
    	rgbfarbeoverlay[95].setRgb(0, 255, 255);
    	rgbfarbeoverlay[96].setRgb(4, 255, 251);
    	rgbfarbeoverlay[97].setRgb(8, 255, 247);
    	rgbfarbeoverlay[98].setRgb(12, 255, 243);
    	rgbfarbeoverlay[99].setRgb(16, 255, 239);
    	rgbfarbeoverlay[100].setRgb(20, 255, 235);
    	rgbfarbeoverlay[101].setRgb(24, 255, 231);
    	rgbfarbeoverlay[102].setRgb(28, 255, 227);
    	rgbfarbeoverlay[103].setRgb(32, 255, 223);
    	rgbfarbeoverlay[104].setRgb(36, 255, 219);
    	rgbfarbeoverlay[105].setRgb(40, 255, 215);
    	rgbfarbeoverlay[106].setRgb(44, 255, 211);
    	rgbfarbeoverlay[107].setRgb(48, 255, 207);
    	rgbfarbeoverlay[108].setRgb(52, 255, 203);
    	rgbfarbeoverlay[109].setRgb(56, 255, 199);
    	rgbfarbeoverlay[110].setRgb(60, 255, 195);
    	rgbfarbeoverlay[111].setRgb(64, 255, 191);
    	rgbfarbeoverlay[112].setRgb(68, 255, 187);
    	rgbfarbeoverlay[113].setRgb(72, 255, 183);
    	rgbfarbeoverlay[114].setRgb(76, 255, 179);
    	rgbfarbeoverlay[115].setRgb(80, 255, 175);
    	rgbfarbeoverlay[116].setRgb(84, 255, 171);
    	rgbfarbeoverlay[117].setRgb(88, 255, 167);
    	rgbfarbeoverlay[118].setRgb(92, 255, 163);
    	rgbfarbeoverlay[119].setRgb(96, 255, 159);
    	rgbfarbeoverlay[120].setRgb(100, 255, 155);
    	rgbfarbeoverlay[121].setRgb(104, 255, 151);
    	rgbfarbeoverlay[122].setRgb(108, 255, 147);
    	rgbfarbeoverlay[123].setRgb(112, 255, 143);
    	rgbfarbeoverlay[124].setRgb(116, 255, 139);
    	rgbfarbeoverlay[125].setRgb(120, 255, 135);
    	rgbfarbeoverlay[126].setRgb(124, 255, 131);
    	rgbfarbeoverlay[127].setRgb(128, 255, 128);
    	rgbfarbeoverlay[128].setRgb(131, 255, 124);
    	rgbfarbeoverlay[129].setRgb(135, 255, 120);
    	rgbfarbeoverlay[130].setRgb(139, 255, 116);
    	rgbfarbeoverlay[131].setRgb(143, 255, 112);
    	rgbfarbeoverlay[132].setRgb(147, 255, 108);
    	rgbfarbeoverlay[133].setRgb(151, 255, 104);
    	rgbfarbeoverlay[134].setRgb(155, 255, 100);
    	rgbfarbeoverlay[135].setRgb(159, 255, 96);
    	rgbfarbeoverlay[136].setRgb(163, 255, 92);
    	rgbfarbeoverlay[137].setRgb(167, 255, 88);
    	rgbfarbeoverlay[138].setRgb(171, 255, 84);
    	rgbfarbeoverlay[139].setRgb(175, 255, 80);
    	rgbfarbeoverlay[140].setRgb(179, 255, 76);
    	rgbfarbeoverlay[141].setRgb(183, 255, 72);
    	rgbfarbeoverlay[142].setRgb(187, 255, 68);
    	rgbfarbeoverlay[143].setRgb(191, 255, 64);
    	rgbfarbeoverlay[144].setRgb(195, 255, 60);
    	rgbfarbeoverlay[145].setRgb(199, 255, 56);
    	rgbfarbeoverlay[146].setRgb(203, 255, 52);
    	rgbfarbeoverlay[147].setRgb(207, 255, 48);
    	rgbfarbeoverlay[148].setRgb(211, 255, 44);
    	rgbfarbeoverlay[149].setRgb(215, 255, 40);
    	rgbfarbeoverlay[150].setRgb(219, 255, 36);
    	rgbfarbeoverlay[151].setRgb(223, 255, 32);
    	rgbfarbeoverlay[152].setRgb(227, 255, 28);
    	rgbfarbeoverlay[153].setRgb(231, 255, 24);
    	rgbfarbeoverlay[154].setRgb(235, 255, 20);
    	rgbfarbeoverlay[155].setRgb(239, 255, 16);
    	rgbfarbeoverlay[156].setRgb(243, 255, 12);
    	rgbfarbeoverlay[157].setRgb(247, 255, 8);
    	rgbfarbeoverlay[158].setRgb(251, 255, 4);
    	rgbfarbeoverlay[159].setRgb(255, 255, 0);
    	rgbfarbeoverlay[160].setRgb(255, 251, 0);
    	rgbfarbeoverlay[161].setRgb(255, 247, 0);
    	rgbfarbeoverlay[162].setRgb(255, 243, 0);
    	rgbfarbeoverlay[163].setRgb(255, 239, 0);
    	rgbfarbeoverlay[164].setRgb(255, 235, 0);
    	rgbfarbeoverlay[165].setRgb(255, 231, 0);
    	rgbfarbeoverlay[166].setRgb(255, 227, 0);
    	rgbfarbeoverlay[167].setRgb(255, 223, 0);
    	rgbfarbeoverlay[168].setRgb(255, 219, 0);
    	rgbfarbeoverlay[169].setRgb(255, 215, 0);
    	rgbfarbeoverlay[170].setRgb(255, 211, 0);
    	rgbfarbeoverlay[171].setRgb(255, 207, 0);
    	rgbfarbeoverlay[172].setRgb(255, 203, 0);
    	rgbfarbeoverlay[173].setRgb(255, 199, 0);
    	rgbfarbeoverlay[174].setRgb(255, 195, 0);
    	rgbfarbeoverlay[175].setRgb(255, 191, 0);
    	rgbfarbeoverlay[176].setRgb(255, 187, 0);
    	rgbfarbeoverlay[177].setRgb(255, 183, 0);
    	rgbfarbeoverlay[178].setRgb(255, 179, 0);
    	rgbfarbeoverlay[179].setRgb(255, 175, 0);
    	rgbfarbeoverlay[180].setRgb(255, 171, 0);
    	rgbfarbeoverlay[181].setRgb(255, 167, 0);
    	rgbfarbeoverlay[182].setRgb(255, 163, 0);
    	rgbfarbeoverlay[183].setRgb(255, 159, 0);
    	rgbfarbeoverlay[184].setRgb(255, 155, 0);
    	rgbfarbeoverlay[185].setRgb(255, 151, 0);
    	rgbfarbeoverlay[186].setRgb(255, 147, 0);
    	rgbfarbeoverlay[187].setRgb(255, 143, 0);
    	rgbfarbeoverlay[188].setRgb(255, 139, 0);
    	rgbfarbeoverlay[189].setRgb(255, 135, 0);
    	rgbfarbeoverlay[190].setRgb(255, 131, 0);
    	rgbfarbeoverlay[191].setRgb(255, 128, 0);
    	rgbfarbeoverlay[192].setRgb(255, 124, 0);
    	rgbfarbeoverlay[193].setRgb(255, 120, 0);
    	rgbfarbeoverlay[194].setRgb(255, 116, 0);
    	rgbfarbeoverlay[195].setRgb(255, 112, 0);
    	rgbfarbeoverlay[196].setRgb(255, 108, 0);
    	rgbfarbeoverlay[197].setRgb(255, 104, 0);
    	rgbfarbeoverlay[198].setRgb(255, 100, 0);
    	rgbfarbeoverlay[199].setRgb(255, 96, 0);
    	rgbfarbeoverlay[200].setRgb(255, 92, 0);
    	rgbfarbeoverlay[201].setRgb(255, 88, 0);
    	rgbfarbeoverlay[202].setRgb(255, 84, 0);
    	rgbfarbeoverlay[203].setRgb(255, 80, 0);
    	rgbfarbeoverlay[204].setRgb(255, 76, 0);
    	rgbfarbeoverlay[205].setRgb(255, 72, 0);
    	rgbfarbeoverlay[206].setRgb(255, 68, 0);
    	rgbfarbeoverlay[207].setRgb(255, 64, 0);
    	rgbfarbeoverlay[208].setRgb(255, 60, 0);
    	rgbfarbeoverlay[209].setRgb(255, 56, 0);
    	rgbfarbeoverlay[210].setRgb(255, 52, 0);
    	rgbfarbeoverlay[211].setRgb(255, 48, 0);
    	rgbfarbeoverlay[212].setRgb(255, 44, 0);
    	rgbfarbeoverlay[213].setRgb(255, 40, 0);
    	rgbfarbeoverlay[214].setRgb(255, 36, 0);
    	rgbfarbeoverlay[215].setRgb(255, 32, 0);
    	rgbfarbeoverlay[216].setRgb(255, 28, 0);
    	rgbfarbeoverlay[217].setRgb(255, 24, 0);
    	rgbfarbeoverlay[218].setRgb(255, 20, 0);
    	rgbfarbeoverlay[219].setRgb(255, 16, 0);
    	rgbfarbeoverlay[220].setRgb(255, 12, 0);
    	rgbfarbeoverlay[221].setRgb(255, 8, 0);
    	rgbfarbeoverlay[222].setRgb(255, 4, 0);
    	rgbfarbeoverlay[223].setRgb(255, 0, 0);
    	rgbfarbeoverlay[224].setRgb(251, 0, 0);
    	rgbfarbeoverlay[225].setRgb(247, 0, 0);
    	rgbfarbeoverlay[226].setRgb(243, 0, 0);
    	rgbfarbeoverlay[227].setRgb(239, 0, 0);
    	rgbfarbeoverlay[228].setRgb(235, 0, 0);
    	rgbfarbeoverlay[229].setRgb(231, 0, 0);
    	rgbfarbeoverlay[230].setRgb(227, 0, 0);
    	rgbfarbeoverlay[231].setRgb(223, 0, 0);
    	rgbfarbeoverlay[232].setRgb(219, 0, 0);
    	rgbfarbeoverlay[233].setRgb(215, 0, 0);
    	rgbfarbeoverlay[234].setRgb(211, 0, 0);
    	rgbfarbeoverlay[235].setRgb(207, 0, 0);
    	rgbfarbeoverlay[236].setRgb(203, 0, 0);
    	rgbfarbeoverlay[237].setRgb(199, 0, 0);
    	rgbfarbeoverlay[238].setRgb(195, 0, 0);
    	rgbfarbeoverlay[239].setRgb(191, 0, 0);
    	rgbfarbeoverlay[240].setRgb(187, 0, 0);
    	rgbfarbeoverlay[241].setRgb(183, 0, 0);
    	rgbfarbeoverlay[242].setRgb(179, 0, 0);
    	rgbfarbeoverlay[243].setRgb(175, 0, 0);
    	rgbfarbeoverlay[244].setRgb(171, 0, 0);
    	rgbfarbeoverlay[245].setRgb(167, 0, 0);
    	rgbfarbeoverlay[246].setRgb(163, 0, 0);
    	rgbfarbeoverlay[247].setRgb(159, 0, 0);
    	rgbfarbeoverlay[248].setRgb(155, 0, 0);
    	rgbfarbeoverlay[249].setRgb(151, 0, 0);
    	rgbfarbeoverlay[250].setRgb(147, 0, 0);
    	rgbfarbeoverlay[251].setRgb(143, 0, 0);
    	rgbfarbeoverlay[252].setRgb(139, 0, 0);
    	rgbfarbeoverlay[253].setRgb(135, 0, 0);
    	rgbfarbeoverlay[254].setRgb(131, 0, 0);
    	rgbfarbeoverlay[255].setRgb(128, 0, 0);
    	
    }
    else
      for (int i=0; i<256; i++ ) {
        rgbfarbeoverlay[255-i].setHsv( 255, 255, 255);
      } 

}

void VLShow::vlhCreateLegend( QPixmap &cpm, QColor *rgbfarbeoverlay, double ppmax, 
							  double pmax, double nnmax, double nmax, bool equidistantColorTable,
							  QColor bg, QColor textcolor) {
    cpm = QPixmap();
    QImage colorimage;
    colorimage.create ( 160,31, 32, 1024 );
    colorimage.fill(bg.rgb());
    //      colorimage.fill(parent->backgroundColor());
    if(!equidistantColorTable) {
	    for (int i=0;i<60;i++) {
	      colorimage.setPixel ( 48+i, 5, rgbfarbeoverlay[(int)(i*2.12)+128].rgb() );
	      colorimage.setPixel ( 48+i, 6, rgbfarbeoverlay[(int)(i*2.12)+128].rgb() );
	      colorimage.setPixel ( 48+i, 7, rgbfarbeoverlay[(int)(i*2.12)+128].rgb() );
	      colorimage.setPixel ( 48+i, 8, rgbfarbeoverlay[(int)(i*2.12)+128].rgb() );
	      colorimage.setPixel ( 48+i, 9, rgbfarbeoverlay[(int)(i*2.12)+128].rgb() );
	      colorimage.setPixel ( 48+i,10, rgbfarbeoverlay[(int)(i*2.12)+128].rgb() );
	      colorimage.setPixel ( 48+i,11, rgbfarbeoverlay[(int)(i*2.12)+128].rgb() );
	      
	      colorimage.setPixel ( 48+i, 5+13, rgbfarbeoverlay[127-(int)(i*2.12)].rgb() );
	      colorimage.setPixel ( 48+i, 6+13, rgbfarbeoverlay[127-(int)(i*2.12)].rgb() );
	      colorimage.setPixel ( 48+i, 7+13, rgbfarbeoverlay[127-(int)(i*2.12)].rgb() );
	      colorimage.setPixel ( 48+i, 8+13, rgbfarbeoverlay[127-(int)(i*2.12)].rgb() );
	      colorimage.setPixel ( 48+i, 9+13, rgbfarbeoverlay[127-(int)(i*2.12)].rgb() );
	      colorimage.setPixel ( 48+i,10+13, rgbfarbeoverlay[127-(int)(i*2.12)].rgb() );
	      colorimage.setPixel ( 48+i,11+13, rgbfarbeoverlay[127-(int)(i*2.12)].rgb() );
	    }
	    //    cpm.convertFromImage(colorimage,~ColorMode_Mask | QPainter::AutoColor );
	    cpm.convertFromImage(colorimage,QPainter::AutoColor );
	    QPainter pcpm( &cpm );
	    int genauigkeit=4;
	    if (pmax<1) genauigkeit=4;
	    pcpm.setFont( QFont( "arial", 9, QFont::Bold, FALSE ) );
	    pcpm.setPen(textcolor);
	    pcpm.drawText ( 7, 13, QWidget::tr("%1").arg(ppmax), genauigkeit );
	    pcpm.drawText ( 118 /* 97 */, 13, QWidget::tr("%1").arg(pmax), genauigkeit );
	    pcpm.drawText ( 7, 13+13, QWidget::tr("%1").arg(-nnmax), genauigkeit+1 );
	    pcpm.drawText ( 118 /* 92 */, 13+13, QWidget::tr("%1").arg(-nmax), genauigkeit+1 );
    } else {
	    for (int i=0;i<60;i++) {
		      colorimage.setPixel( 48+i, 12, rgbfarbeoverlay[(int)(i*4.26)].rgb() );
		      colorimage.setPixel( 48+i, 13, rgbfarbeoverlay[(int)(i*4.26)].rgb() );
		      colorimage.setPixel( 48+i, 14, rgbfarbeoverlay[(int)(i*4.26)].rgb() );
		      colorimage.setPixel( 48+i, 15, rgbfarbeoverlay[(int)(i*4.26)].rgb() );
		      colorimage.setPixel( 48+i, 16, rgbfarbeoverlay[(int)(i*4.26)].rgb() );
		      colorimage.setPixel( 48+i, 17, rgbfarbeoverlay[(int)(i*4.26)].rgb() );
		      colorimage.setPixel( 48+i, 18, rgbfarbeoverlay[(int)(i*4.26)].rgb() );
	    }		      
	    cpm.convertFromImage(colorimage,QPainter::AutoColor );
	    QPainter pcpm( &cpm );
	    int genauigkeit=5;
	    if (pmax<1) genauigkeit=5;
	    pcpm.setFont( QFont( "arial", 9, QFont::Bold, FALSE ) );
	    pcpm.setPen(textcolor);
	    pcpm.drawText ( 7, 20, QWidget::tr("%1").arg(-nmax), genauigkeit );
	    pcpm.drawText ( 118 /* 97 */, 20, QWidget::tr("%1").arg(pmax), genauigkeit );
    }
}

QImage VLShow::vlhCreateImage ( QWidget *parent, VImage *src, VImage *fnc, int type, int columns, int rows, int bands, int fnc_columns, int fnc_rows, int fnc_bands, VArgVector in_files, double *fixpoint_m, QColor *rgbfarbe, int transzmap, int bildzoom, int XX, int YY, int ZZ, int files_m, int *hgfarbe, int zmapview, int ifile_m, double sc1, double sc2, double ppmax, double pmax, double nnmax, double nmax, int verbose ) {
  QImage image;
  double farbe1;
  int verplus, horplus, XXplus, YYplus, ZZplus;

  int int1=(int)rint(-fixpoint_m[1*files_m+0]+fixpoint_m[1*files_m+ifile_m]);
  int int2=(int)rint(-fixpoint_m[2*files_m+0]+fixpoint_m[2*files_m+ifile_m]);
 
  int int6=(int)rint(-fixpoint_m[0*files_m+0]+fixpoint_m[0*files_m+ifile_m]);
  int int7=(int)rint(-fixpoint_m[0*files_m+0]+fixpoint_m[3*files_m+ifile_m]);

  int int12=(int)rint(-fixpoint_m[1*files_m+0]+fixpoint_m[4*files_m+ifile_m]);
  int int13=(int)rint(-fixpoint_m[2*files_m+0]+fixpoint_m[5*files_m+ifile_m]);

  double n20=20/(nmax-nnmax);
  double p20=20/(pmax-ppmax);

  double vgroesse=parent->width(), hgroesse=parent->height();
  if (type==3) {
    image.create ( columns, rows, 32 );
    if (ZZ>=bands) ZZ=bands/2;
    if (ZZ<bands) {
      //      if (verbose) fprintf(stderr,"sf: %d %d %d\n",sf[0],sf[1],sf[2]);
      for ( int i=0; i<rows; i++ ) {
	for ( int j=0; j<columns; j++ ) {
	  if (in_files.number>1) {
	    if (i+int1>=0 && i+int1<=rows && j+int2>=0 && j+int2<=columns && ZZ+int6>=0 && ZZ+int6<=bands) {
	      if (VPixelRepn(src[ifile_m])==VShortRepn)
		farbe1=VPixel(src[ifile_m],ZZ+int6,i+int1,j+int2,VShort)*210/256;
	      else
		farbe1=VPixel(src[ifile_m],ZZ+int6,i+int1,j+int2,VUByte)*210/256;
	    } else {
	      farbe1=0;
	    }
	  } else {
	    if (VPixelRepn(src[0])==VShortRepn)
	      farbe1=VPixel(src[0],ZZ,i,j,VShort)*210/256;
	    else
	      farbe1=VPixel(src[0],ZZ,i,j,VUByte)*210/256;
	  }
	  if (farbe1>NP) 
	    image.setPixel ( j, i, rgbfarbe[(int)farbe1-NP].rgb() );
	  else if (hgfarbe[0]==0) {
	    image.setPixel ( j, i, rgbfarbe[0].rgb() );
	  } else {
	    //               C2 R1
	    image.setPixel ( j, i, rgbfarbe[MFT-1].rgb() );
	  }

	  // functional data
	  if (fnc[0]==NULL) {
	    if (ZZ+int7>=0 && ZZ+int7<=fnc_bands && i+int12>=0 && i+int12<=fnc_rows && j+int13>=0 && j+int13<=fnc_columns) {
	      int tzm;
	      if (transzmap) tzm=220-(int) ((double)farbe1/1.5); else tzm=0;
	      if ( VPixel(fnc[ifile_m],ZZ+int7,i+int12,j+int13,VFloat) > ppmax && zmapview==1 ) {
		if ((int)rint(p20*(VPixel(fnc[ifile_m],ZZ+int7,i+int12,j+int13,VFloat)-ppmax)) > 19) {
		  //image.setPixel ( j-(sf[2]/2), i-(sf[1]/2), 19+210 );
		  image.setPixel ( j, i, rgbfarbe[19+210].dark(tzm).rgb() );
		} else {
		  image.setPixel ( j, i, rgbfarbe[(int)rint(p20*(VPixel(fnc[ifile_m],ZZ+int7,i+int12,j+int13,VFloat)-ppmax))+210].dark(tzm).rgb() );
		}
	      }
	      if ( VPixel(fnc[ifile_m],ZZ+int7,i+int12,j+int13,VFloat) < -nnmax && zmapview==1 ) {
		if ((int)rint(n20*(-VPixel(fnc[ifile_m],ZZ+int7,i+int12,j+int13,VFloat)-nnmax)) > 19) {
		  image.setPixel ( j, i, rgbfarbe[19+230].dark(tzm).rgb() );
		} else {
		  image.setPixel ( j, i, rgbfarbe[(int)rint(n20*(-VPixel(fnc[ifile_m],ZZ+int7,i+int12,j+int13,VFloat)-nnmax))+230].dark(tzm).rgb() );
		}
	      }
	    }
	  }
	}
      } 
      sc1=vgroesse/columns;
      sc2=hgroesse/rows;
      if (bildzoom>=2) {
	verplus=-(int)rint((double)XX/(double)bildzoom*((double)bildzoom-1.0));
	horplus=-(int)rint((double)YY/(double)bildzoom*((double)bildzoom-1.0));
	XXplus=(int)rint((double)XX/(double)bildzoom*((double)bildzoom-1.0));
	YYplus=(int)rint((double)YY/(double)bildzoom*((double)bildzoom-1.0));
      } else {
	verplus=0;
	horplus=0;
	XXplus=0;
	YYplus=0;
      }
    }
  } else if (type==2) {
    image.create ( rows, bands, 32 );
    if (XX<columns) 
      for ( int i=0; i<bands; i++ ) {
	for ( int j=0; j<rows; j++ ) {
	  if (in_files.number>1) {
	    if (i+int6>=0 && i+int6<bands && j+int1>=0 && j+int1<rows && XX+int2>=0 && XX+int2<columns) {
	      if (VPixelRepn(src[ifile_m])==VShortRepn)
		farbe1=VPixel(src[ifile_m],i+int6,j+int1,XX+int2,VShort)*210/256;
	      else
		farbe1=VPixel(src[ifile_m],i+int6,j+int1,XX+int2,VUByte)*210/256;
	    } else
	      farbe1=0;
	  } else {
	    if (VPixelRepn(src[0])==VShortRepn)
	      farbe1=VPixel(src[0], i, j, XX, VShort)*210/256;
	    else
	      farbe1=VPixel(src[0], i, j, XX, VUByte)*210/256;
	  }
	  if (farbe1>NP) 
	    image.setPixel ( j, i, rgbfarbe[(int)farbe1-NP].rgb() );
	  else if (hgfarbe[0]==0) {
	    image.setPixel ( j, i, rgbfarbe[0].rgb() );
	  } else {
	    image.setPixel ( j, i, rgbfarbe[MFT-1].rgb() );
	  }
	  

	  // functional data
	  if (fnc[0]==NULL) {
	    if (i+int7>=0 && i+int7<fnc_bands && XX+int13>=0 && XX+int13<fnc_columns && j+int12>=0 && j+int12<fnc_rows) {
	      int tzm;
	      if (transzmap) tzm=220-(int) ((double)farbe1/1.5); else tzm=0;
	      if ( VPixel(fnc[ifile_m],i+int7,j+int12,XX+int13,VFloat) > ppmax && zmapview==1 ) {
		if ((int)rint(p20*(VPixel(fnc[ifile_m],i+int7,j+int12,XX+int13,VFloat)-ppmax)) > 19) 
		  image.setPixel ( j, i, rgbfarbe[19+210].dark(tzm).rgb() );
		else
		  image.setPixel ( j, i, rgbfarbe[(int)rint(p20*(VPixel(fnc[ifile_m],i+int7,j+int12,XX+int13,VFloat)-ppmax))+210].dark(tzm).rgb() );
	      }
	      if ( VPixel(fnc[ifile_m],i+int7,j+int12,XX+int13,VFloat) < -nnmax && zmapview==1 )
		if ((int)rint(n20*(-VPixel(fnc[ifile_m],i+int7,j+int12,XX+int13,VFloat)-nnmax)) > 19)
		  image.setPixel ( j, i, rgbfarbe[19+230].dark(tzm).rgb() );
		else
		  image.setPixel ( j, i, rgbfarbe[(int)rint(n20*(-VPixel(fnc[ifile_m],i+int7,j+int12,XX+int13,VFloat)-nnmax))+230].dark(tzm).rgb() );
	    }
	  }
	}
      }
    sc1=vgroesse/rows;
    sc2=hgroesse/bands;
    if (bildzoom>=2) {
      verplus=-(int)rint((double)YY/(double)bildzoom*((double)bildzoom-1.0));
      horplus=-(int)rint((double)ZZ/(double)bildzoom*((double)bildzoom-1.0));
      ZZplus=(int)rint((double)ZZ/(double)bildzoom*((double)bildzoom-1.0));
      YYplus=(int)rint((double)YY/(double)bildzoom*((double)bildzoom-1.0));
    } else {
      verplus=0;
      horplus=0;
      ZZplus=0;
      YYplus=0;
    }
    if (verbose) {
      fprintf(stderr,"v:%i f:%i l:%i\n",int7,(int)fixpoint_m[0*files_m+0],(int)fixpoint_m[3*files_m+ifile_m]);
      fprintf(stderr,"v:%i f:%i l:%i\n",int12,(int)fixpoint_m[1*files_m+0],(int)fixpoint_m[4*files_m+ifile_m]);
      fprintf(stderr,"v:%i f:%i l:%i\n",int13,(int)fixpoint_m[2*files_m+0],(int)fixpoint_m[5*files_m+ifile_m]);
    }
  } else if (type==1) {
    image.create ( columns, bands, 32 );
    if (YY<rows) 
      for ( int i=0; i<bands; i++ ) {
	for ( int j=0; j<columns; j++ ) {
	  if (in_files.number>1) {
	    if (i+int6>=0 && i+int6<bands && j+int2>=0 && j+int2<bands && YY+int1>=0 && YY+int1<rows) {

	      /* ANATOMIE ZEICHNEN VORBEREITEN */
	      if (VPixelRepn(src[ifile_m])==VShortRepn)
		farbe1=VPixel(src[ifile_m],i+int6,YY+int1,j+int2,VShort)*210/256;
	      else
		farbe1=VPixel(src[ifile_m],i+int6,YY+int1,j+int2,VUByte)*210/256;
	    } else {
	      farbe1=0;
	    }
	  } else {
	    if (VPixelRepn(src[0])==VShortRepn)
	      farbe1=VPixel(src[0],i,YY,j,VShort)*210/256;
	    else
	      farbe1=VPixel(src[0],i,YY,j,VUByte)*210/256;
	  }
	  if (farbe1>NP) 
	    image.setPixel ( j, i, rgbfarbe[(int)farbe1-NP].rgb() );
	  else if (hgfarbe[0]==0) {
	    image.setPixel ( j, i, rgbfarbe[0].rgb() );
	  } else {
	    image.setPixel ( j, i, rgbfarbe[MFT-1].rgb() );
	  }
	  

	  // functional data
	  if (fnc[0]==NULL) {
	    if ( i+int7>=0 && i+int7<fnc_bands && j+int13>=0 && j+int13<fnc_columns && YY+int12>=0 && YY+int12<fnc_rows) {
	      int tzm;
	      if (transzmap) tzm=220-(int) ((double)farbe1/1.5); else tzm=0;
	      if ( VPixel(fnc[ifile_m],i+int7,YY+int12,j+int13,VFloat) > ppmax && zmapview==1 ) {
		if ((int)rint(p20*(VPixel(fnc[ifile_m],i+int7,YY+int12,j+int13,VFloat)-ppmax)) > 19) {
		  image.setPixel ( j, i, rgbfarbe[19+210].dark(tzm).rgb() );
		} else {
		  image.setPixel ( j, i, rgbfarbe[(int)rint(p20*(VPixel(fnc[ifile_m],i+int7,YY+int12,j+int13,VFloat)-ppmax))+210].dark(tzm).rgb() );
		}
	      }
	      if ( VPixel(fnc[ifile_m],i+int7,YY+int12,j+int13,VFloat) < -nnmax && zmapview==1 ) {
		if ((int)rint(n20*(-VPixel(fnc[ifile_m],i+int7,YY+int12,j+int13,VFloat)-nnmax)) > 19) {
		  image.setPixel ( j, i, rgbfarbe[19+230].dark(tzm).rgb() );
		} else {
		  image.setPixel ( j, i, rgbfarbe[(int)rint(n20*(-VPixel(fnc[ifile_m],i+int7,YY+int12,j+int13,VFloat)-nnmax))+230].dark(tzm).rgb() );
		}
	      }
	    }
	  }
	}
      }
    sc1=vgroesse/columns;
    sc2=hgroesse/bands;
    if (bildzoom>=2) {
      verplus=-(int)rint((double)XX/(double)bildzoom*((double)bildzoom-1.0));
      horplus=-(int)rint((double)ZZ/(double)bildzoom*((double)bildzoom-1.0));
      ZZplus=(int)rint((double)ZZ/(double)bildzoom*((double)bildzoom-1.0));
      XXplus=(int)rint((double)XX/(double)bildzoom*((double)bildzoom-1.0));
    } else {
      verplus=0;
      horplus=0;
      XXplus=0;
      ZZplus=0;
    }
  }



  return image;
}
