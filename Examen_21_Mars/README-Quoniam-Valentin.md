# Examen machine du 21 Mars 2023 - Valentin QUONIAM-BARRE

## Informations de ma machine

Mon ordinateur possède (commande `lscpu` ) :

- **4** coeurs physiques
- **8** coeurs logiques
- **5 MiB** de cache L2 par coeur physique
- **8** MiB de cache L3

## Automate cellulaire 1D

J'ai appliqué une première parallélisation en **bucket statique**. Chaque processus se voit attribuer un bucket de tâches à réaliser, et les buckets sont préalablement initialisés. C'est une première stratégie simple qui se révèle efficace. 

On obtient, pour 4 coeurs utilisés (commande `mpiexec -n 4 python3 automate_cellulaire_1d.py`):

```bash
Temps de calcul des générations de cellules : 0.403844 s.
Temps d'affichage des résultats : 7.03159 s.
```

Néanmoins, le temps d'affichage des résultats est largement supérieur à celui de calcul. On pourrait, pour

