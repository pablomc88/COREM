#include "fixationalMovGrating.h"

fixationalMovGrating::fixationalMovGrating(int X,int Y,double radius,double jitter,double period,double step,double luminance,double contrast,double orientation,double red_weight,double green_weigh, double blue_weight,int t1,int t2,int ts)
{  
    unsigned seed1,seed2,seed3,seed4;
    seed1 = std::chrono::system_clock::now().time_since_epoch().count();
    seed3 = std::chrono::system_clock::now().time_since_epoch().count();

    // Fixed seed to lower processing time when optimizing parameters:
    // it is not necessary to average several trials, only one trial
    // per individual to optimize. Random distributions wouldn't be used
//    seed1 = 10;
//    seed2 = 20;
//    seed3 = 30;
//    seed4 = 40;

    sizeX = X;
    sizeY = Y;
    type1 = t1;
    type2 = t2;
    tswitch = ts;
    circle_radius = radius;
    jitter_period = jitter;
    spatial_period = period;
    step_size = step;
    Lum = luminance;
    Cont = contrast;
    theta = orientation;
    r = red_weight;
    g = green_weigh;
    b = blue_weight;

    x0= (int)(X/2);
    y0 =(int)(Y/2);
    cos_theta=cos(theta);
    sin_theta=sin(theta);
    A=Cont*Lum;
    aux = *(new CImg <double>(Y,X,1,3));


    generator1 = *(new default_random_engine(seed1));
    distribution1 = *(new normal_distribution<double>(0.0,step_size));

    if(type1 == 0){
        seed2 = std::chrono::system_clock::now().time_since_epoch().count();
    }else{
        seed2 = seed1;
    }

    generator2 = *(new default_random_engine(seed2));
    distribution2 = *(new normal_distribution<double>(0.0,step_size));


    generator3 = *(new default_random_engine(seed3));

    if(type2 == 0){
        seed4 = std::chrono::system_clock::now().time_since_epoch().count();
    }else{
        seed4 = seed3;
    }

    generator4 = *(new default_random_engine(seed4));


    Pi = 3.14159265;
    jitter1 = 0.0;
    jitter2 = 0.0;
    j1 = -step_size;
    value1,value2,value3 = 0.0;

}

fixationalMovGrating::fixationalMovGrating(const fixationalMovGrating& copy){

}

fixationalMovGrating::~fixationalMovGrating(void){

}

//------------------------------------------------------------------------------//

CImg <double>* fixationalMovGrating::compute_grating(double t){

    if((int)t%(int)jitter_period == 0){

        if(t < tswitch){
            j1 = distribution1(generator1);
            j2 = distribution2(generator2);

            // shift fixed
//            if(type1 == 0){
//                j1 = -j1;
//                j2 = -j1;
//            }else{
//                j1 = -j1;
//                j2 = j1;
//            }

        }else{
            j1 = distribution1(generator3);
            j2 = distribution2(generator4);

            // shift fixed
//            if(type2 == 0){
//                j1 = -j1;
//                j2 = -j1;
//            }else{
//                j1 = -j1;
//                j2 = j1;
//            }
        }


        jitter1 = j1;
        jitter2 = j2;


       cimg_forXY(aux,x,y) {

           radius = sqrt((double(x) - double(x0))*(double(x) - double(x0)) + (double(y) - double(y0))*(double(y) - double(y0)));

           if(radius < circle_radius){
               value3 = b*Lum + b * A *cos(Pi/2 +  (((x-x0+jitter1)*cos_theta+(y-y0+jitter1)*sin_theta)/spatial_period)*2*Pi);
               value2 = g*Lum + g * A *cos(Pi/2 +   (((x-x0+jitter1)*cos_theta+(y-y0+jitter1)*sin_theta)/spatial_period)*2*Pi);
               value1 = r*Lum + r * A *cos(Pi/2 +   (((x-x0+jitter1)*cos_theta+(y-y0+jitter1)*sin_theta)/spatial_period)*2*Pi);
           }else{
               value3 = b*Lum + b * A *cos(Pi/2 +   (((x-x0+jitter2)*cos_theta+(y-y0+jitter2)*sin_theta)/spatial_period)*2*Pi);
               value2 = g*Lum + g * A *cos(Pi/2 +   (((x-x0+jitter2)*cos_theta+(y-y0+jitter2)*sin_theta)/spatial_period)*2*Pi);
               value1 = r*Lum + r * A *cos(Pi/2 +   (((x-x0+jitter2)*cos_theta+(y-y0+jitter2)*sin_theta)/spatial_period)*2*Pi);
           }

               aux(x,y,0,0)=value1;
               aux(x,y,0,1)=value2;
               aux(x,y,0,2)=value3;
       }

    }

   return &aux;
}

//------------------------------------------------------------------------------//
