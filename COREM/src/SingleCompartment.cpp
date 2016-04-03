#include "SingleCompartment.h"

SingleCompartment::SingleCompartment(int x, int y, double temporal_step):module(x,y,temporal_step){

    Cm=10^(-9);
    Rm=1.0;
    taum=10^(-6);
    El=0.0;

    number_current_ports=0;
    number_conductance_ports=0;

    conductances=0;
    currents=0;

    current_potential=new CImg<double> (sizeY,sizeX,1,1,0.0);
    last_potential=new CImg<double> (sizeY,sizeX,1,1,0.0);
    total_cond=new CImg<double> (sizeY,sizeX,1,1,0.0);
    potential_inf=new CImg<double> (sizeY,sizeX,1,1,0.0);
    tau=new CImg<double> (sizeY,sizeX,1,1,0.0);
    exp_term=new CImg<double> (sizeY,sizeX,1,1,0.0);
}

SingleCompartment::SingleCompartment(const SingleCompartment &copy){

    step=copy.step;
    sizeX=copy.sizeX;
    sizeY=copy.sizeY;

    number_current_ports=copy.number_current_ports;
    number_conductance_ports=copy.number_conductance_ports;
    Cm = copy.Cm;
    Rm = copy.Rm;
    taum = copy.taum;
    El = copy.El;

    conductances=0;
    currents=0;

    current_potential=new CImg<double> (sizeY,sizeX,1,1,0.0);
    last_potential=new CImg<double> (sizeY,sizeX,1,1,0.0);
    total_cond=new CImg<double> (sizeY,sizeX,1,1,0.0);
    potential_inf=new CImg<double> (sizeY,sizeX,1,1,0.0);
    tau=new CImg<double> (sizeY,sizeX,1,1,0.0);
    exp_term=new CImg<double> (sizeY,sizeX,1,1,0.0);
}

SingleCompartment::~SingleCompartment(){
    if(conductances) delete[] conductances;
    if(currents) delete[] currents;

    if(current_potential) delete current_potential;
    if(last_potential) delete last_potential;
    if(total_cond) delete total_cond;
    if(potential_inf) delete potential_inf;
    if(tau) delete tau;
    if(exp_term) delete exp_term;
}

//------------------------------------------------------------------------------//


void SingleCompartment::allocateValues(){
    conductances = new CImg<double>*[number_conductance_ports];
    currents = new CImg<double>*[number_current_ports];

    for (int i=0;i<number_conductance_ports;i++){
      conductances[i]=new CImg<double> (sizeY,sizeX,1,1,0.0);
    }
    for (int j=0;j<number_current_ports;j++)
      currents[j]=new CImg<double> (sizeY,sizeX,1,1,0.0);


}


SingleCompartment& SingleCompartment::set_Cm(double capacitance){
    if (capacitance>0)
        Cm = capacitance;
}

SingleCompartment& SingleCompartment::set_Rm(double resistance){
    if (resistance>0)
        Rm = resistance;
}

SingleCompartment& SingleCompartment::set_taum(double temporal_constant){
    if (temporal_constant>0)
        taum = temporal_constant;
}

SingleCompartment& SingleCompartment::set_El(double Nerst_l){
    if (Nerst_l>=0)
        El = Nerst_l;
}

SingleCompartment& SingleCompartment::set_E(double NernstPotential, int port){
    if (port>=0 && number_conductance_ports>0)
        E[port]=NernstPotential;
}

SingleCompartment& SingleCompartment::set_number_current_ports(int number){
    if (number>0)
        number_current_ports = number;
}

SingleCompartment& SingleCompartment::set_number_conductance_ports(int number){
    if (number>0)
        number_conductance_ports = number;

    for (int i=0;i<number_conductance_ports;i++){
      E.push_back(0.0);
    }
}

//------------------------------------------------------------------------------//

bool SingleCompartment::setParameters(vector<double> params, vector<string> paramID){

    bool correct = true;
    int port = 0;

    for (int i = 0;i<params.size();i++){
        const char * s = paramID[i].c_str();

        if (strcmp(s,"number_current_ports")==0){
            set_number_current_ports((int)params[i]);
        }else if (strcmp(s,"number_conductance_ports")==0){
            set_number_conductance_ports((int)params[i]);
        }else if (strcmp(s,"Rm")==0){
            set_Rm(params[i]);
        }else if (strcmp(s,"tau")==0){
            set_taum(params[i]);
        }
        else if (strcmp(s,"Cm")==0){
            set_Cm(params[i]);
        }
        else if (strcmp(s,"E")==0){
            set_El(params[i]);
            for(int j=0;j<E.size();j++)
                set_E(params[i],j);
        }
        else{
              correct = false;
        }

    }

    return correct;

}

//------------------------------------------------------------------------------//

void SingleCompartment::feedInput(const CImg<double>& new_input,bool isCurrent,int port){

    // the parameter 'port' corresponds to both current and conductance ports.
    // Next piece of code adapts port to its correct range.
    int number_of_currents = 0;
    int number_of_conductances = 0;
    for(int k=0;k<typeSynapse.size();k++){

        if (k<port){
            if (typeSynapse[k]==0)
                number_of_currents+=1;
            else
                number_of_conductances+=1;

        }else if(k==port){
            if (isCurrent)
                port = port - number_of_conductances;
            else
                port = port - number_of_currents;
        }

    }

    // feed new input
        if(isCurrent && port<number_current_ports){
            *(currents[port])=new_input;
        }else if(port<number_conductance_ports){
            *(conductances[port])=new_input;
        }

}

//------------------------------------------------------------------------------//

void SingleCompartment::update(){

    (*last_potential)=(*current_potential);
    (*current_potential).fill(0.0);

    // When there are conductance ports
    if (number_conductance_ports>0){

        (*total_cond) = (*(conductances[0]));
        for(int k=1;k<number_conductance_ports;k++){
            (*total_cond) += (*(conductances[k]));
        }
        // in case total_cond = 0
        (*total_cond) += DBL_EPSILON;

     // tau
        tau->fill(Cm);
        tau->div((*total_cond));

     // potential at infinity
        (*potential_inf) = (*(conductances[0]))*E[0];

        for(int k=1;k<number_conductance_ports;k++){
            (*potential_inf) += (*(conductances[k]))*E[k];
        }
        for(int k=0;k<number_current_ports;k++){
            (*potential_inf) += (*(currents[k]));
        }

        potential_inf->div((*total_cond));

      // When there are only current ports
      }else{
        //tau
        tau->fill(taum);
        // potential at infinity
        potential_inf->fill(El);
        for(int k=0;k<number_current_ports;k++){
            (*potential_inf) += (*(currents[k]))*Rm;
        }
    }

        // exponential term
        exp_term->fill(-step);
        exp_term->div(*tau);
        exp_term->exp();

        // membrane potential update

        (*current_potential) += (*potential_inf);
        (*current_potential) -= (*potential_inf).mul(*exp_term);
        (*current_potential) += (*last_potential).mul(*exp_term);



}

//------------------------------------------------------------------------------//


CImg<double>* SingleCompartment::getOutput(){
    return current_potential;
}
