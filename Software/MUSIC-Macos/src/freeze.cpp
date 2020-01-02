// MUSIC - a 3+1D viscous relativistic hydrodynamic code for heavy ion collisions
// Copyright (C) 2017  Gabriel Denicol, Charles Gale, Sangyong Jeon, Matthew Luzum, Jean-François Paquet, Björn Schenke, Chun Shen

#include "./freeze.h"

using namespace std;

Freeze::Freeze(InitData* DATA_in) {
    if (etasize < DATA_in->pseudo_steps + 1) {
        music_message << __func__
            << "etasize is smaller than pseudo_steps + 1! "
            << "Please increase the value of etasize >= pseudo_steps + 1 "
            << "in freeze.h";
        music_message.flush("error");
        exit(1);
    }
    util = new Util;

    DATA_ptr = DATA_in;
    // for final particle spectra and flow analysis, define the list
    // of charged hadrons that have a long enough lifetime to reach
    // the detectors
    int temp_list [] = {211, -211, 321, -321, 2212, -2212, 3222, -3222,
                        3112, -3112, 3312, -3312};
    charged_hadron_list_length = sizeof(temp_list)/sizeof(int);
    charged_hadron_list = new int [charged_hadron_list_length];
    for(int i = 0; i < charged_hadron_list_length; i++)
        charged_hadron_list[i] = temp_list[i];

    particleMax = DATA_ptr->NumberOfParticlesToInclude;
    music_message << "number of particle species to compute = " << particleMax;
    music_message.flush("info");
    bulk_deltaf_kind = 1;
    boost_invariant = DATA_ptr->boost_invariant;
    if (boost_invariant) {
        // define the gauss integration points along eta_s
        // the weights already take care of the factor of 2
        // from -infty to +infty
        double temp_gauss[] = {
            2.4014964e-02, 1.2545322e-01, 3.0358683e-01,
            5.5116454e-01, 8.5805566e-01, 1.2116973e+00,
            1.5976118e+00, 2.0000000e+00, 2.4023882e+00,
            2.7883027e+00, 3.1419444e+00, 3.4488355e+00,
            3.6964132e+00, 3.8745468e+00, 3.9759850e+00
        };
        double temp_gauss_weight[] = {
            1.2301297e-01, 2.8146419e-01, 4.2863688e-01,
            5.5828271e-01, 6.6507682e-01, 7.4464400e-01,
            7.9372594e-01, 8.1031297e-01, 7.9372594e-01,
            7.4464400e-01, 6.6507682e-01, 5.5828271e-01,
            4.2863688e-01, 2.8146419e-01, 1.2301297e-01,
        };
        n_eta_s_integral = static_cast<int>(sizeof(temp_gauss)/sizeof(double));
        eta_s_inte_array = new double[n_eta_s_integral];
        eta_s_inte_weight = new double[n_eta_s_integral];
        cosh_eta_s_inte = new double[n_eta_s_integral];
        sinh_eta_s_inte = new double[n_eta_s_integral];
        for (int i = 0; i < n_eta_s_integral; i++) {
            eta_s_inte_array[i] = temp_gauss[i];
            eta_s_inte_weight[i] = temp_gauss_weight[i];
            cosh_eta_s_inte[i] = cosh(eta_s_inte_array[i]);
            sinh_eta_s_inte[i] = sinh(eta_s_inte_array[i]);
        }
    }
    phiArray = NULL;
}

// destructors
Freeze::~Freeze() {
    delete util;
    delete[] charged_hadron_list;
    if (phiArray != NULL) {
        delete[] phiArray;
    }

    if (boost_invariant) {
        delete[] eta_s_inte_array;
        delete[] eta_s_inte_weight;
        delete[] sinh_eta_s_inte;
        delete[] cosh_eta_s_inte;
    }
}

void Freeze::checkForReadError(FILE *file, const char* name) {
    if (!(file)) {
        music_message << "file " << name << " not found. Exiting...";
        music_message.flush("error");
        exit(0);
    }
}

void Freeze::read_particle_PCE_mu(InitData* DATA, EOS *eos) {
    double ef;
    if (DATA->useEpsFO == 1) {
        ef = DATA->epsilonFreeze;
    } else {
        music_message << "determining epsFO from TFO=" << DATA->TFO;
        music_message.flush("info");
        ef = eos->findRoot(&EOS::Tsolve, 0., DATA->TFO/hbarc,
                           0.001, 300.,0.001)*hbarc;
        music_message << "freeze out energy density is " << ef;
        music_message.flush("info");
    }

    music_message << "Determining chemical potentials at freeze out "
                  << "energy density " << ef << " GeV/fm^3.";
    music_message.flush("info");
    
    // get environment path
    const char* EOSPATH = "HYDROPROGRAMPATH";
    char * pre_envPath= getenv(EOSPATH);
    std::string envPath;
    if (pre_envPath == 0) {
	    envPath = ".";
    } else {
	    envPath = pre_envPath;
    }

    string mu_name;
    if (DATA->whichEOS == 3) {
        mu_name = envPath + "/EOS/s95p-PCE-v1/s95p-PCE-v1_pichem1.dat";
    } else if (DATA->whichEOS == 4) {
        mu_name = envPath + "/EOS/s95p-PCE155/pichem1.dat";
    } else if (DATA->whichEOS == 5) {
        mu_name = envPath + "/EOS/s95p-PCE160/pichem1.dat";
    } else if (DATA->whichEOS == 6) {
        mu_name = envPath + "/EOS/s95p-PCE165-v0/s95p-PCE165-v0_pichem1.dat";
    }
    music_message << "Reading chemical potentials from file " << mu_name; 
    music_message.flush("info");
    
    ifstream mu_file(mu_name.c_str());
    if (!mu_file.is_open()) {
        music_message << "file " << mu_name << "not found. Exiting...";
        music_message.flush("error");
        exit(0);
    }
    double EPP1;            // start value for \mu_B and epsilon
    double deltaEPP1;       // step size for \mu_B and epsilon
    int NEPP1;              // number of entries for \mu_B and epsilon
    int numStable = 0;      // number of stable particles
    double **chemPot;

    mu_file >> EPP1 >> deltaEPP1 >> NEPP1;
    mu_file >> numStable;
      
    // chemical potential for every stable particle 
    chemPot = util->mtx_malloc(numStable+1, NEPP1+1);

    for (int j = NEPP1-1; j >= 0; j--) {
        for (int i = 0; i < numStable; i++) {
            mu_file >> chemPot[i][j];
        }
    }
    mu_file.close();
    
    double frace;
    int ie1, ie2;
    if (ef < EPP1) {
        ie1 = 0;
        ie2 = 1;
        frace = ef/(EPP1);
    } else {
        ie1 = floor((ef-EPP1)/deltaEPP1);
        ie2 = floor((ef-EPP1)/deltaEPP1+1);
        frace = (ef-(ie1*deltaEPP1+EPP1))/deltaEPP1; 
    }
      
    if (ie1 > NEPP1) {
        music_message.error("ERROR in ReadParticleData. out of range.");
        music_message << "ie1=" << ie1 << ",NEPP1=" << NEPP1;
        music_message.flush("error");
        exit(0);
    }
    
    if (ie2 > NEPP1) {
        music_message.error("ERROR in ReadParticleData. out of range.");
        music_message << "ie2=" << ie2 << ",NEPP1=" << NEPP1;
        music_message.flush("error");
        exit(0);
    }

    double pa, pb;
    double mu[numStable+1];
    music_message << "num_of_stable_particles = " << numStable;
    music_message.flush("info");
    
    for(int i = 1; i <= numStable; i++) {
        pa = chemPot[i-1][ie1];
        pb = chemPot[i-1][ie2];
      
        if (ef < EPP1) {
            mu[i] = pa*(frace);
        } else {
            mu[i] = pa*(1-frace) + pb*frace;
        }
    }
     
    for (int i = 0; i < DATA->NumberOfParticlesToInclude; i++) {
        particleList[i].muAtFreezeOut = 0.;
    }
      
    if (DATA->NumberOfParticlesToInclude >= 8) {
        for(int i = 1; i <= 8; i++) {
            particleList[i].muAtFreezeOut = mu[i];
        }
    } else {
        music_message << "Need at least 8 particles. "
             << "Increase number of particles to include. Exiting.";
        music_message.flush("error");
        exit(1);
    }
    
    if (DATA->whichEOS != 6) {
        if (DATA->NumberOfParticlesToInclude >= 12)
            particleList[12].muAtFreezeOut = mu[9];
        if (DATA->NumberOfParticlesToInclude >= 17)
            particleList[17].muAtFreezeOut = mu[10];
        if (DATA->NumberOfParticlesToInclude>=18)
            particleList[18].muAtFreezeOut = mu[11];
        if (DATA->NumberOfParticlesToInclude>=19)
            particleList[19].muAtFreezeOut = mu[12];
        if (DATA->NumberOfParticlesToInclude>=20)
            particleList[20].muAtFreezeOut = mu[13];
        if (DATA->NumberOfParticlesToInclude>=21)
            particleList[21].muAtFreezeOut = mu[14];

        if (DATA->NumberOfParticlesToInclude>=26)
            particleList[26].muAtFreezeOut = mu[15];
        if (DATA->NumberOfParticlesToInclude>=27)
            particleList[27].muAtFreezeOut = mu[16];
        if (DATA->NumberOfParticlesToInclude>=28)
            particleList[28].muAtFreezeOut = mu[17];

        if (DATA->NumberOfParticlesToInclude>=30)
            particleList[30].muAtFreezeOut = mu[18];
        if (DATA->NumberOfParticlesToInclude>=31)
            particleList[31].muAtFreezeOut = mu[19];
        if (DATA->NumberOfParticlesToInclude>=32)
            particleList[32].muAtFreezeOut = mu[20];
        if (DATA->NumberOfParticlesToInclude>=33)
            particleList[33].muAtFreezeOut = mu[21];
        if (DATA->NumberOfParticlesToInclude>=34)
            particleList[34].muAtFreezeOut = mu[22];
        if (DATA->NumberOfParticlesToInclude>=35)
            particleList[35].muAtFreezeOut = mu[23];

        if (DATA->NumberOfParticlesToInclude>=60)
            particleList[60].muAtFreezeOut = mu[24];
        if (DATA->NumberOfParticlesToInclude>=61)
            particleList[61].muAtFreezeOut = mu[25];
        if (DATA->NumberOfParticlesToInclude>=62)
            particleList[62].muAtFreezeOut = mu[26];
        if (DATA->NumberOfParticlesToInclude>=63)
            particleList[63].muAtFreezeOut = mu[27];

        if (DATA->NumberOfParticlesToInclude>=110)
            particleList[110].muAtFreezeOut = mu[28];
      
        if (DATA->NumberOfParticlesToInclude>=111)
            particleList[111].muAtFreezeOut = mu[29];

        if (DATA->NumberOfParticlesToInclude>=117)
            particleList[117].muAtFreezeOut = mu[30];
        if (DATA->NumberOfParticlesToInclude>=118)
            particleList[118].muAtFreezeOut = mu[31];
        if (DATA->NumberOfParticlesToInclude>=119)
            particleList[119].muAtFreezeOut = mu[32];
        if (DATA->NumberOfParticlesToInclude>=120)
            particleList[120].muAtFreezeOut = mu[33];

        if (DATA->NumberOfParticlesToInclude>=170)
            particleList[170].muAtFreezeOut = mu[34];
        if (DATA->NumberOfParticlesToInclude>=171)
            particleList[171].muAtFreezeOut = mu[35];
    } else {   
        if (DATA->NumberOfParticlesToInclude>=17)
            particleList[17].muAtFreezeOut = mu[9];
        if (DATA->NumberOfParticlesToInclude>=18)
            particleList[18].muAtFreezeOut = mu[10];
        if (DATA->NumberOfParticlesToInclude>=19)
            particleList[19].muAtFreezeOut = mu[11];
        if (DATA->NumberOfParticlesToInclude>=20)
            particleList[20].muAtFreezeOut = mu[12];
        if (DATA->NumberOfParticlesToInclude>=21)
            particleList[21].muAtFreezeOut = mu[13];
        if (DATA->NumberOfParticlesToInclude>=26)
            particleList[26].muAtFreezeOut = mu[14];
        if (DATA->NumberOfParticlesToInclude>=27)
            particleList[27].muAtFreezeOut = mu[15];
        if (DATA->NumberOfParticlesToInclude>=28)
            particleList[28].muAtFreezeOut = mu[16];

        if (DATA->NumberOfParticlesToInclude>=30)
            particleList[30].muAtFreezeOut = mu[17];
        if (DATA->NumberOfParticlesToInclude>=31)
            particleList[31].muAtFreezeOut = mu[18];
        if (DATA->NumberOfParticlesToInclude>=32)
            particleList[32].muAtFreezeOut = mu[19];
        if (DATA->NumberOfParticlesToInclude>=33)
            particleList[33].muAtFreezeOut = mu[20];
        if (DATA->NumberOfParticlesToInclude>=34)
            particleList[34].muAtFreezeOut = mu[21];
        if (DATA->NumberOfParticlesToInclude>=35)
            particleList[35].muAtFreezeOut = mu[22];
        if (DATA->NumberOfParticlesToInclude>=60)
            particleList[60].muAtFreezeOut = mu[23];
        if (DATA->NumberOfParticlesToInclude>=61)
            particleList[61].muAtFreezeOut = mu[24];
        if (DATA->NumberOfParticlesToInclude>=62)
            particleList[62].muAtFreezeOut = mu[25];
        if (DATA->NumberOfParticlesToInclude>=63)
            particleList[63].muAtFreezeOut = mu[26];
        if (DATA->NumberOfParticlesToInclude>=170)
            particleList[170].muAtFreezeOut = mu[27];
        if (DATA->NumberOfParticlesToInclude>=171)
            particleList[171].muAtFreezeOut = mu[28];
    }
    music_message << "Got the chemical potentials at freeze out for "
                  << "stable particles.";
    music_message.flush("info");

    int k = 0;
    for(int i = 1; i < DATA->NumberOfParticlesToInclude; i++) {
        // skip the photon (i=0)
        if (particleList[i].muAtFreezeOut == 0) {
            if (particleList[i].baryon == -1) {
                k -= particleList[i].decays;
            }
            for (int j = 1; j <= particleList[i].decays; j++) {
                k++;
                for (int m=0; m<abs(decay[k].numpart); m++) {
                    if (particleList[i].baryon == -1
                        && particleList[partid[MHALF+decay[k].part[m]]].baryon != 0) {
                        particleList[i].muAtFreezeOut += (
                            decay[k].branch
                            *particleList[partid[MHALF-decay[k].part[m]]].muAtFreezeOut);
                    } else {
                        particleList[i].muAtFreezeOut += (
                            decay[k].branch
                            *particleList[partid[MHALF+decay[k].part[m]]].muAtFreezeOut);
                    }
                }
            }
        } else {
            for (int j = 1; j <= particleList[i].decays; j++) {
                if (particleList[i].baryon > -1)
                    k++;
            }
        }
    }
    music_message << "Got the chemical potentials at freeze-out for the resonances.";
    music_message.flush("info");

    // clean up
    util->mtx_free(chemPot, numStable+1, NEPP1+1);
}


void Freeze::ReadParticleData(InitData *DATA, EOS *eos) {
    // read in particle and decay information from file:
    partid = new int[MAXINTV]; 
    music_message.info("reading particle data");
    // open particle data file:
    const char* EOSPATH = "HYDROPROGRAMPATH";
    char * pre_envPath= getenv(EOSPATH);
    std::string envPath;

    if (pre_envPath == 0) {
	    envPath=".";
    } else {
	    envPath=pre_envPath;
    }
    string p_name = envPath + "/EOS/pdg05.dat";
    if (DATA->whichEOS == 7) {
        p_name = envPath + "/EOS/pdg-urqmd_v3.3+.dat";
    }

    music_message << "read in particle species table from " << p_name;
    music_message.flush("info");

    FILE *p_file;
    p_file = fopen(p_name.c_str(), "r");
    checkForReadError(p_file, p_name.c_str());

    for (int k = 0; k < MAXINTV; k++) 
        partid[k] = -1; 

    if (DATA->echo_level > 5) {
        music_message << "size_of_Particle= " << sizeof(Particle)/1024/1024
                      << " MB";
        music_message.flush("info");
    }
    particleList = (Particle *)malloc(
                    (DATA->NumberOfParticlesToInclude + 2)*sizeof(Particle));
    if (DATA->echo_level > 5) {
        music_message << "after first (check if there is enough memory... "
                      << "seg fault may be due to lack of memory)";
        music_message.flush("warning");
    }
    int i = 0;
    int j = 0;
    int status = 0;
    // read particle data:
    while (i < DATA->NumberOfParticlesToInclude) {
        //particleList[i].name = util->char_malloc(50);
        status = fscanf(p_file, "%d", &particleList[i].number);
        status = fscanf(p_file, "%s", particleList[i].name);
        status = fscanf(p_file, "%lf", &particleList[i].mass);
        status = fscanf(p_file, "%lf", &particleList[i].width);
        status = fscanf(p_file, "%d", &particleList[i].degeneracy);
        status = fscanf(p_file, "%d", &particleList[i].baryon);
        status = fscanf(p_file, "%d", &particleList[i].strange);
        status = fscanf(p_file, "%d", &particleList[i].charm);
        status = fscanf(p_file, "%d", &particleList[i].bottom);
        status = fscanf(p_file, "%d", &particleList[i].isospin);
        status = fscanf(p_file, "%lf", &particleList[i].charge);
        status = fscanf(p_file, "%d", &particleList[i].decays);   // number of decays
        if (status == 0 && DATA->echo_level > 9) {
            music_message << "fscanf status = " << status;
            music_message.flush("warning");
        }

        partid[MHALF + particleList[i].number] = i;
        particleList[i].stable = 0;

        int h;
        for(int k = 0; k < particleList[i].decays; k++) {
            h = fscanf(p_file, "%i%i%lf%i%i%i%i%i",
                       &decay[j].reso, &decay[j].numpart, &decay[j].branch, 
                       &decay[j].part[0], &decay[j].part[1], &decay[j].part[2],
                       &decay[j].part[3], &decay[j].part[4]);
            if (h != 8) {
                printf("Error in scanf decay \n");
                exit(0);
            }
            if (decay[j].numpart == 1) {
                // "decays" into one particle, i.e. is stable 
                particleList[i].stable = 1;
            }
            j++;   // increase the decay counting variable "j" by 1
        }
    
        // include anti-baryons (there are none in the file)
        if (particleList[i].baryon != 0) {
            i++;
            //particleList[i].name = util->char_malloc(50);
            particleList[i].width = particleList[i-1].width;
            particleList[i].charm = -particleList[i-1].charm;
            particleList[i].bottom = -particleList[i-1].bottom;
            particleList[i].isospin = particleList[i-1].isospin;
            particleList[i].charge = -particleList[i-1].charge;
            particleList[i].decays = particleList[i-1].decays;
            particleList[i].stable = particleList[i-1].stable;
            particleList[i].number = -particleList[i-1].number;
            strcpy(particleList[i].name, "Anti-");
            strcat(particleList[i].name,particleList[i-1].name);
            particleList[i].mass = particleList[i-1].mass;
            particleList[i].degeneracy = particleList[i-1].degeneracy;
            particleList[i].baryon = -particleList[i-1].baryon;
            particleList[i].strange = -particleList[i-1].strange;
            particleList[i].charge = -particleList[i-1].charge;
            partid[MHALF + particleList[i].number] = i;
        }
        i++;
    }
    decayMax = j;
    fclose(p_file);
    music_message << "Read in particle spicies " << i;
    music_message.flush("info");
    DATA->NumberOfParticlesToInclude = i;

    // here read the stable particles' chemical potential at freeze-out
    if (DATA->whichEOS > 2 && DATA->whichEOS < 6) {
        read_particle_PCE_mu(DATA, eos);
    }
      
    music_message.info("Done reading particle data.");
}


int Freeze::countLines(std::istream& in) {
    return std::count(std::istreambuf_iterator<char>(in),
                      std::istreambuf_iterator<char>(),
                      '\n');
}

void Freeze::ReadFreezeOutSurface(InitData *DATA) {
    music_message.info("reading freeze-out surface");

    FILE *s_file;
    const char* s_name = "./surface.dat";
    s_file = fopen(s_name, "r");
    checkForReadError(s_file, s_name);

    NCells = 0;
    // new counting, mac compatible ...
    ifstream in;
    in.open(s_name);
    NCells = 0;
    NCells += countLines(in);
    music_message << "NCells = " << NCells;
    music_message.flush("info");
    fclose(s_file);

    s_file = fopen(s_name, "r");
    // Now allocate memory: array of surfaceElements with length NCells
    surface = (SurfaceElement *) malloc((NCells)*sizeof(SurfaceElement));
    int i = 0;
    int status = 0;
    while (i < NCells) {
        // position in (tau, x, y, eta)
        status = fscanf(s_file, "%lf", &surface[i].x[0]);
        status = fscanf(s_file, "%lf", &surface[i].x[1]);
        status = fscanf(s_file, "%lf", &surface[i].x[2]);
        status = fscanf(s_file, "%lf", &surface[i].x[3]);
        surface[i].sinh_eta_s = sinh(surface[i].x[3]);
        surface[i].cosh_eta_s = cosh(surface[i].x[3]);
        // hypersurface vector in (tau, x, y, eta)
        status = fscanf(s_file, "%lf", &surface[i].s[0]);
        status = fscanf(s_file, "%lf", &surface[i].s[1]);
        status = fscanf(s_file, "%lf", &surface[i].s[2]);
        status = fscanf(s_file, "%lf", &surface[i].s[3]);
        // flow velocity in (tau, x, y, eta)
        status = fscanf(s_file, "%lf", &surface[i].u[0]);
        status = fscanf(s_file, "%lf", &surface[i].u[1]);
        status = fscanf(s_file, "%lf", &surface[i].u[2]);
        status = fscanf(s_file, "%lf", &surface[i].u[3]);
        // freeze-out energy density
        status = fscanf(s_file, "%lf", &surface[i].epsilon_f);
        if (surface[i].epsilon_f < 0)  {
            music_message.error("Error: epsilon-f<0.");
            exit(1);
        }
        // freeze-out temperature
        status = fscanf(s_file, "%lf", &surface[i].T_f);
        if (surface[i].T_f < 0) {
            music_message.error("WARNING: T_f<0.");
            exit(1);
        }
        // freeze-out baryon chemical potential
        status = fscanf(s_file, "%lf", &surface[i].mu_B);
        // freeze-out entropy density s
        status = fscanf(s_file, "%lf", &surface[i].eps_plus_p_over_T_FO);
        // freeze-out Wmunu
        status = fscanf(s_file, "%lf", &surface[i].W[0][0]);
        status = fscanf(s_file, "%lf", &surface[i].W[0][1]);
        status = fscanf(s_file, "%lf", &surface[i].W[0][2]);
        status = fscanf(s_file, "%lf", &surface[i].W[0][3]);
        status = fscanf(s_file, "%lf", &surface[i].W[1][1]);
        status = fscanf(s_file, "%lf", &surface[i].W[1][2]);
        status = fscanf(s_file, "%lf", &surface[i].W[1][3]);
        status = fscanf(s_file, "%lf", &surface[i].W[2][2]);
        status = fscanf(s_file, "%lf", &surface[i].W[2][3]);
        status = fscanf(s_file, "%lf", &surface[i].W[3][3]);
        if (DATA->turn_on_bulk) {
            status = fscanf(s_file, "%lf", &surface[i].pi_b);
        }
        i++;
        if (status == 0 && DATA->echo_level > 9) {
            music_message << "fscanf status = " << status;
            music_message.flush("warning");
        }
    }
    fclose(s_file);
}

