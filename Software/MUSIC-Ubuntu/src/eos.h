// MUSIC - a 3+1D viscous relativistic hydrodynamic code for heavy ion collisions
// Copyright (C) 2017  Gabriel Denicol, Charles Gale, Sangyong Jeon, Matthew Luzum, Jean-François Paquet, Björn Schenke, Chun Shen

// Copyright 2011 @ Bjoern Schenke, Sangyong Jeon, and Charles Gale
#ifndef SRC_EOS_H_
#define SRC_EOS_H_

#include "util.h"
#include "data.h"
#include <iostream>
#include "gsl/gsl_interp.h"
#include "gsl/gsl_spline.h"
#include "gsl/gsl_errno.h"
#include "./pretty_ostream.h"

//! This is the class that handles equation of state

class EOS {
 private:
    InitData *parameters_ptr;
    pretty_ostream music_message;

    double BNP1, EPP1;            // start value for \mu_B and epsilon
    double BNP2, EPP2;            // start value for \mu_B and epsilon
    double BNP3, EPP3;            // start value for \mu_B and epsilon
    double BNP4, EPP4;            // start value for \mu_B and epsilon
    double BNP5, EPP5;            // start value for \mu_B and epsilon
    double BNP6, EPP6;            // start value for \mu_B and epsilon
    double BNP7, EPP7;            // start value for \mu_B and epsilon

    double deltaBNP1, deltaEPP1;  // step size for \mu_B and epsilon
    double deltaBNP2, deltaEPP2;  // step size for \mu_B and epsilon
    double deltaBNP3, deltaEPP3;  // step size for \mu_B and epsilon
    double deltaBNP4, deltaEPP4;  // step size for \mu_B and epsilon
    double deltaBNP5, deltaEPP5;  // step size for \mu_B and epsilon
    double deltaBNP6, deltaEPP6;  // step size for \mu_B and epsilon
    double deltaBNP7, deltaEPP7;  // step size for \mu_B and epsilon

    int NBNP1, NEPP1;             // number of entries for \mu_B and epsilon
    int NBNP2, NEPP2;             // number of entries for \mu_B and epsilon
    int NBNP3, NEPP3;             // number of entries for \mu_B and epsilon
    int NBNP4, NEPP4;             // number of entries for \mu_B and epsilon
    int NBNP5, NEPP5;             // number of entries for \mu_B and epsilon
    int NBNP6, NEPP6;             // number of entries for \mu_B and epsilon
    int NBNP7, NEPP7;             // number of entries for \mu_B and epsilon
    
    double **pressure1;
    double **pressure2;
    double **pressure3;
    double **pressure4;
    double **pressure5;
    double **pressure6;
    double **pressure7;
    double **temperature1;
    double **temperature2;
    double **temperature3;
    double **temperature4;
    double **temperature5;
    double **temperature6;
    double **temperature7;
    double **QGPfraction1;
    double **QGPfraction2;
    double **QGPfraction3;
    double **QGPfraction4;
    double **QGPfraction5;
    double **QGPfraction6;
    double **QGPfraction7;
    double **entropyDensity1;
    double **entropyDensity2;
    double **entropyDensity3;
    double **entropyDensity4;
    double **entropyDensity5;
    double **entropyDensity6;
    double **entropyDensity7;
    double **mu1;
    double **mu2;
    double **cs2_1;
    double **cs2_2;
    double **cs2_3;
    double **cs2_4;
    double **cs2_5;
    double **cs2_6;
    double **cs2_7;

    double * eps_list_rho0, * s_list_rho0;
    int s_list_rho0_length;
    gsl_interp * interp_s2e;
    gsl_interp_accel * accel_s2e;

    int whichEOS;  //!< type of EoS
    
    double eps_max;

    Util *util;
  
 public:
    EOS(InitData *para_in);
    ~EOS();
    void initialize_eos();
    void init_eos0();               //!< initialize EoS for whichEOS=0
    void init_eos();                //!< initialize EoS for whichEOS=1
    void init_eos2();               //!< initialize EoS for whichEOS=2
    void init_eos3(int selector);   //!< initialize EoS for whichEOS=3 (PCE 150 MeV), 
                                    //!< initialize EoS whichEOS=4 (PCE 155 MeV), 
                                    //!< initialize EoS whichEOS=5 (PCE 160 MeV), 
                                    //!< initialize EoS whichEOS=6 (PCE 165 MeV)
    void init_eos7();               //!< initialize EoS for whichEOS=7 s95p-v1.2 (for UrQMD)
    void checkForReadError(FILE *file, const char* name);
    double interpolate_pressure(double e, double rhob);  // for whichEOS=1
    double interpolate(double e, double rhob, int selector);
    // for whichEOS=2
    double interpolate2(double e, double rhob, int selector); 
    
    double get_cs2(double e, double rhob);
    double calculate_velocity_of_sound_sq(double e, double rhob);
    void fill_cs2_matrix(double e0, double de, int ne, 
                         double rhob0, double drhob, int nrhob, 
                         double** cs2_ptr);
    void build_velocity_of_sound_sq_matrix();
    double get_dpOverde(double e, double rhob);
    double get_dpOverde2(double e, double rhob);
    double get_dpOverde3(double e, double rhob);
    double get_dpOverdrhob(double e, double rhob);
    double get_dpOverdrhob2(double e, double rhob);
    double p_rho_func(double e, double rhob);
    double p_e_func(double e, double rhob);
    double T_from_eps_ideal_gas(double eps);
    double get_entropy(double epsilon, double rhob);
    double get_temperature(double epsilon, double rhob);
    double get_mu(double epsilon, double rhob);
    double get_muS(double epsilon, double rhob);
    double get_qgp_frac(double epsilon, double rhob);
    double get_pressure(double epsilon, double rhob);
    double ssolve(double e, double rhob, double s);
    double Tsolve(double e, double rhob, double T);
    double findRoot(double (EOS::*function)(double, double, double), 
                   double rhob, double s, double e1, double e2, double eacc);
    double s2e_ideal_gas(double s);
    double get_s2e(double s, double rhob);
    
    //! Thie function returns maximum local energy density of the EoS table
    //! in the unit of [1/fm^4]
    double get_eps_max() {return(eps_max);}
};

#endif  // SRC_EOS_H_
  
