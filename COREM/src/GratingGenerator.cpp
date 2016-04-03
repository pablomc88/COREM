#include "GratingGenerator.h"

GratingGenerator::GratingGenerator(){
    cout << "please do not use the default constructor." << endl;
}

GratingGenerator::GratingGenerator(int ptype, double pstep, double plengthB, double plength, double plength2, int pX, int pY, double pfreq, double pT, double pLum, double pCont, double pphi, double pphi_t, double ptheta, double red, double green, double blue, double pred_phi, double pgreen_phi, double pblue_phi){

    type = ptype;
    step = pstep;
    lengthB = plengthB;
    length = plength;
    length2 = plength2;
    X = pX;
    Y = pY;
    freq = pfreq;
    T = pT;
    Lum = pLum;
    Cont = pCont;
    phi = pphi;
    phi_t = pphi_t;
    theta = ptheta;
    r=red;
    g=green;
    b=blue;

    red_phi = pred_phi;
    green_phi = pgreen_phi;
    blue_phi = pblue_phi;

    Bsize=(int) (lengthB/step);
    first_grating_size= (int) (length/step);
    second_grating_size= (type==2)? (int) (length2/step):0;

    x0= (int)(X/2);
    y0 =(int)(Y/2);
    cos_theta=cos(theta);
    sin_theta=sin(theta);
    A=Cont*Lum;
    aux = *(new CImg <double>(Y,X,1,3));
}

GratingGenerator::GratingGenerator(const GratingGenerator& copy){

}

GratingGenerator::~GratingGenerator(void){

}

//------------------------------------------------------------------------------//

CImg <double>* GratingGenerator::compute_grating(double t){

    if(t>=0 && t<Bsize){
       cimg_forXY(aux,x,y) {
           aux(x,y,0,0) = r*Lum,
           aux(x,y,0,1) = g*Lum,
           aux(x,y,0,2) = b*Lum;
       }
    }

   double Pi = 3.14159265;


   if(t>=Bsize && t<Bsize+first_grating_size){
       if(type==0){
                   cimg_forXY(aux,x,y) {

                           double value3 = b*Lum + b * A *cos( (((x-x0)*cos_theta+(y-y0)*sin_theta)/T + phi/2 - freq*step*t)*2*Pi + blue_phi*Pi);
                           double value2 = g*Lum + g * A *cos( (((x-x0)*cos_theta+(y-y0)*sin_theta)/T + phi/2 - freq*step*t)*2*Pi + green_phi*Pi);
                           double value1 = r*Lum + r * A *cos( (((x-x0)*cos_theta+(y-y0)*sin_theta)/T + phi/2 - freq*step*t)*2*Pi + red_phi*Pi);
                           aux(x,y,0,0)=value1;
                           aux(x,y,0,1)=value2;
                           aux(x,y,0,2)=value3;
                   }

       }else
       {
         if(type==1){
             cimg_forXY(aux,x,y) {
                     double value3 = b*Lum + b * A *cos( (((x-x0)*cos_theta+(y-y0)*sin_theta)/T + phi/2 )*2*Pi + blue_phi*Pi) * cos((freq*step*t+phi_t/2)*2*Pi);
                     double value2 = g*Lum + g * A *cos( (((x-x0)*cos_theta+(y-y0)*sin_theta)/T + phi/2 )*2*Pi + green_phi*Pi) * cos((freq*step*t+phi_t/2)*2*Pi);
                     double value1 = r*Lum + r * A *cos( (((x-x0)*cos_theta+(y-y0)*sin_theta)/T + phi/2 )*2*Pi + red_phi*Pi) * cos((freq*step*t+phi_t/2)*2*Pi);
                     aux(x,y,0,0)=value1;
                     aux(x,y,0,1)=value2;
                     aux(x,y,0,2)=value3;

             }
         }
         else{
             cimg_forXY(aux,x,y) {
                     double value3 = b*Lum + b * A *cos( (((x-x0)*cos_theta+(y-y0)*sin_theta)/T + phi/2)*2*Pi + blue_phi*Pi);
                     double value2 = g*Lum + g * A *cos( (((x-x0)*cos_theta+(y-y0)*sin_theta)/T + phi/2)*2*Pi + green_phi*Pi);
                     double value1 = r*Lum + r * A *cos( (((x-x0)*cos_theta+(y-y0)*sin_theta)/T + phi/2)*2*Pi + red_phi*Pi);
                     aux(x,y,0,0)=value1;
                     aux(x,y,0,1)=value2;
                     aux(x,y,0,2)=value3;

                }
           }
       }
   }


   //adding the reversed grating in case that type==2
   if(t>=Bsize+first_grating_size && t<Bsize+first_grating_size+second_grating_size){
       cimg_forXY(aux,x,y) {
             double value3 = b*Lum - b * A *cos( (((x-x0)*cos_theta+(y-y0)*sin_theta)/T + phi/2)*2*Pi + blue_phi*Pi);
             double value2 = g*Lum - g * A *cos( (((x-x0)*cos_theta+(y-y0)*sin_theta)/T + phi/2)*2*Pi + green_phi*Pi);
             double value1 = r*Lum - r * A *cos( (((x-x0)*cos_theta+(y-y0)*sin_theta)/T + phi/2)*2*Pi + red_phi*Pi);
             aux(x,y,0,0)=value1;
             aux(x,y,0,1)=value2;
             aux(x,y,0,2)=value3;

       }
   }


   return &aux;
}

