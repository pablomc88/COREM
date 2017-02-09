#include "SingleCompartment.h"

SingleCompartment::SingleCompartment(int x, int y, double temporal_step):module(x,y,temporal_step){

    Cm=10^(-9);
    Rm=1.0;
    taum=10^(-6);
    El=0.0;

    number_current_ports=0;
    number_conductance_ports=0;

    conductances=NULL;
    currents=NULL;

    current_potential=new CImg<double> (sizeY,sizeX,1,1,0.0);
    last_potential=new CImg<double> (sizeY,sizeX,1,1,0.0);
    total_cond=new CImg<double> (sizeY,sizeX,1,1,0.0);
    potential_inf=new CImg<double> (sizeY,sizeX,1,1,0.0);
    tau=new CImg<double> (sizeY,sizeX,1,1,0.0);
    exp_term=new CImg<double> (sizeY,sizeX,1,1,0.0);
}

SingleCompartment::SingleCompartment(const SingleCompartment &copy):module(copy){

    number_current_ports=copy.number_current_ports;
    number_conductance_ports=copy.number_conductance_ports;
    Cm = copy.Cm;
    Rm = copy.Rm;
    taum = copy.taum;
    El = copy.El;

    conductances = new CImg<double>*[number_conductance_ports];
    currents = new CImg<double>*[number_current_ports];

    for (int i=0;i<number_conductance_ports;i++)
        conductances[i]=new CImg<double> (*(copy.currents[i]));
    for (int j=0;j<number_current_ports;j++)
        currents[j]=new CImg<double> (*(copy.currents[j]));

    current_potential=new CImg<double>(*(copy.current_potential));
    last_potential=new CImg<double>(*(copy.last_potential));
    total_cond=new CImg<double>(*(copy.total_cond));
    potential_inf=new CImg<double>(*(copy.potential_inf));
    tau=new CImg<double>(*(copy.tau));
    exp_term=new CImg<double>(*(copy.exp_term));
}

SingleCompartment::~SingleCompartment(){
    if(conductances!=NULL){
        for (int i=0;i<number_conductance_ports;i++)
            delete conductances[i];
        delete[] conductances;
    }
    if(currents!=NULL){
        for (int j=0;j<number_current_ports;j++)
            delete currents[j];
        delete[] currents;
    }

    if(current_potential!=NULL) delete current_potential;
    if(last_potential!=NULL) delete last_potential;
    if(total_cond!=NULL) delete total_cond;
    if(potential_inf!=NULL) delete potential_inf;
    if(tau!=NULL) delete tau;
    if(exp_term!=NULL) delete exp_term;
}

//------------------------------------------------------------------------------//


bool SingleCompartment::allocateValues(){
    // Just in case allocateValues() is called several times, check if arrays were
    // already allocated, before allocating them again
    if(conductances!=NULL){
        for (int i=0;i<number_conductance_ports;i++)
            delete conductances[i];
        delete[] conductances;
    }
    if(currents!=NULL){
        for (int j=0;j<number_current_ports;j++)
            delete currents[j];
        delete[] currents;
    }
    // Allocate memory according to new dimensions
    conductances = new CImg<double>*[number_conductance_ports];
    currents = new CImg<double>*[number_current_ports];

    for (int i=0;i<number_conductance_ports;i++)
        conductances[i]=new CImg<double> (sizeY,sizeX,1,1,0.0);
    for (int j=0;j<number_current_ports;j++)
        currents[j]=new CImg<double> (sizeY,sizeX,1,1,0.0);
        
    // Ajust image sizes to new dimensions (just in case they have chanded)
    current_potential->assign(sizeY, sizeX, 1, 1, 0);
    last_potential->assign(sizeY, sizeX, 1, 1, 0);
    total_cond->assign(sizeY, sizeX, 1, 1, 0);
    potential_inf->assign(sizeY, sizeX, 1, 1, 0);
    tau->assign(sizeY, sizeX, 1, 1, 0);
    exp_term->assign(sizeY, sizeX, 1, 1, 0);

    return(true);
}

bool SingleCompartment::set_Cm(double capacitance){
    bool ret_correct;
    if (capacitance>0) {
        Cm = capacitance;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SingleCompartment::set_Rm(double resistance){
    bool ret_correct;
    if (resistance>=0) {
        Rm = resistance;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SingleCompartment::set_taum(double temporal_constant){
    bool ret_correct;
    if (temporal_constant>=0) {
        taum = temporal_constant;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SingleCompartment::set_El(double Nerst_l){
    bool ret_correct;
    El = Nerst_l;
    ret_correct=true;
    return(ret_correct);
}

bool SingleCompartment::set_E(double NernstPotential, int port){
    bool ret_correct;
    if (port>=0 && number_conductance_ports>0) {
        E[port]=NernstPotential;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SingleCompartment::set_number_current_ports(int number){
    bool ret_correct;
    if (number>0) {
        if(currents!=NULL){
            for (int j=0;j<number_current_ports;j++)
                delete currents[j];
            delete[] currents;
            currents=NULL;
        }
        number_current_ports = number;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SingleCompartment::set_number_conductance_ports(int number){
    bool ret_correct;
    if (number>0) {
        if(conductances!=NULL){
            for (int i=0;i<number_conductance_ports;i++)
                delete conductances[i];
            delete[] conductances;
            conductances=NULL;
        }

        number_conductance_ports = number;

        for (int i=0;i<number_conductance_ports;i++){
            E.push_back(0.0);
    }
        ret_correct=true;
    } else
        ret_correct=true;
    return(ret_correct);
}

//------------------------------------------------------------------------------//

bool SingleCompartment::setParameters(vector<double> params, vector<string> paramID){

    bool correct = true;

    int g_port = 0;

    for (vector<double>::size_type i = 0;i < params.size() && correct;i++){
        const char * s = paramID[i].c_str();


        if (strcmp(s,"number_current_ports")==0){
            correct = set_number_current_ports((int)params[i]);
        }else if (strcmp(s,"number_conductance_ports")==0){
            correct = set_number_conductance_ports((int)params[i]);
        }else if (strcmp(s,"Rm")==0){
            correct = set_Rm(params[i]);
        }else if (strcmp(s,"tau")==0){
            correct = set_taum(params[i]);
        }
        else if (strcmp(s,"Cm")==0){
            correct = set_Cm(params[i]);
        }
        else if (strcmp(s,"E")==0){
            if(number_conductance_ports == 0)
                correct = set_El(params[i]);
            else if(number_conductance_ports > 1){
                if(g_port < E.size())
                    set_E(params[i],g_port);
                else
                    correct = false;
            }else
                correct = false;

            g_port+=1;
        }
        else{
              correct = false;
        }

    }

    return correct;

}

//------------------------------------------------------------------------------//

void SingleCompartment::feedInput(double sim_time, const CImg<double>& new_input,bool isCurrent,int port){

    // the parameter 'port' corresponds to both current and conductance ports.
    // Next piece of code adapts port to its correct range.
    int number_of_currents = 0;
    int number_of_conductances = 0;
    for(size_t k=0;k<typeSynapse.size();k++){

        if ((int)k<port){ // For each synapse (input module) of this module, count the number of previous synpses in the list 
            if (typeSynapse[k]==0)
                number_of_currents+=1; // Accumulate the number of previous current port in the list
            else
                number_of_conductances+=1;

        }else if((int)k==port){ // Subtract the number of synspses of the type different from the current one to get the relative position
            if (isCurrent)
                port = port - number_of_conductances;
            else
                port = port - number_of_currents;
        }

    }

    // feed new input
    if(isCurrent){ // type is Current
        if(port<number_current_ports)
            *(currents[port])=new_input;
        else
            cout << "Warning: Found 'Current' input number " << port+1 << " in SingleCompartment module but only " << number_current_ports << " 'Current' ports have been defined in parameters" << endl;
    }else{ // type is Conductance
        if(port<number_conductance_ports)
            *(conductances[port])=new_input;
        else
            cout << "Warning: Found 'Conductance' input number " << port+1 << " in SingleCompartment module but only " << number_conductance_ports << " 'Conductance' ports have been defined in parameters" << endl;
    }

}

//------------------------------------------------------------------------------//

void SingleCompartment::update(){

    (*last_potential)=(*current_potential);
    (*current_potential).fill(0.0);

    // When there are conductance ports
    if (number_conductance_ports>0){

        (*total_cond) = (*(conductances[0]));
        for(int k=1;k<number_conductance_ports-1;k++){
            (*total_cond) += (*(conductances[k]));
        }
        // g_L
        if(Rm > 0.0)
            (*total_cond) += 1.0 / Rm;
        // in case total_cond = 0
        (*total_cond) += DBL_EPSILON;

     // tau
        tau->fill(Cm);
        tau->div((*total_cond));

     // potential at infinity
        (*potential_inf) = (*(conductances[0]))*E[0];

        for(int k=1;k<number_conductance_ports-1;k++){
            (*potential_inf) += (*(conductances[k]))*E[k];
        }
        // E_L
        if(Rm > 0.0)
            (*potential_inf) += (1.0 / Rm)*E[number_conductance_ports-1];

        for(int k=0;k<number_current_ports;k++){
            (*potential_inf) += (*(currents[k]));
        }

        potential_inf->div((*total_cond));

      // When there are only current ports
      }else{
        //tau
        tau->fill(taum);
        // potential at infinity
        potential_inf->fill(El); // El is set equal to E parameter
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
