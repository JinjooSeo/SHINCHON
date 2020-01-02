#########################################
# parameters list for MUSIC with comments
#########################################
control_dict = {
    'mode': 2,  # MUSIC running mode
                # 1: Does everything. Evolution. Computation of thermal spectra.
                #    Resonance decays. Observables.
                #    Only compatible with freeze_out_method=3 and pseudofreeze=1
                # 2: Evolution only.
                # 3: Compute all thermal spectra only.
                # 4: Resonance decays only.
                # 5: Resonance decays for just one specific particle
                #    (only for testing
                #     - this will miss the complete chain of decays).
                # 6: only combine charged hadrons - can be used for any
                #    postprocessing with the stored results
                # 13: Compute observables from thermal spectra
                # 14: Compute observables from post-decay spectra
    'echo_level' : 1,   # switch to control the mount of warning message output
                        # chosen from 1 to 9
}


###################################
# parameters for initial conditions
###################################
initial_condition_dict = {
    'Initial_profile': 8,   # type of initial condition
                             # 0: Sangyong simple profile
                             # 1: Optical Glauber model
                             # 2: Test scenario for the freeze-out surface
                             #    finder
                             # 3: Event-by-event Glauber MC
                             # 4: for testing the Glauber MC initial condition
                             # 5: Something like p+p
                             # 6,7,8: Read in initial profile from a file

    'initialize_with_entropy': 0,   # 0: with energy density
                                    # 1: with entropy density

    # read in initial conditions from external file
    'Initial_Distribution_Filename': 'initial/initial_ed.dat',
    's_factor': 1.0,        # normalization factor read in initial data file

    # parameters for Glauber model
    'binary_collision_scaling_fraction': 0.,      # for wounded nucleon/binary collision mixing ratio in the Glauber model
    'Maximum_energy_density': 54.,                # maximum energy density for Glauber initial condition
    'SigmaNN': 42.1,                              # NN inelastic cross section for Glauber model
    'Impact_parameter': 3.,                       # impact parameter for optical Glauber model
    'bmin': 9.,                                   # minimum sampling range for impact parameter in MC-Glauber model (fm)
    'bmax': 9.,                                   # maximum sampling range for impact parameter in MC-Glauber model (fm)
    'sigma_0': 0.4,                               # Gaussian width for individual nucleon (fm)
    'Target': 'Au',                               # type of target nucleus
    'Projectile': 'Au',                           # type of projectile nucleus
    'Lexus_Imax': 100,                            # ???

    # envelope function in eta_s direction
    'Eta_plateau_size': 20.,                      # size of the plateau in eta_s direction
    'Eta_fall_off': 0.7,                          # the scale of the fall off of the plateau in eta_s direction
}


#######################################
# parameters for hydrodynamic evolution
#######################################
hydro_dict = {
    # grid information
    'Initial_time_tau_0': 0.6,          # starting time of the hydrodynamic evolution (fm/c)
    'Total_evolution_time_tau': 30.,    # the maximum allowed running evolution time (fm/c)
                                        # need to be set to some large enough number
    'Delta_Tau': 0.04,                  # time step to use in the evolution [fm/c]
    'Eta_grid_size': 14.0,              # spatial rapidity range
                                        # [-Eta_grid_size/2, Eta_grid_size/2 - delta_eta]
    'Grid_size_in_eta': 4,              # number of the grid points in spatial rapidity direction
                                        # have at least 4 cells per processor.
                                        # Must be an even number.
                                        # One cell is positioned at eta=0,
                                        # half the cells are at negative eta,
                                        # the rest (one fewer) are at positive eta
    'X_grid_size_in_fm': 26.0,          # spatial range along x direction in the transverse plane
                                        # [-X_grid_size_in_fm/2, X_grid_size_in_fm/2]
    'Y_grid_size_in_fm': 26.0,          # spatial range along y direction in the transverse plane
                                        # [-Y_grid_size_in_fm/2, Y_grid_size_in_fm/2]
    'Grid_size_in_y': 260,              # number of the grid points in y direction
    'Grid_size_in_x': 260,              # number of the grid points in x direction


    'EOS_to_use': 2,  # type of the equation of state
                      # 0: ideal gas
                      # 1: EOS-Q from azhydro
                      # 2: lattice EOS s95p-v1 from Huovinen and Petreczky
                      # 3: lattice EOS s95p with partial chemical equilibrium (PCE) at 150 MeV
                      #    (see https://wiki.bnl.gov/TECHQM/index.php/QCD_Equation_of_State)
                      # 4: lattice EOS s95p with chemical freeze out at 155 MeV
                      # 5: lattice EOS s95p at 160 MeV
                      # 6: lattice EOS s95p at 165 MeV
                      # 7: lattice EOS s95p-v1.2 (for UrQMD)
    'Minmod_Theta': 1.8,     # theta parameter in the min-mod like limiter
    'Runge_Kutta_order': 2,  # order of Runge_Kutta for temporal evolution (must be 1 or 2)
    'reconst_type': 1,       # the type of quantity that will be first reconstruct from T^0\mu and J^0
                             # 0: energy density and rho_baryon
                             # 1: flow velocity

    #viscosity and diffusion options
    'Viscosity_Flag_Yes_1_No_0': 1,               # turn on viscosity in the evolution
    'Include_Shear_Visc_Yes_1_No_0': 1,           # include shear viscous effect
    'Shear_to_S_ratio': 0.08,                     # value of \eta/s
    'T_dependent_Shear_to_S_ratio': 0,            # switch to turn on temperature dependent eta/s(T)
    'Include_Bulk_Visc_Yes_1_No_0': 0,            # include bulk viscous effect
    'Bulk_to_S_ratio': 0.1,                       # value of \zeta/s
    'Include_second_order_terms': 0,              # include second order coupling terms

    # quest revert parameters to stablize the hydro
    'QuestRevert_rho_shear_max': 0.1,             # quest revert for shear
    'QuestRevert_rho_bulk_max': 0.1,              # quest revert for bulk

    'Maximum_Local_Rapidity': 20.0,               # the maximum allowed flow rapidity in reconst.cpp

    'output_hydro_debug_info': 1,                 # flag to output additional evolution information for debuging
    'output_evolution_data': 0,                   # flag to output evolution history to file
    'output_hydro_params_header' : 1,             # flag to output hydro evolution information header
    'outputBinaryEvolution': 1,                   # flag to output evolution history in binary format
    'output_evolution_every_N_timesteps' : 1,     # number of points to skip in tau direction for hydro evolution
    'output_evolution_every_N_x' : 1,             # number of points to skip in x direction for hydro evolution
    'output_evolution_every_N_y' : 1,             # number of points to skip in y direction for hydro evolution
    'output_evolution_every_N_eta' : 1,           # number of points to skip in eta direction for hydro evolution
}

###########################################
# parameters for freeze out and Cooper-Frye
###########################################
freeze_out_dict = {
    'Do_FreezeOut_Yes_1_No_0': 1,         # flag to find freeze-out surface
    'freeze_out_method': 2,               # method for hyper-surface finder
                                          # 2: Schenke's more complex method
    'average_surface_over_this_many_time_steps': 5,   # the step skipped in the tau direction
    'epsilon_freeze': 0.18,                       # the freeze out energy density (GeV/fm^3)
    'use_eps_for_freeze_out': 1,                  # flag to use energy density as criteria to find freeze-out surface 0: use temperature, 1: use energy density
    'T_freeze': 0.135,                            # freeze-out temperature (GeV)

    'number_of_particles_to_include': 320,        # number of thermal particles to compute for particle spectra and vn
                                          # current maximum = 320
    'particle_spectrum_to_compute': 0,            # 0: Do all up to number_of_particles_to_include
                                          # any natural number: Do the particle with this (internal) ID
    'pseudofreeze': 1,                            # calculated particle spectra in equally-spaced pseudorapidity
    'max_pseudorapidity': 2.5,                    # particle spectra calculated from (0, max_pseudorapidity)
    'pseudo_steps': 11,                            # number of lattice points along pseudo-rapidity
    'phi_steps': 40,                              # number of points calculated in phi for Cooper-Frye
    'min_pt': 0.01,                                # the minimum value of pT calculated in the Cooper-Frye
    'max_pt': 3.0,                                # the maximum value of pT calculated in the Cooper-Frye
    'pt_steps': 15,                               # number of the pT points calculated in the Cooper-Frye

    'Include_deltaf': 1,                          # flag to include delta f correction in Cooper-Frye formula
    'Inlucde_deltaf_bulk': 0,                     # flag to include delta f for bulk viscosity
}

###########################################################
# parameters for mode 14 to collect particle spectra and vn
###########################################################
collect_dict = {
    'dNdy_y_min': -0.01,                           # the minimum rapidity
    'dNdy_y_max': 0.01,
    'dNdy_eta_min': -2.5,                         # the minimum pseudorapidity
    'dNdy_eta_max': 2.5,
    'dNdy_nrap': 30,
    'dNdyptdpt_y_min': -0.01,
    'dNdyptdpt_y_max': 0.01,
    'dNdyptdpt_eta_min': -0.5,
    'dNdyptdpt_eta_max': 0.5,
}

