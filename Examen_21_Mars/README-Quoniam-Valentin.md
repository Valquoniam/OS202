# Examen machine du 21 Mars 2023 - Valentin QUONIAM-BARRE

## Informations de ma machine

Mon ordinateur possède (commande `lscpu` ) :

- **4** coeurs physiques
- **8** coeurs logiques
- **5 MiB** de cache L2 par coeur physique
- **8 MiB** de cache L3

## Automate cellulaire 1D

#### Tous les temps de calcul seront basés sur le calcul des 256 possibilités et leur affichage en .md

### (1)
Sans parallélisation, voici les temps de calcul et d'affichage du code :

```bash
Temps calcul des générations de cellules : 0.750677 s.
Temps d'affichage des résultats : 15.74 s.
```

J'ai appliqué une première parallélisation en **bucket statique**. Chaque processus se voit attribuer un bucket de tâches à réaliser, et les buckets sont préalablement initialisés. J'ai choisi cette stratégie car on remarque que les temps d'affichage sont bien plus grands que ceux de calcul. On ne peut donc pas se permettre d'avoir un processus dédié au calcul et les autres à l'affichage (comme dans une stratégie maître-esclave par exemple).

Il faut donc que tous les processus s'occupent de l'affichage. Par ailleurs, il n'y a pas de nécessité de distribuer les tâches selon la disponibilité des processus, donc aucune raison de faire un bucket dynamique.

On obtient, pour 4 coeurs utilisés (commande `mpiexec -n 4 python3 automate_cellulaire_1d.py`):

```bash
Temps de calcul des générations de cellules : 0.403844 s.
Temps d'affichage des résultats : 7.03159 s.
```

On a un donc speedup de **2,2** sur le temps d'affichage en passant de 1 à 4 coeurs.

Voici un tableau qui résume les résultats obtenus :

| **Nombre de coeurs**| 1 | 2 | 3 | 4 |
| ------------------- | --| --| --| --|
| **Temps de calcul** (s)| 0.757 | 0.47 | 0.44 | 0.31|
| **Temps d'affichage** (s)| 14.99 | 6.89 | 5.99 | 4.9 |

### (2)

On ajoute à la fin de notre code python un plot de graph pour un nombre de coeurs allant de 0 à 4. Voici le résultat :

![Temps de calcul et d'affichage en fonction du nombre de processus](images/plots_automate.png.png){width=50%}{height =50%}

## Calcul d'une enveloppe convexe

