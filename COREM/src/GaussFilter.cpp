#include "GaussFilter.h"

GaussFilter::GaussFilter(int x, int y, double ppd):module(x,y,1.0){
    sizeX = x;
    sizeY = y;
    pixelsPerDegree = ppd;
}

GaussFilter::GaussFilter(const GaussFilter &copy){
    (*this)=copy;
}

GaussFilter::~GaussFilter(){
    delete buffer;
}
//------------------------------------------------------------------------------//

void GaussFilter::allocateValues(){

    // transform sigma to pixels
    sigma*=pixelsPerDegree;

    inputImage=new CImg<double> (sizeY,sizeX,1,1,0.0);
    outputImage=new CImg<double> (sizeY,sizeX,1,1,0.0);

    // reserve space for all possible threads
    buffer = new double[(sizeX+sizeY)*omp_get_max_threads()];

    if (spaceVariantSigma==false){

        // coefficient calculation
        q = 0.98711 * sigma - 0.96330;

        if (sigma<2.5)
            q = 3.97156 - 4.14554 * sqrt (1.0 - 0.26891 * sigma);

        b0 = 1.57825 + 2.44413*q + 1.4281*q*q + 0.422205*q*q*q;
        b1 = 2.44413*q + 2.85619*q*q + 1.26661*q*q*q;
        b2 = -1.4281*q*q - 1.26661*q*q*q;
        b3 = 0.422205*q*q*q;
        B = 1.0 - (b1+b2+b3) / b0;

        b1 /= b0;
        b2 /= b0;
        b3 /= b0;

        // From: Bill Triggs, Michael Sdika: Boundary Conditions for Young-van Vliet Recursive Filtering
        M[0][0] = -b3*b1+1.0-b3*b3-b2;
        M[0][1] = (b3+b1)*(b2+b3*b1);
        M[0][2] = b3*(b1+b3*b2);
        M[1][0] = b1+b3*b2;
        M[1][1] = -(b2-1.0)*(b2+b3*b1);
        M[1][2] = -(b3*b1+b3*b3+b2-1.0)*b3;
        M[2][0] = b3*b1+b2+b1*b1-b2*b2;
        M[2][1] = b1*b2+b3*b2*b2-b1*b3*b3-b3*b3*b3-b3*b2+b3;
        M[2][2] = b3*(b1+b3*b2);
        for (int i=0; i<3; i++)
            for (int j=0; j<3; j++)
                M[i][j] /= (1.0+b1-b2+b3)*(1.0+b2+(b1-b3)*b3);

    }else{


        double new_sigma = 0.0;
        double r,rmax;

        // initialize matrices
        q_m = CImg <double>(sizeY,sizeX,1,1,0.0);
        b0_m = CImg <double>(sizeY,sizeX,1,1,0.0);
        b1_m = CImg <double>(sizeY,sizeX,1,1,0.0);
        b2_m = CImg <double>(sizeY,sizeX,1,1,0.0);
        b3_m = CImg <double>(sizeY,sizeX,1,1,0.0);
        B_m = CImg <double>(sizeY,sizeX,1,1,0.0);
        M_m = CImg <double>(sizeY,sizeX,9,1,0.0);

        for(int i=0;i<sizeY;i++){
            for(int j=0;j<sizeX;j++){

                // update sigma value
                r = sqrt((double(i)-floor(sizeY/2))*(double(i)-floor(sizeY/2)) + (double(j)-floor(sizeX/2))*(double(j)-floor(sizeX/2)));          
                new_sigma = sigma / density(r/pixelsPerDegree);

                // coefficient calculation
                q_m(i,j,0) = 0.98711 * new_sigma - 0.96330;

                if (new_sigma<2.5)
                    q_m(i,j,0) = 3.97156 - 4.14554 * sqrt (1.0 - 0.26891 * new_sigma);

                b0_m(i,j,0) = 1.57825 + 2.44413*q_m(i,j,0) + 1.4281*q_m(i,j,0)*q_m(i,j,0) + 0.422205*q_m(i,j,0)*q_m(i,j,0)*q_m(i,j,0);
                b1_m(i,j,0) = 2.44413*q_m(i,j,0) + 2.85619*q_m(i,j,0)*q_m(i,j,0) + 1.26661*q_m(i,j,0)*q_m(i,j,0)*q_m(i,j,0);
                b2_m(i,j,0) = -1.4281*q_m(i,j,0)*q_m(i,j,0) - 1.26661*q_m(i,j,0)*q_m(i,j,0)*q_m(i,j,0);
                b3_m (i,j,0)= 0.422205*q_m(i,j,0)*q_m(i,j,0)*q_m(i,j,0);
                B_m(i,j,0) = 1.0 - (b1_m(i,j,0)+b2_m(i,j,0)+b3_m(i,j,0)) / b0_m(i,j,0);

                b1_m(i,j,0) /= b0_m(i,j,0);
                b2_m(i,j,0) /= b0_m(i,j,0);
                b3_m(i,j,0) /= b0_m(i,j,0);

                // From: Bill Triggs, Michael Sdika: Boundary Conditions for Young-van Vliet Recursive Filtering
                M_m(i,j,0) = -b3_m(i,j,0)*b1_m(i,j,0)+1.0-b3_m(i,j,0)*b3_m(i,j,0)-b2_m(i,j,0);
                M_m(i,j,1) = (b3_m(i,j,0)+b1_m(i,j,0))*(b2_m(i,j,0)+b3_m(i,j,0)*b1_m(i,j,0));
                M_m(i,j,2) = b3_m(i,j,0)*(b1_m(i,j,0)+b3_m(i,j,0)*b2_m(i,j,0));
                M_m(i,j,3) = b1_m(i,j,0)+b3_m(i,j,0)*b2_m(i,j,0);
                M_m(i,j,4) = -(b2_m(i,j,0)-1.0)*(b2_m(i,j,0)+b3_m(i,j,0)*b1_m(i,j,0));
                M_m(i,j,5) = -(b3_m(i,j,0)*b1_m(i,j,0)+b3_m(i,j,0)*b3_m(i,j,0)+b2_m(i,j,0)-1.0)*b3_m(i,j,0);
                M_m(i,j,6) = b3_m(i,j,0)*b1_m(i,j,0)+b2_m(i,j,0)+b1_m(i,j,0)*b1_m(i,j,0)-b2_m(i,j,0)*b2_m(i,j,0);
                M_m(i,j,7) = b1_m(i,j,0)*b2_m(i,j,0)+b3_m(i,j,0)*b2_m(i,j,0)*b2_m(i,j,0)-b1_m(i,j,0)*b3_m(i,j,0)*b3_m(i,j,0)-b3_m(i,j,0)*b3_m(i,j,0)*b3_m(i,j,0)-b3_m(i,j,0)*b2_m(i,j,0)+b3_m(i,j,0);
                M_m(i,j,8) = b3_m(i,j,0)*(b1_m(i,j,0)+b3_m(i,j,0)*b2_m(i,j,0));
                for (int z=0; z<9; z++)
                    M_m(i,j,z) /= (1.0+b1_m(i,j,0)-b2_m(i,j,0)+b3_m(i,j,0))*(1.0+b2_m(i,j,0)+(b1_m(i,j,0)-b3_m(i,j,0))*b3_m(i,j,0));

            }
        }

    }


}


//------------------------------------------------------------------------------//

GaussFilter& GaussFilter::setSigma(double sigm){
    if (sigm>= 0){
        sigma = sigm;
    }
}

//------------------------------------------------------------------------------//


void GaussFilter::gaussVertical(CImg<double> &src){

#pragma omp parallel for

    for (int i=0; i<sizeY; i++) {

        double* temp2 = buffer + omp_get_thread_num()*(sizeX+sizeY);

        temp2[0] = B*(double)src(i,0,0) + b1*(double)src(i,0,0) + b2*(double)src(i,0,0) + b3*(double)src(i,0,0);
        temp2[1] = B*(double)src(i,1,0) + b1*temp2[0]  + b2*(double)src(i,0,0) + b3*(double)src(i,0,0);
        temp2[2] = B*(double)src(i,2,0) + b1*temp2[1]  + b2*temp2[0]  + b3*(double)src(i,0,0);

        for (int j=3; j<sizeX; j++)
            temp2[j] = B*(double)src(i,j,0) + b1*temp2[j-1] + b2*temp2[j-2] + b3*temp2[j-3];

        double temp2Wm1 = (double)src(i,sizeX-1,0) + M[0][0]*(temp2[sizeX-1] - (double)src(i,sizeX-1,0)) + M[0][1]*(temp2[sizeX-2] - (double)src(i,sizeX-1,0)) + M[0][2]*(temp2[sizeX-3] - (double)src(i,sizeX-1,0));
        double temp2W   = (double)src(i,sizeX-1,0) + M[1][0]*(temp2[sizeX-1] - (double)src(i,sizeX-1,0)) + M[1][1]*(temp2[sizeX-2] - (double)src(i,sizeX-1,0)) + M[1][2]*(temp2[sizeX-3] - (double)src(i,sizeX-1,0));
        double temp2Wp1 = (double)src(i,sizeX-1,0) + M[2][0]*(temp2[sizeX-1] - (double)src(i,sizeX-1,0)) + M[2][1]*(temp2[sizeX-2] - (double)src(i,sizeX-1,0)) + M[2][2]*(temp2[sizeX-3] - (double)src(i,sizeX-1,0));

        temp2[sizeX-1] = temp2Wm1;
        temp2[sizeX-2] = B * temp2[sizeX-2] + b1*temp2[sizeX-1] + b2*temp2W + b3*temp2Wp1;
        temp2[sizeX-3] = B * temp2[sizeX-3] + b1*temp2[sizeX-2] + b2*temp2[sizeX-1] + b3*temp2W;

            for (int j=sizeX-4; j>=0; j--)
                temp2[j] = B * temp2[j] + b1*temp2[j+1] + b2*temp2[j+2] + b3*temp2[j+3];
            for (int j=0; j<sizeX; j++)
                src(i,j,0) = (double)temp2[j];
        }

}

//------------------------------------------------------------------------------//

void GaussFilter::gaussHorizontal(CImg<double> &src){

#pragma omp parallel for

    for (int i=0; i<sizeX; i++) {

        double* temp2 = buffer + omp_get_thread_num()*(sizeX+sizeY);

        temp2[0] = B*(double)src(0,i,0) + b1*(double)src(0,i,0) + b2*(double)src(0,i,0) + b3*(double)src(0,i,0);
        temp2[1] = B*(double)src(1,i,0) + b1*temp2[0]  + b2*(double)src(0,i,0) + b3*(double)src(0,i,0);
        temp2[2] = B*(double)src(2,i,0) + b1*temp2[1]  + b2*temp2[0]  + b3*(double)src(0,i,0);

        for (int j=3; j<sizeY; j++)
            temp2[j] = B*(double)src(j,i,0) + b1*temp2[j-1] + b2*temp2[j-2] + b3*temp2[j-3];

        double temp2Wm1 = (double)src(sizeY-1,i,0) + M[0][0]*(temp2[sizeY-1] - (double)src(sizeY-1,i,0)) + M[0][1]*(temp2[sizeY-2] - (double)src(sizeY-1,i,0)) + M[0][2]*(temp2[sizeY-3] - (double)src(sizeY-1,i,0));
        double temp2W   = (double)src(sizeY-1,i,0) + M[1][0]*(temp2[sizeY-1] - (double)src(sizeY-1,i,0)) + M[1][1]*(temp2[sizeY-2] - (double)src(sizeY-1,i,0)) + M[1][2]*(temp2[sizeY-3] - (double)src(sizeY-1,i,0));
        double temp2Wp1 = (double)src(sizeY-1,i,0) + M[2][0]*(temp2[sizeY-1] - (double)src(sizeY-1,i,0)) + M[2][1]*(temp2[sizeY-2] - (double)src(sizeY-1,i,0)) + M[2][2]*(temp2[sizeY-3] - (double)src(sizeY-1,i,0));

        temp2[sizeY-1] = temp2Wm1;
        temp2[sizeY-2] = B * temp2[sizeY-2] + b1*temp2[sizeY-1] + b2*temp2W + b3*temp2Wp1;
        temp2[sizeY-3] = B * temp2[sizeY-3] + b1*temp2[sizeY-2] + b2*temp2[sizeY-1] + b3*temp2W;

            for (int j=sizeY-4; j>=0; j--)
                temp2[j] = B * temp2[j] + b1*temp2[j+1] + b2*temp2[j+2] + b3*temp2[j+3];
            for (int j=0; j<sizeY; j++)
                src(j,i,0) = (double)temp2[j];

        }
}

//------------------------------------------------------------------------------//

void GaussFilter::gaussFiltering(CImg<double> &src){
    gaussVertical(src);
    gaussHorizontal(src);
}

//------------------------------------------------------------------------------//

void GaussFilter::spaceVariantGaussHorizontal(CImg<double> &src){

#pragma omp parallel for

    for (int i=0; i<sizeX; i++) {

        double* temp2 = buffer + omp_get_thread_num()*(sizeX+sizeY);

        temp2[0] = B_m(0,i,0)*(double)src(0,i,0) + b1_m(0,i,0)*(double)src(0,i,0) + b2_m(0,i,0)*(double)src(0,i,0) + b3_m(0,i,0)*(double)src(0,i,0);
        temp2[1] = B_m(1,i,0)*(double)src(1,i,0) + b1_m(1,i,0)*temp2[0]  + b2_m(1,i,0)*(double)src(0,i,0) + b3_m(1,i,0)*(double)src(0,i,0);
        temp2[2] = B_m(2,i,0)*(double)src(2,i,0) + b1_m(2,i,0)*temp2[1]  + b2_m(2,i,0)*temp2[0]  + b3_m(2,i,0)*(double)src(0,i,0);

        for (int j=3; j<sizeY; j++)
            temp2[j] = B_m(j,i,0)*(double)src(j,i,0) + b1_m(j,i,0)*temp2[j-1] + b2_m(j,i,0)*temp2[j-2] + b3_m(j,i,0)*temp2[j-3];

        double temp2Wm1 = (double)src(sizeY-1,i,0) + M_m(0,i,0)*(temp2[sizeY-1] - (double)src(sizeY-1,i,0)) + M_m(0,i,1)*(temp2[sizeY-2] - (double)src(sizeY-1,i,0)) + M_m(0,i,2)*(temp2[sizeY-3] - (double)src(sizeY-1,i,0));
        double temp2W   = (double)src(sizeY-1,i,0) + M_m(0,i,3)*(temp2[sizeY-1] - (double)src(sizeY-1,i,0)) + M_m(0,i,4)*(temp2[sizeY-2] - (double)src(sizeY-1,i,0)) + M_m(0,i,5)*(temp2[sizeY-3] - (double)src(sizeY-1,i,0));
        double temp2Wp1 = (double)src(sizeY-1,i,0) + M_m(0,i,6)*(temp2[sizeY-1] - (double)src(sizeY-1,i,0)) + M_m(0,i,7)*(temp2[sizeY-2] - (double)src(sizeY-1,i,0)) + M_m(0,i,8)*(temp2[sizeY-3] - (double)src(sizeY-1,i,0));

        temp2[sizeY-1] = temp2Wm1;
        temp2[sizeY-2] = B_m(sizeY-2,i,0) * temp2[sizeY-2] + b1_m(sizeY-2,i,0)*temp2[sizeY-1] + b2_m(sizeY-2,i,0)*temp2W + b3_m(sizeY-2,i,0)*temp2Wp1;
        temp2[sizeY-3] = B_m(sizeY-3,i,0) * temp2[sizeY-3] + b1_m(sizeY-3,i,0)*temp2[sizeY-2] + b2_m(sizeY-3,i,0)*temp2[sizeY-1] + b3_m(sizeY-3,i,0)*temp2W;

            for (int j=sizeY-4; j>=0; j--)
                temp2[j] = B_m(j,i,0) * temp2[j] + b1_m(j,i,0)*temp2[j+1] + b2_m(j,i,0)*temp2[j+2] + b3_m(j,i,0)*temp2[j+3];
            for (int j=0; j<sizeY; j++)
                src(j,i,0) = (double)temp2[j];

        }

}

//------------------------------------------------------------------------------//

void GaussFilter::spaceVariantGaussVertical(CImg<double> &src){

#pragma omp parallel for

    for (int i=0; i<sizeY; i++) {

        double* temp2 = buffer + omp_get_thread_num()*(sizeX+sizeY);

        temp2[0] = B_m(i,0,0)*(double)src(i,0,0) + b1_m(i,0,0)*(double)src(i,0,0) + b2_m(i,0,0)*(double)src(i,0,0) + b3_m(i,0,0)*(double)src(i,0,0);
        temp2[1] = B_m(i,1,0)*(double)src(i,1,0) + b1_m(i,1,0)*temp2[0]  + b2_m(i,1,0)*(double)src(i,0,0) + b3_m(i,1,0)*(double)src(i,0,0);
        temp2[2] = B_m(i,2,0)*(double)src(i,2,0) + b1_m(i,2,0)*temp2[1]  + b2_m(i,2,0)*temp2[0]  + b3_m(i,2,0)*(double)src(i,0,0);

        for (int j=3; j<sizeX; j++)
            temp2[j] = B_m(i,j,0)*(double)src(i,j,0) + b1_m(i,j,0)*temp2[j-1] + b2_m(i,j,0)*temp2[j-2] + b3_m(i,j,0)*temp2[j-3];

        double temp2Wm1 = (double)src(i,sizeX-1,0) + M_m(i,0,0)*(temp2[sizeX-1] - (double)src(i,sizeX-1,0)) + M_m(i,0,1)*(temp2[sizeX-2] - (double)src(i,sizeX-1,0)) + M_m(i,0,2)*(temp2[sizeX-3] - (double)src(i,sizeX-1,0));
        double temp2W   = (double)src(i,sizeX-1,0) + M_m(i,0,3)*(temp2[sizeX-1] - (double)src(i,sizeX-1,0)) + M_m(i,0,4)*(temp2[sizeX-2] - (double)src(i,sizeX-1,0)) + M_m(i,0,5)*(temp2[sizeX-3] - (double)src(i,sizeX-1,0));
        double temp2Wp1 = (double)src(i,sizeX-1,0) + M_m(i,0,6)*(temp2[sizeX-1] - (double)src(i,sizeX-1,0)) + M_m(i,0,7)*(temp2[sizeX-2] - (double)src(i,sizeX-1,0)) + M_m(i,0,8)*(temp2[sizeX-3] - (double)src(i,sizeX-1,0));

        temp2[sizeX-1] = temp2Wm1;
        temp2[sizeX-2] = B_m(i,sizeX-2,0) * temp2[sizeX-2] + b1_m(i,sizeX-2,0)*temp2[sizeX-1] + b2_m(i,sizeX-2,0)*temp2W + b3_m(i,sizeX-2,0)*temp2Wp1;
        temp2[sizeX-3] = B_m(i,sizeX-3,0) * temp2[sizeX-3] + b1_m(i,sizeX-3,0)*temp2[sizeX-2] + b2_m(i,sizeX-3,0)*temp2[sizeX-1] + b3_m(i,sizeX-3,0)*temp2W;

            for (int j=sizeX-4; j>=0; j--)
                temp2[j] = B_m(i,j,0) * temp2[j] + b1_m(i,j,0)*temp2[j+1] + b2_m(i,j,0)*temp2[j+2] + b3_m(i,j,0)*temp2[j+3];
            for (int j=0; j<sizeX; j++)
                src(i,j,0) = (double)temp2[j];
        }
}

//------------------------------------------------------------------------------//

void GaussFilter::spaceVariantGaussFiltering(CImg<double> &src){
    spaceVariantGaussVertical(src);
    spaceVariantGaussHorizontal(src);

}

//------------------------------------------------------------------------------//

void GaussFilter::feedInput(const CImg<double> &new_input, bool isCurrent, int port){
      // copy input image
    *inputImage=new_input;
}

//------------------------------------------------------------------------------//


void GaussFilter::update(){
    if(spaceVariantSigma)
        spaceVariantGaussFiltering(*inputImage);
    else
        gaussFiltering(*inputImage);

    *outputImage = *inputImage;
}

//------------------------------------------------------------------------------//

CImg<double>* GaussFilter::getOutput(){
    return outputImage;
}

//------------------------------------------------------------------------------//

double GaussFilter::density(double r){
    // cell density
    // r in degrees
    return (r>R0)?  1/(1+K*(r-R0)) : 1 ;
}

//------------------------------------------------------------------------------//


bool GaussFilter::setParameters(vector<double> params, vector<string> paramID){

    bool correct = true;

    for (int i = 0;i<params.size();i++){
        const char * s = paramID[i].c_str();

        if (strcmp(s,"sigma")==0){
            sigma = params[i];
        }else if (strcmp(s,"True")==0){
            spaceVariantSigma = true;
        }else if (strcmp(s,"False")==0){
            spaceVariantSigma = false;
        }else if (strcmp(s,"K")==0){
            K = params[i];
        }else if (strcmp(s,"R0")==0){
            R0 = params[i];
        }
        else{
              correct = false;
        }

    }

    return correct;

}
