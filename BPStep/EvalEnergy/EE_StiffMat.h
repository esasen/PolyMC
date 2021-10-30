#ifndef __EE_STIFFMAT_INCLUDED__
#define __EE_STIFFMAT_INCLUDED__

#include "../EvalEnergy.h"

class EE_StiffMat;

class EE_StiffMat: public EvalEnergy
{
//////////////////////////////////////////////////////////////////////////////////
// member variables //////////////////////////////////////////////////////////////
protected:
    arma::mat stiffmat;
    arma::mat current_stiffmat;
    double    factor;

public:
////////////////////////////////////////////////////////////////////////////////////
/////// constructor / destructor ///////////////////////////////////////////////////
    EE_StiffMat(const std::vector<double> & params,double disc_len, double temp, bool is_diag);
    ~EE_StiffMat();

////////////////////////////////////////////////////////////////////////////////////
/////////////////   main   /////////////////////////////////////////////////////////
    double cal_beta_energy_diag(const arma::colvec & Theta);
    double cal_beta_energy_offdiag(const arma::colvec & Theta1, const arma::colvec & Theta2);
////////////////////////////////////////////////////////////////////////////////////
/////////////////  setters /////////////////////////////////////////////////////////
    void set_temp(double temp);
    void set_params(const std::vector<double> & new_params);
    void set_current_stiffmat();

    void deactivate_twist_energy();
    void reactivate_twist_energy();

////////////////////////////////////////////////////////////////////////////////////
/////////////////  getters /////////////////////////////////////////////////////////
    arma::mat get_cov();
    bool isotropic_bending();

};

#endif