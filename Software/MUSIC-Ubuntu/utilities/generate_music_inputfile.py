#!/usr/bin/env python

import sys
from os import path
from music_parameters_dict import *

class color:
    """
    define colors in the terminal
    """
    purple = '\033[95m'
    cyan = '\033[96m'
    darkcyan = '\033[36m'
    blue = '\033[94m'
    green = '\033[92m'
    yellow = '\033[93m'
    red = '\033[91m'
    bold = '\033[1m'
    underline = '\033[4m'
    end = '\033[0m'

initial_condition_dict.update({
    'Initial_profile': 9,           # type of initial condition
    'initialize_with_entropy': 0,   # 0: with energy density
                                    # 1: with entropy density
    'Initial_Distribution_Filename': 'initial/new_u_field_0.dat',
    's_factor': 1.0,   # normalization factor read in initial data file

    #  envelope function in eta_s direction
    'Eta_plateau_size': 40.0,       # size of the plateau in eta_s direction
    'Eta_fall_off': 0.2,            # the scale of the fall off of the plateau 
                                    # in eta_s direction
})

hydro_dict.update({
    # grid information
    'Initial_time_tau_0': 0.4,   # starting time of the hydrodynamic evolution
    'Delta_Tau': 0.02,           # time step to use in the evolution [fm/c]

    'Eta_grid_size': 20.0,       # spatial rapidity range
    'Grid_size_in_eta': 4,       # number of the grid points in spatial rapidity
    'X_grid_size_in_fm': 34.0,   # spatial range along x direction
    'Y_grid_size_in_fm': 34.0,   # spatial range along y direction 
    'Grid_size_in_y': 256,       # number of the grid points in y direction
    'Grid_size_in_x': 256,       # number of the grid points in x direction
    
    'EOS_to_use': 2,         # type of the equation of state
    'reconst_type': 1,       # the type of quantity that will be first 
                             # reconstruct from T^0\mu and J^0
                             # 0: energy density; 1: u^0
    'boost_invariant': 1,    # initial condition is boost invariant

    #viscosity and diffusion options
    'Viscosity_Flag_Yes_1_No_0': 1,         # turn on viscosity in the evolution
    'Include_Shear_Visc_Yes_1_No_0': 1,     # include shear viscous effect
    'Shear_to_S_ratio': 0.08,               # value of \eta/s
    'T_dependent_Shear_to_S_ratio': 0,      # switch to turn on temperature 
                                            # dependent eta/s(T)
    'Include_Bulk_Visc_Yes_1_No_0': 1,      # include bulk viscous effect
    'Bulk_to_S_ratio': 0.1,                 # value of \zeta/s
    'Include_second_order_terms': 1,        # include second order coupling terms

    'output_hydro_debug_info': 0,           # flag to output additional evolution information for debuging
    'output_evolution_data': 0,             # flag to output evolution history to file
    'output_hydro_params_header' : 1,       # flag to output hydro evolution information header
    'outputBinaryEvolution': 1,             # flag to output evolution history in binary format
    'output_evolution_every_N_timesteps' : 1,     # number of points to skip in tau direction for hydro evolution
    'output_evolution_every_N_x' : 2,             # number of points to skip in x direction for hydro evolution
    'output_evolution_every_N_y' : 2,             # number of points to skip in y direction for hydro evolution
    'output_evolution_every_N_eta' : 1,           # number of points to skip in eta direction for hydro evolution
})

freeze_out_dict.update({
    'Do_FreezeOut_Yes_1_No_0': 1,   # flag to find freeze-out surface
    'freeze_out_method': 2,         # method for hyper-surface finder

    'average_surface_over_this_many_time_steps': 5,   # the step skipped in the tau direction

    'Include_deltaf': 0,        # flag to include delta f correction in Cooper-Frye formula
    'Inlucde_deltaf_bulk': 0,   # flag to include delta f for bulk viscosity

    'number_of_particles_to_include': 320,  # number of thermal particles to compute for particle spectra and vn
    'particle_spectrum_to_compute': 0,      # 0: Do all up to number_of_particles_to_include

    'pseudofreeze': 1,          # calculated particle spectra in equally-spaced pseudorapidity
    'max_pseudorapidity': 5.0,  # particle spectra calculated from (0, max_pseudorapidity)
    'pseudo_steps': 47,         # number of lattice points along pseudo-rapidity
    'phi_steps': 40,            # number of points calculated in phi for Cooper-Frye
    'min_pt': 0.01,             # the minimum value of pT calculated in the Cooper-Frye
    'max_pt': 3.0,              # the maximum value of pT calculated in the Cooper-Frye
    'pt_steps': 15,             # number of the pT points calculated in the Cooper-Frye
})

collect_dict.update({
    'dNdy_y_min': -0.5,
    'dNdy_y_max': 0.5,
    'dNdy_eta_min': -5.0,
    'dNdy_eta_max': 5.0,
    'dNdy_nrap': 51,
    'dNdyptdpt_y_min': -0.5,
    'dNdyptdpt_y_max': 0.5,
    'dNdyptdpt_eta_min': -0.5,
    'dNdyptdpt_eta_max': 0.5,
})


def generate_music_input_file(include_nodeltaf, include_y):
    print(color.purple + "\n" + "-"*80 
          + "\n>>>>> generating music input files! <<<<<\n" + "-"*80 
          + color.end)
    # mode 2:
    control_dict.update({'mode': 2})
    f = open('music_input_2', 'w')
    dict_list = [control_dict, initial_condition_dict, hydro_dict,
                 freeze_out_dict, collect_dict]
    for idict in range(len(dict_list)):
        temp_list = dict_list[idict].items()
        for i in range(len(temp_list)):
            f.write('%s  %s \n' % (temp_list[i][0], str(temp_list[i][1])))
    f.write('EndOfData\n')
    f.close()

    # mode 3:
    control_dict.update({'mode': 3})
    f = open('music_input_3', 'w')
    dict_list = [control_dict, initial_condition_dict, hydro_dict,
                 freeze_out_dict, collect_dict]
    for idict in range(len(dict_list)):
        temp_list = dict_list[idict].items()
        for i in range(len(temp_list)):
            f.write('%s  %s \n' % (temp_list[i][0], str(temp_list[i][1])))
    f.write('EndOfData\n')
    f.close()
    
    if include_nodeltaf == 1:
        freeze_out_dict.update({'Include_deltaf_qmu': 0})
        f = open('music_input_3_nodeltaf', 'w')
        dict_list = [control_dict, initial_condition_dict, hydro_dict,
                     freeze_out_dict, collect_dict]
        for idict in range(len(dict_list)):
            temp_list = dict_list[idict].items()
            for i in range(len(temp_list)):
                f.write('%s  %s \n' % (temp_list[i][0], str(temp_list[i][1])))
        f.write('EndOfData\n')
        f.close()
        freeze_out_dict.update({'Include_deltaf_qmu': 1})
    
    if include_y == 1:
        freeze_out_dict.update({'pseudofreeze': 0})
        f = open('music_input_3_y', 'w')
        dict_list = [control_dict, initial_condition_dict, hydro_dict,
                     freeze_out_dict, collect_dict]
        for idict in range(len(dict_list)):
            temp_list = dict_list[idict].items()
            for i in range(len(temp_list)):
                f.write('%s  %s \n' % (temp_list[i][0], str(temp_list[i][1])))
        f.write('EndOfData\n')
        f.close()
        if include_nodeltaf == 1:
            freeze_out_dict.update({'Include_deltaf_qmu': 0})
            f = open('music_input_3_y_nodeltaf', 'w')
            dict_list = [control_dict, initial_condition_dict, hydro_dict,
                         freeze_out_dict, collect_dict]
            for idict in range(len(dict_list)):
                temp_list = dict_list[idict].items()
                for i in range(len(temp_list)):
                    f.write('%s  %s \n' 
                            % (temp_list[i][0], str(temp_list[i][1])))
            f.write('EndOfData\n')
            f.close()
            freeze_out_dict.update({'Include_deltaf_qmu': 1})
        freeze_out_dict.update({'pseudofreeze': 1})

    # mode 4:
    control_dict.update({'mode': 4})
    f = open('music_input_4', 'w')
    dict_list = [control_dict, initial_condition_dict, hydro_dict,
                 freeze_out_dict, collect_dict]
    for idict in range(len(dict_list)):
        temp_list = dict_list[idict].items()
        for i in range(len(temp_list)):
            f.write('%s  %s \n' % (temp_list[i][0], str(temp_list[i][1])))
    f.write('EndOfData\n')
    f.close()
    if include_y == 1:
        freeze_out_dict.update({'pseudofreeze': 0})
        f = open('music_input_4_y', 'w')
        dict_list = [control_dict, initial_condition_dict, hydro_dict,
                     freeze_out_dict, collect_dict]
        for idict in range(len(dict_list)):
            temp_list = dict_list[idict].items()
            for i in range(len(temp_list)):
                f.write('%s  %s \n' % (temp_list[i][0], str(temp_list[i][1])))
        f.write('EndOfData\n')
        f.close()
        freeze_out_dict.update({'pseudofreeze': 1})

    # mode 13:
    control_dict.update({'mode': 13})
    f = open('music_input_13', 'w')
    dict_list = [control_dict, initial_condition_dict, hydro_dict,
                 freeze_out_dict, collect_dict]
    for idict in range(len(dict_list)):
        temp_list = dict_list[idict].items()
        for i in range(len(temp_list)):
            f.write('%s  %s \n' % (temp_list[i][0], str(temp_list[i][1])))
    f.write('EndOfData\n')
    f.close()
    if include_y == 1:
        freeze_out_dict.update({'pseudofreeze': 0})
        collect_dict.update({
            'dNdy_y_min': -5.0,
            'dNdy_y_max': 5.0,
            'dNdy_eta_min': -0.5,
            'dNdy_eta_max': 0.5,
        })
        f = open('music_input_13_y', 'w')
        dict_list = [control_dict, initial_condition_dict, hydro_dict,
                     freeze_out_dict, collect_dict]
        for idict in range(len(dict_list)):
            temp_list = dict_list[idict].items()
            for i in range(len(temp_list)):
                f.write('%s  %s \n' % (temp_list[i][0], str(temp_list[i][1])))
        f.write('EndOfData\n')
        f.close()
        freeze_out_dict.update({'pseudofreeze': 1})
        collect_dict.update({
            'dNdy_y_min': -0.5,
            'dNdy_y_max': 0.5,
            'dNdy_eta_min': -5.0,
            'dNdy_eta_max': 5.0,
        })

    # mode 14:
    control_dict.update({'mode': 14})
    f = open('music_input_14', 'w')
    dict_list = [control_dict, initial_condition_dict, hydro_dict,
                 freeze_out_dict, collect_dict]
    for idict in range(len(dict_list)):
        temp_list = dict_list[idict].items()
        for i in range(len(temp_list)):
            f.write('%s  %s \n' % (temp_list[i][0], str(temp_list[i][1])))
    f.write('EndOfData\n')
    f.close()
    if include_y == 1:
        freeze_out_dict.update({'pseudofreeze': 0})
        collect_dict.update({
            'dNdy_y_min': -5.0,
            'dNdy_y_max': 5.0,
            'dNdy_eta_min': -0.5,
            'dNdy_eta_max': 0.5,
        })
        f = open('music_input_14_y', 'w')
        dict_list = [control_dict, initial_condition_dict, hydro_dict,
                     freeze_out_dict, collect_dict]
        for idict in range(len(dict_list)):
            temp_list = dict_list[idict].items()
            for i in range(len(temp_list)):
                f.write('%s  %s \n' % (temp_list[i][0], str(temp_list[i][1])))
        f.write('EndOfData\n')
        f.close()
        freeze_out_dict.update({'pseudofreeze': 1})
        collect_dict.update({
            'dNdy_y_min': -0.5,
            'dNdy_y_max': 0.5,
            'dNdy_eta_min': -5.0,
            'dNdy_eta_max': 5.0,
        })


def generate_submit_script(include_nodeltaf, include_y):
    print(color.purple + "\n" + "-"*80 
          + "\n>>>>> generating submission script! <<<<<\n" + "-"*80 
          + color.end)
    decoupling_energy_density = ['0.1']
    queue_name = 'sw'
    ppn = 16
    walltime = '12:00:00'
    working_folder = path.abspath('./') 
    hydro_results_folder = 'results'
    spectra_results_folder = 'particle_spectra'
    folder_name = working_folder.split('/')[-1]
    script = open("submit_full_job.pbs", "w")
    script.write(
"""#!/usr/bin/env bash
#PBS -N %s
#PBS -l walltime=%s
#PBS -l nodes=1:ppn=%d
#PBS -S /bin/bash
#PBS -e test.err
#PBS -o test.log
#PBS -A cqn-654-ad
#PBS -q %s
#PBS -d %s

module add ifort_icc/14.0.4

results_folder=%s
spectra_folder=%s

# hydro evolution
mpirun -np %d ./mpihydro music_input_2 1>mode_2.log 2>mode_2.err
./sweeper.sh $results_folder
""" % (folder_name, walltime, ppn, queue_name, working_folder, 
       hydro_results_folder, spectra_results_folder, ppn))

    # multiple Cooper-Frye in sequence
    script.write("# multiple Cooper-Frye in sequence ... \n")
    for isurf in range(len(decoupling_energy_density)):
        thermal_folder_name = 'spvn_eps_%s' % decoupling_energy_density[isurf]
        script.write(
"""cp results/surface_eps_%s.dat ./surface.dat
mpirun -np %d ./mpihydro music_input_3 1>mode_3.log 2>mode_3.err
rm -fr yptphiSpectra?.dat yptphiSpectra??.dat
thermal_folder=%s
mkdir $thermal_folder
mv particleInformation.dat $thermal_folder
mv yptphiSpectra.dat $thermal_folder
cp mpihydro $thermal_folder
cp music_input_4 $thermal_folder
cp music_input_13 $thermal_folder
cp music_input_14 $thermal_folder
cp known_nuclei.dat $thermal_folder
cp -r EOS $thermal_folder
./generate_resonance_decay_job.py $thermal_folder
cd $thermal_folder
qsub -A cqn-654-ad submit_resonance_job.pbs
cd ..
""" % (decoupling_energy_density[isurf], ppn, thermal_folder_name))

        if include_nodeltaf == 1:
            script.write(
"""mpirun -np %d ./mpihydro music_input_3_nodeltaf 1>mode_3_nodeltaf.log 2>mode_3_nodeltaf.err
rm -fr yptphiSpectra?.dat yptphiSpectra??.dat
thermal_folder=spvn_nodeltaf_eps_%s
mkdir $thermal_folder
mv particleInformation.dat $thermal_folder
mv yptphiSpectra.dat $thermal_folder
cp mpihydro $thermal_folder
cp music_input_4 $thermal_folder
cp music_input_13 $thermal_folder
cp music_input_14 $thermal_folder
cp known_nuclei.dat $thermal_folder
cp -r EOS $thermal_folder
./generate_resonance_decay_job.py $thermal_folder
cd $thermal_folder
qsub -A cqn-654-ad submit_resonance_job.pbs
cd ..
""" % (ppn, decoupling_energy_density[isurf]))

        if include_y == 1:
            script.write(
"""mpirun -np %d ./mpihydro music_input_3_y 1>mode_3_y.log 2>mode_3_y.err
rm -fr yptphiSpectra?.dat yptphiSpectra??.dat
thermal_folder=spvn_y_eps_%s
mkdir $thermal_folder
mv particleInformation.dat $thermal_folder
mv yptphiSpectra.dat $thermal_folder
cp mpihydro $thermal_folder
cp music_input_4_y $thermal_folder
cp music_input_13_y $thermal_folder
cp music_input_14_y $thermal_folder
cp known_nuclei.dat $thermal_folder
cp -r EOS $thermal_folder
./generate_resonance_decay_job.py $thermal_folder
cd $thermal_folder
qsub -A cqn-654-ad submit_resonance_job.pbs
cd ..
""" % (ppn, decoupling_energy_density[isurf]))

            if include_nodeltaf == 1:
                script.write(
"""mpirun -np %d ./mpihydro music_input_3_y_nodeltaf 1>mode_3_y_nodeltaf.log 2>mode_3_y_nodeltaf.err
rm -fr yptphiSpectra?.dat yptphiSpectra??.dat
thermal_folder=spvn_y_nodeltaf_eps_%s
mkdir $thermal_folder
mv particleInformation.dat $thermal_folder
mv yptphiSpectra.dat $thermal_folder
cp mpihydro $thermal_folder
cp music_input_4_y $thermal_folder
cp music_input_13_y $thermal_folder
cp music_input_14_y $thermal_folder
cp known_nuclei.dat $thermal_folder
cp -r EOS $thermal_folder
./generate_resonance_decay_job.py $thermal_folder
cd $thermal_folder
qsub -A cqn-654-ad submit_resonance_job.pbs
cd ..
""" % (ppn, decoupling_energy_density[isurf]))

    script.close()


def print_help_message():
    print "Usage : "
    print(color.bold
          + "./generate_music_inputfile.py"
          + "[-cen centrality -shear_vis eta_over_s -EOS eos_type]"
          + color.end)
    print "Usage of runHydro.py command line arguments: "
    print(color.bold + "-shear_vis" + color.end
          + "   the specific shear viscosity used in the hydro simulation\n"
          + color.bold + "       eta/s = 0.08 [default]" + color.end)
    print(color.bold + "-EOS" + color.end
          + "   the equation of state for hydrodynamic simulation \n"
          + color.purple + color.bold + "       10 [default]"
          + color.end
          + color.purple + ", 2-5" + color.end)
    print(color.bold + "-cen" + color.end
          + "  specify the centrality bin: "
          + color.bold + "0-5 [default]" + color.end
          + color.purple + ', e.g. 20-30' + color.end)
    print(color.bold + "-evo" + color.end
          + "   switch to output the hydro evolution history file (0 or 1)\n"
          + color.bold + "       evo = 0 [default]" + color.end)
    print(color.bold + "-tau0" + color.end
          + "   the starting time for hydrodynamic evolution [fm/c]\n"
          + color.bold + "       tau_0 = 0.6 [default]" + color.end)
    print(color.bold + "-h | -help" + color.end + "    This message")


def read_in_parameters_from_command_line():
    while len(sys.argv) > 1:
        option = sys.argv[1]
        del sys.argv[1]
        if option == '-cen':            # centrality 
            centrality = str(sys.argv[1])
            del sys.argv[1]
            initial_condition_dict.update({
                'Initial_Distribution_Filename':
                    'initial/sdAvg_order_2_C%s.dat' % centrality,
            })
        elif option == '-iev':            # event by event mode
            iev = int(sys.argv[1])
            del sys.argv[1]
            initial_condition_dict.update({
                'Initial_Distribution_Filename':
                    'epsilon-u-Hydro%d.dat' % iev,
            })
        elif option == '-shear_vis':    # shear viscosity
            vis = float(sys.argv[1])
            del sys.argv[1]
            hydro_dict.update({'Shear_to_S_ratio': vis,})
        elif option == '-EOS':          # EOS
            eos_type = int(sys.argv[1])
            del sys.argv[1]
            hydro_dict.update({'EOS_to_use': eos_type,})
        elif option == '-evo':          # output hydro evolution file
            evo_type = int(sys.argv[1])
            del sys.argv[1]
            hydro_dict.update({'output_evolution_data': evo_type,})
        elif option == '-tau0':         # hydro start time
            tau0 = float(sys.argv[1])
            del sys.argv[1]
            hydro_dict.update({'Initial_time_tau_0': tau0,})
        elif option == '-h':
            print_help_message()
            sys.exit(0)
        else:
            print sys.argv[0], ': invalid option', option
            print_help_message()
            sys.exit(1)
    


if __name__ == "__main__":
    read_in_parameters_from_command_line()

    generate_music_input_file(0, 0)
    generate_submit_script(0, 0)
