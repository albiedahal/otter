import pandas as pd
import numpy as np
import matplotlib.pyplot as plt


df = pd.read_csv('cyclic_out.csv')
fig, axs = plt.subplots(1,2, figsize=(9,3))
s1 = df.iloc[0:301]
s2 = df.iloc[302:1501]
s3 = df.iloc[1502:2401]
s4 = df.iloc[2402:3601]
s5 = df.iloc[3602:4401]

majy1 = np.arange(-1.2e-4, 1.2e-4, 0.00006)
majy2 = np.arange(-250, 250, 50)
majx1 = np.arange(0,45,5)
majx2 = np.arange(-0.00012, 0.00012, 0.00006)

# axs[0].plot(s1['time'],s1['load'], color='k', linestyle='dashed')
# axs[0].plot(s2['time'],s2['load'], color='k', linestyle='dashed')
# axs[0].plot(s3['time'],s3['load'], color='k', linestyle='dashed')
# axs[0].plot(s4['time'],s4['load'], color='k', linestyle='dashed')
# axs[0].plot(s5['time'],s5['load'], color='k')
# axs[0].plot(s4['time'],s4['load'], color='k')
# axs[0].plot(s3['time'],s3['load'], color='k')
# axs[0].plot(s2['time'],s2['load'], color='k')
axs[0].plot(s1['time'],s1['load'], color='k')

axs[0].set_ylim(-1.2e-4, 1.2e-4)
axs[0].set_ylabel('Displacement (m)')
axs[0].set_xlabel('Load Step')
axs[0].set_xlim(0,45)
axs[0].set_xticks(majx1)
axs[0].set_yticks(majy1)
axs[0].ticklabel_format(axis='y', style='scientific')
axs[0].grid(True)
# axs[0].spines['left'].set_position('zero')
# axs[0].spines['right'].set_color('none')
# axs[0].yaxis.tick_left()
# axs[0].spines['bottom'].set_position('zero')
# axs[0].spines['top'].set_color('none')
# axs[0].xaxis.tick_bottom()
# axs[1].plot(s1['strain_xx'], s1['stress_xx'], color='k', linestyle='dashed')
# axs[1].plot(s2['strain_xx'], s2['stress_xx'], color='k', linestyle='dashed')
# axs[1].plot(s3['strain_xx'], s3['stress_xx'], color='k', linestyle='dashed')
# axs[1].plot(s4['strain_xx'], s4['stress_xx'], color='k', linestyle='dashed')
# axs[1].plot(s5['strain_xx'], s5['stress_xx'], color='k')
# axs[1].plot(s4['strain_xx'], s4['stress_xx'], color='k')
# axs[1].plot(s3['strain_xx'], s3['stress_xx'], color='k')
# axs[1].plot(s2['strain_xx'], s2['stress_xx'], color='k')
axs[1].plot(s1['strain_xx'], s1['stress_xx'], color='k')


axs[1].set_xlabel('Strain (mm/mm)')
axs[1].set_xlim(-0.00012, 0.00012)
axs[1].set_ylabel('Stress (MPa)')
axs[1].set_ylim(-250, 250)
axs[1].grid(True, 'both')
axs[1].set_xticks(majx2)
axs[1].set_yticks(majy2)

plt.tight_layout()
plt.show()
