import numpy as np
# Importation de la bibliothèque MPI
from mpi4py import MPI

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

# Dimension du problème (peut-être changé)
dim = 120

#Taille des blocs de colonne
Ncols = dim//size              # La taille d'un bloc de colonnes
start = rank*Ncols             # La borne de départ de chaque processus
end = (rank+1)*Ncols           # La borne de fin de chaque processus

# Initialisation de la matrice
A = np.array([ [(i+j)%dim+1 for i in range(dim)] for j in range(dim) ])


# Initialisation du vecteur u
u = np.array([i+1 for i in range(dim)])

A_local = np.array(A[:,start:end]) #Sous-bloc de colonnes de A
u_local = np.array(u[start:end])   #Sous bloc de u pour avoir les bonnes dimensions 

v_local = A_local.dot(u_local)      #Calcul de chaque sous bloc
v = comm.allreduce(v_local)         #On additionne toutes les vecteurs obtenus et on envoie à tous les processus le résultat
print(f"Valeur du résultat pour le processus {rank} : \n{v}\n")
    

