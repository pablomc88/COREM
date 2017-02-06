#ifndef LINEARFILTER_H
#define LINEARFILTER_H

/* BeginDocumentation
 * Name: LinearFilter
 *
 * Description: temporal recursive linear filtering:
 * y(k) =( -( a(1).y(k-1)+..+a(N).y(k-N) )  + b(0).x(k)+...+b(M-1).x(k-M+1) )/a(0)
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *
 * Source code adapted from Virtual Retina [1] (licensed under CeCILL-C)
 *
 * [1] Wohrer, Adrien, and Pierre Kornprobst. "Virtual Retina: a biological retina
 * model and simulator, with contrast gain control." Journal of computational
 * neuroscience 26.2 (2009): 219-249.
 *
 * SeeAlso:module
 */


#include "module.h"

using namespace cimg_library;
using namespace std;

class LinearFilter:public module{
protected:
    // filter parameters
    int M;
    int N;
    double* a;
    double* b;

    // recursion buffers
    CImg<double>** last_inputs;
    CImg<double>** last_values;

    double initial_input_value;

public:
    // Constructor, copy, destructor.
    LinearFilter(int x=1,int y=1,double temporal_step=1.0,double initial_value=0.0);
    LinearFilter(const LinearFilter& copy);
    ~LinearFilter(void);

    // Allocate values and set protected parameters
    virtual bool allocateValues();

    // Exponential and gamma filter
    bool Exp(double tau);
    bool Gamma(double tau,int n);

    // New input and update of equations
    virtual void feedInput(double sim_time, const CImg<double> &new_input, bool isCurrent, int port);
    virtual void update();

    // Get output image (y(k))
    virtual CImg<double>* getOutput();

    // set Parameters
    virtual bool setParameters(vector<double> params, vector<string> paramID);

    // Combinatorials of gamma function:
    inline int arrangement(int n, int k)
    {
      int res=1;
      for(int i=n;i>n-k;--i)
          res*=i;

      return res;
    }

    inline int factorial(int n)
    {return arrangement(n,n);}

    inline int combination(int n, int k)
    {return arrangement(n,k)/factorial(k);}
};

#endif // LINEARFILTER_H
