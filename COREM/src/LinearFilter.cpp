#include "LinearFilter.h"

LinearFilter::LinearFilter(int x, int y, double temporal_step,double initial_value):module(x,y,temporal_step){
    a=new double[1];
    a[0]=1;
    b=new double[1];
    b[0]=1;
    M=1;N=0;

    initial_input_value=initial_value;
    last_inputs=0;
    last_values=0;

}

LinearFilter::LinearFilter(const LinearFilter &copy){

    M=copy.M;
    N=copy.N;

    b=new double[M];
    for(int i=0;i<M;++i)
        b[i]=copy.b[i];

    a=new double[N+1];
    for(int i=0;i<N+1;++i)
        a[i]=copy.a[i];

    step=copy.step;
    sizeX=copy.sizeX;
    sizeY=copy.sizeY;

    initial_input_value=copy.initial_input_value;
    last_inputs=0;
    last_values=0;


}

LinearFilter::~LinearFilter(){
    if(a) delete[] a;
    if(b) delete[] b;

    if(last_inputs) delete[] last_inputs;
    if(last_values) delete[] last_values;
}

//------------------------------------------------------------------------------//


void LinearFilter::allocateValues(){
    last_inputs = new CImg<double>*[M];
    last_values = new CImg<double>*[N+1];

    last_inputs[0]=new CImg<double> (sizeY,sizeX,1,1,0.0);
    for (int i=1;i<M;i++)
      last_inputs[i]=new CImg<double> (sizeY,sizeX,1,1,initial_input_value);
    for (int j=0;j<N+1;j++)
      last_values[j]=new CImg<double> (sizeY,sizeX,1,1,0.0);

}


//------------------------------------------------------------------------------//

LinearFilter& LinearFilter::Exp(double tau){

    if(tau>0)
    {
      M=1;
      N=1;
      a=new double[N+1];
      b=new double[M];
      a[0]=1; a[1]=-exp(-step/tau);
      b[0]=1-exp(-step/tau);
    }

}

LinearFilter& LinearFilter::Gamma(double tau,int n){

    if(tau>0 && n>=0)
    {
       M=1; N=n+1;
       double tauC=n? tau/n : tau;
       double c=exp(-step/tauC);

       b=new double[1]; b[0]=pow(1-c,N);
       a=new double[N+1];

       for(int i=0;i<=N;++i)
           a[i]=pow(-c,i)*combination(N,i);
    }

}


//------------------------------------------------------------------------------//

bool LinearFilter::setParameters(vector<double> params, vector<string> paramID){

    bool correct = true;
    double tau = 0.0;
    int n = 0;
    int type = 0;

    for (int i = 0;i<params.size();i++){
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
            Exp(tau);
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


void LinearFilter::feedInput(const CImg<double>& new_input, bool isCurrent, int port){

    *(last_inputs[0])=new_input;

}

//------------------------------------------------------------------------------//

void LinearFilter::update(){

    // Rotation on addresses of the last_values.
    CImg<double>* fakepoint=last_values[N];
    for(int i=1;i<N+1;++i)
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
