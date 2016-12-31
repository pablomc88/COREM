#include "LinearFilter.h"

LinearFilter::LinearFilter(int x, int y, double temporal_step,double initial_value):module(x,y,temporal_step){
    M=1;N=0;
    a=new double[N+1];
    b=new double[M];
    a[0]=1;
    b[0]=1;

    initial_input_value=initial_value;
    last_inputs=NULL;
    last_values=NULL;
}

LinearFilter::LinearFilter(const LinearFilter &copy):module(copy){

    M=copy.M;
    N=copy.N;

    b=new double[M];
    for(int i=0;i<M;++i)
        b[i]=copy.b[i];

    a=new double[N+1];
    for(int i=0;i<N+1;++i)
        a[i]=copy.a[i];

    initial_input_value=copy.initial_input_value;

    last_inputs = new CImg<double>*[M];
    last_values = new CImg<double>*[N+1];

    last_inputs[0]=new CImg<double>(*(copy.last_inputs[0]));
    for (int i=1;i<M;i++)
        last_inputs[i]=new CImg<double>(*(copy.last_inputs[i]));
    for (int j=0;j<N+1;j++)
        last_values[j]=new CImg<double>(*(copy.last_values[j]));
}

LinearFilter::~LinearFilter(){
    if(a!=NULL) delete[] a;
    if(b!=NULL) delete[] b;

    if(last_inputs!=NULL){
        for (int i=0;i<M;i++)
            delete last_inputs[i];
        delete[] last_inputs;
    }
    if(last_values!=NULL){
        for (int j=0;j<N+1;j++)
            delete last_values[j];
        delete[] last_values;
    }
}

//------------------------------------------------------------------------------//


bool LinearFilter::allocateValues(){
    //Check if allocateValues() was called before, free memory before allocating
    if(last_inputs!=NULL){
        for (int i=0;i<M;i++)
            delete last_inputs[i];
        delete[] last_inputs;
    }
    if(last_values!=NULL){
        for (int j=0;j<N+1;j++)
            delete last_values[j];
        delete[] last_values;
    }
    
    last_inputs = new CImg<double>*[M];
    last_values = new CImg<double>*[N+1];

    last_inputs[0]=new CImg<double> (sizeY,sizeX,1,1,0.0);
    for (int i=1;i<M;i++)
        last_inputs[i]=new CImg<double> (sizeY,sizeX,1,1,initial_input_value);
    for (int j=0;j<N+1;j++)
        last_values[j]=new CImg<double> (sizeY,sizeX,1,1,0.0);
    return(true);
}

//------------------------------------------------------------------------------//

bool LinearFilter::Exp(double tau){
    bool val_correct;
    if(tau>0)
    {
        M=1;
        N=1;
        a=new double[N+1]; 
        b=new double[M];
        a[0]=1; a[1]=-exp(-step/tau);
        b[0]=1-exp(-step/tau);
        val_correct=true;
    } else
        val_correct=false;
    return(val_correct);
}

bool LinearFilter::Gamma(double tau,int n){
    bool val_correct;
    if(tau>0 && n>=0)
    {
        M=1; N=n+1;
        double tauC=n? tau/n : tau;
        double c=exp(-step/tauC);

        b=new double[1];
        b[0]=pow(1-c,N);
        a=new double[N+1];

        for(int i=0;i<=N;++i)
            a[i]=pow(-c,i)*combination(N,i);
        val_correct=true;
    } else
        val_correct=false;
    return(val_correct);
}


//------------------------------------------------------------------------------//

bool LinearFilter::setParameters(vector<double> params, vector<string> paramID){

    bool correct = true;
    double tau = 0.0;
    int n = 0;
    int type = 0;

    for (vector<double>::size_type i = 0;i < params.size() && correct;i++){
        const char * s = paramID[i].c_str();

        if (strcmp(s,"tau")==0){
            tau = params[i];
        }else if (strcmp(s,"n")==0){
            n = (int)params[i];
        }else if (strcmp(s,"Exp")==0){
            type = 0;
        }else if (strcmp(s,"Gamma")==0){
            type = 1;
        }
        else{
              correct = false;
        }
    }

    if(correct){

        switch(type){
        case 0:
            Exp(tau); // TODO: Move memory code for popullation, allocation/reallocation to the constructor/allocateValues()
            break;
        case 1:
            Gamma(tau,n);
            break;
        default:
            correct = false;
            break;
        }
    }

    return correct;

}

//------------------------------------------------------------------------------//
#include <iostream>

void LinearFilter::feedInput(double sim_time, const CImg<double>& new_input, bool isCurrent, int port){

    *(last_inputs[0])=new_input;
}

//------------------------------------------------------------------------------//

void LinearFilter::update(){

    // Rotation on addresses of the last_values.
    CImg<double>* fakepoint=last_values[N];
    for(int i=1;i<N+1;++i) // last_values has N+1 elements (image pointers)
      last_values[N+1-i]=last_values[N-i];
    last_values[0]=fakepoint;

    // Calculating new value of filter recursively:
    *(last_values[0]) = b[0]* (*(last_inputs[0]));
    for(int j=1;j<M;j++)
      *(last_values[0]) += ( b[j] * (*(last_inputs[j])) );
    for(int k=1;k<N+1;k++)
      *(last_values[0]) -= ( a[k] * (*(last_values[k])) );
    if(a[0]!=1)
      ( *(last_values[0]) )/=a[0];


    //Reinitialization procedure
      if(M>1)
      {
        for(int i=1;i<M;++i)
          last_inputs[M-i]=last_inputs[M-i-1];
      }
//      last_inputs[0]->fill(0.0);
}

//------------------------------------------------------------------------------//


CImg<double>* LinearFilter::getOutput(){
    return last_values[0];
}
