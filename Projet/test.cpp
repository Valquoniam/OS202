#include <iostream>
#include <cstdlib>
#include <ctime>
#include <mpi.h>

int main(int argc, char** argv) {
    // Initialisation de MPI
    MPI_Init(NULL, NULL);

    // Récupération de l'identifiant du processus et du nombre total de processus
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    std::cout << size <<std::endl;
    std::cout << rank <<std::endl;
    // Initialisation du générateur de nombres aléatoires
    srand(time(NULL) + rank);

    // Définition de la taille du tableau
    const int N = 1000000;

    // Calcul du nombre d'éléments assignés à chaque processus
    int n_local = N / size;

    // Allocation de mémoire pour le tableau local
    int* data_local = new int[n_local];

    // Remplissage du tableau local avec des nombres aléatoires
    for (int i = 0; i < n_local; i++) {
        data_local[i] = rand();
    }

    // Calcul de la somme partielle des éléments locaux
    long long sum_local = 0;
    for (int i = 0; i < n_local; i++) {
        sum_local += data_local[i];
    }

    // Calcul de la somme globale des éléments en utilisant MPI_Allreduce
    long long sum_global;
    MPI_Allreduce(&sum_local, &sum_global, 1, MPI_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);

    // Affichage du résultat
    if (rank == 0) {
        std::cout << "La somme globale est " << sum_global << std::endl;
    }

    // Libération de la mémoire allouée pour le tableau local
    delete[] data_local;

    // Finalisation de MPI
    MPI_Finalize();

    return 0;
}