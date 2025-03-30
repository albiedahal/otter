import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy.integrate import trapz
import matplotlib.ticker as ticker

def interpolate_data(disp, force, interp_len):
    temp_disp1 = np.arange(1, len(disp) + 1)
    temp_disp2 = np.arange(1, len(disp) + 1, interp_len)
    new_disp = np.interp(temp_disp2, temp_disp1, disp)

    temp_force1 = np.arange(1, len(force) + 1)
    temp_force2 = np.arange(1, len(force) + 1, interp_len)
    new_force = np.interp(temp_force2, temp_force1, force)
    
    # print(new_disp)
    # print(disp)
    return new_disp, new_force


def compute_CycEn(force, disp):
    # Identify cycle indices where displacement crosses zero
    zero_crossings = np.where(np.diff(np.sign(disp)))[0]

    # Compute energy dissipation per cycle
    cycle_energies = []
    cycle_numbers = []
    cum_energy = []

    temp = 0
    for i in range(len(zero_crossings) - 1):
        start, end = zero_crossings[i], zero_crossings[i + 1]

        # Extract displacement and force for the cycle
        cycle_disp = disp[start:end+1]
        cycle_force = force[start:end+1]

        # print(cycle_disp)
        # print(cycle_force)

        # Compute energy dissipation (area enclosed in cycle)
        energy = np.abs(trapz(cycle_force, cycle_disp))
        temp += energy

        # Store results
        cycle_energies.append(energy)
        cycle_numbers.append(i + 1)
        cum_energy.append(temp)

    return cycle_numbers, cycle_energies, cum_energy

def compute_Error(exp, sim):
    x = np.asarray(exp)
    y = np.asarray(sim)

    rel_err = 100 * abs(x - y)/x     
    rmse = np.sqrt(np.mean((x - y) ** 2))/np.mean(np.abs(x))
    mae = np.mean(np.abs(x - y))/np.mean(np.abs(x))

    print(rmse)
    print(mae)

    return rel_err, rmse, mae

# Function to create a multi-axis plot
def plot_Energy(ax1, exp_cyc, exp_cu, sim_cyc, sim_cu, cum_err, rms):
    # First axis (left)
    ax1.plot(exp_cyc, exp_cu, 'ko-', label="Experiment")
    ax1.plot(sim_cyc, sim_cu, 'k*--', label="Simulation")
    ax1.set_xlabel("Cycle Number", fontsize=14)
    ax1.set_ylabel("Dissipated Energy (kN-m)", color='k', fontsize=14)
    ax1.tick_params(axis='both', colors='k', labelsize=12)
    ax1.set_ylim(bottom=0)
    ax1.legend(loc='upper center', fontsize=18)
    ax1.xaxis.set_major_locator(ticker.MaxNLocator(integer=True))
    ax1.text(0.3, 0.5, f'RMSE: {rms:.1%}', transform=ax1.transAxes, fontsize=16, bbox=dict(facecolor='white', alpha=0.7))

    # Second axis (right)
#    ax2 = ax1.twinx()
#    ax2.plot(exp_cyc, cum_err, 'b^:', label="Relative Error")
#    ax2.set_ylabel("Relative Error (%)", color='b', fontsize=14)
#    ax2.tick_params(axis='y', colors='b', labelsize=12)
#    ax2.set_ylim(bottom=0, top=100)
#    ax2.legend(loc='lower right', fontsize=12)


#def plot_Hys(ax1, disp_exp, force_exp, disp_sim, force_sim):
#    # First axis (left)
#    ax1.plot(disp_exp, force_exp, 'k-', label="Experiment")
#    ax1.plot(disp_sim, force_sim, 'b--', label="Simulation")
#    ax1.set_xlabel("Displacement (mm)", fontsize=14)
#    ax1.set_ylabel("Force (kN)", color='k', fontsize=14)
#    ax1.tick_params(axis='both', colors='k', labelsize=12)
#    ax1.legend(loc='upper left', fontsize=14)
#
#    ax1.xaxis.set_major_locator(ticker.MaxNLocator(integer=True))
#    ax1.spines['left'].set_position('zero')
#    ax1.spines['bottom'].set_position('zero')


# Load data
ex_D_c0, ex_F_c0 = np.loadtxt("./c0/experiment.csv", delimiter=",",skiprows=1, unpack=True)
sm_D_c0, sm_F_c0 = np.loadtxt("./c0/simulated.csv", delimiter=",", skiprows=1, unpack=True)
ex_D_c5, ex_F_c5 = np.loadtxt("./c5/experiment.csv", delimiter=",",skiprows=1, unpack=True)
sm_D_c5, sm_F_c5 = np.loadtxt("./c5/simulated.csv", delimiter=",", skiprows=1, unpack=True)


# int_disp, int_force = interpolate_data(exp_disp, exp_force, 0.1)
# int_force = np.interp(sim_disp, exp_disp, exp_force)

ex_cyl_c0, ex_en_c0, ex_cu_c0 = compute_CycEn(ex_F_c0, ex_D_c0/1000)
sm_cyl_c0, sm_en_c0, sm_cu_c0 = compute_CycEn(sm_F_c0, sm_D_c0/1000)
ex_cyl_c5, ex_en_c5, ex_cu_c5 = compute_CycEn(ex_F_c5, ex_D_c5/1000)
sm_cyl_c5, sm_en_c5, sm_cu_c5 = compute_CycEn(sm_F_c5, sm_D_c5/1000)

print(ex_cu_c0)
print(sm_cu_c0)

cy_err_c0, cy_rms_c0, cy_mae_c0 = compute_Error(ex_en_c0, sm_en_c0)
cu_err_c0, cu_rms_c0, cu_mae_c0 = compute_Error(ex_cu_c0, sm_cu_c0)
cy_err_c5, cy_rms_c5, cy_mae_c5 = compute_Error(ex_en_c5, sm_en_c5)
cu_err_c5, cu_rms_c5, cu_mae_c5 = compute_Error(ex_cu_c5, sm_cu_c5)




# Create subplots
fig, axes = plt.subplots(1, 2, figsize=(14, 6))


plot_Energy(axes[0], ex_cyl_c0, ex_cu_c0, sm_cyl_c0, sm_cu_c0, cu_err_c0, cu_rms_c0)
plot_Energy(axes[1], ex_cyl_c5, ex_cu_c5, sm_cyl_c5, sm_cu_c5, cu_err_c5, cu_rms_c5)

# Add subcaptions below each subplot
fig.text(0.25, 0.02, "(a)", ha='center', fontsize=20)
fig.text(0.75, 0.02, "(b)", ha='center', fontsize=20)
plt.tight_layout(rect=[0, 0.05, 1, 0.95])  # Prevent overlap with subcaptions
fig.savefig("Energy.pdf", dpi=600)
plt.show()

# Create subplots
fig, axes = plt.subplots(1, 2, figsize=(14, 6))


#plot_Hys(axes[0], ex_D_c0, ex_F_c0, sm_D_c0, sm_F_c0)
#plot_Hys(axes[1], ex_D_c5, ex_F_c5, sm_D_c5, sm_F_c5)
#
# # Add subcaptions below each subplot
#fig.text(0.25, 0.02, "(a)", ha='center', fontsize=20)
#fig.text(0.75, 0.02, "(b)", ha='center', fontsize=20)
#plt.tight_layout(rect=[0, 0.05, 1, 0.95])  # Prevent overlap with subcaptions
#fig.savefig("Hysteresis.png", dpi=600)
#plt.show()
#
#


