import numpy as np
import time
import matplotlib.pyplot as plt
import sys
from mpi4py import MPI

# Initialisation MPI
comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

# On va faire une 1ère parallélisation en buckets telle que :
#   - Tous les processus calculent une partie des résultats
#   - Et s'occupent d'afficher leur partie des résultats 
#   - Le processus 0 renvoie le temps maximum de calcul et d'affichage des résultats

nombre_cas   : int = 256
nb_cellules  : int = 360  # Cellules fantomes
nb_iterations: int = 360

compute_time = 0.
display_time = 0.

# Programmes de sauvegarde
def save_as_md(cells, symbols='⬜⬛'):
    res = np.empty(shape=cells.shape, dtype='<U')
    res[cells==0] = symbols[0]
    res[cells==1] = symbols[1]
    np.savetxt(f'results_md/resultat_{num_config:03d}.md', res, fmt='%s', delimiter='', header=f'Config #{num_config}', encoding='utf-8')

def save_as_png(cells):
    fig = plt.figure(figsize=(nb_iterations/10., nb_cellules/10.))
    ax = plt.axes()
    ax.set_axis_off()
    ax.imshow(cells[:, 1:-1], interpolation='none', cmap='RdPu')
    plt.savefig(f"results_png/resultat_{num_config:03d}.png", dpi=100, bbox_inches='tight')
    plt.close()

# On divise équitablement le nombre de cas traités par processus
cas_traites_par_proc = nombre_cas // size
premier_cas = rank*cas_traites_par_proc
dernier_cas = premier_cas + cas_traites_par_proc

for num_config in range(premier_cas,dernier_cas):
    t1 = time.time()
    
    cells = np.zeros((nb_iterations, nb_cellules+2), dtype=np.int16)
    cells[0, (nb_cellules+2)//2] = 1
    for iter in range(1, nb_iterations):
        vals = np.left_shift(1, 4*cells[iter-1, 0:-2]
                             + 2*cells[iter-1, 1:-1]
                             + cells[iter-1, 2:])
        cells[iter, 1:-1] = np.logical_and(np.bitwise_and(vals, num_config), 1)

    t2 = time.time()
    compute_time += t2 - t1

    t1 = time.time()
    
    save_as_md(cells)      # Pour des soucis de temps, nous ferons uniquement les tests avec une sauvegarde en .md
    #save_as_png(cells)
    t2 = time.time()
    display_time += t2 - t1

comm.Barrier()
compute_time_max = comm.reduce(compute_time,op=MPI.MAX,root=0) # Calcul du maximum des temps de calcul (tous à peu près égaux)
display_time_max = comm.reduce(display_time,op=MPI.MAX,root=0) # Calcul du max des temps d'affichage

if rank == 0:
    print(f"Temps de calcul des générations de cellules : {compute_time_max:.6g} s.")
    print(f"Temps d'affichage des résultats : {display_time_max:.6g} s.")

MPI.Finalize()