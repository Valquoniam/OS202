import matplotlib.pyplot as plt
import numpy as np

x = np.arange(1,12,1)
y = [4.76,3.339,2.25,2.1216,1.5,1.46,1.4,1.28,1.055,1.02,1.00]
z = []

plt.clf()

#plt.subplot(1,2,1)
#plt.plot(x,y)
#plt.ylabel("Temps de calcul (s)")
#plt.xlabel("Nombre de threads utilisés")
#plt.title("Temps de calcul en fonction du nombre de threads")

#plt.subplot(1,2,2)
#plt.plot(x,z)
#plt.ylabel("Puissance de calcul (Mflops)")
#plt.xlabel("Nombre de threads utilisés")
#plt.title("Puissance en fonction du nombre de threads")


plt.plot(np.arange(1,129,2),[3.31385, 1.18312, 0.963017, 0.918346, 0.842595, 0.918002, 0.935019, 0.976521, 0.877099, 0.868672, 0.87294, 0.872644, 0.859782, 0.912962, 0.901139, 0.908001, 0.879056, 0.872044, 0.859164, 0.855331, 0.803744, 0.823627, 0.82763, 0.836014, 0.847502, 0.843538, 0.846849, 0.825325, 0.824365, 0.822342, 0.83646, 0.838782, 0.811513, 0.807709, 0.811685, 0.79768, 0.786404, 0.795546, 0.79695, 0.802135, 0.774376, 0.758522, 0.7404, 0.736093, 0.721973, 0.743659, 0.74456, 0.73968, 0.75437, 0.737196, 0.701631, 0.724081, 0.727487, 0.735023, 0.754511, 0.760816, 0.754642, 0.727869, 0.709124, 0.714074, 0.71174, 0.70999, 0.714519, 0.722582])
plt.ylabel("Taille des sous-blocs")
plt.xlabel("Temps de calcul (s)")
plt.title("Temps de calcul d'une matrice de dimension 1024 selon différentes tailles de sous-blocs")
plt.show()