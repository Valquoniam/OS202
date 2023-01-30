## Exercice 2.1

nbp = 10  #valeur arbitraire à fixer

def circulation_jeton(jeton,n) :
    if n == 0 :
        if jeton == 0 :
            jeton = 1
            circulation_jeton(jeton,n+1)
        else : 
            print(jeton)
            return 0    
    elif n == nbp :
        jeton+=1
        circulation_jeton(jeton,0)
    else :
        jeton+=1
        circulation_jeton(jeton,n+1)
    return 1


## Exercice 2.2
import numpy as np
import numpy.random as rd

def calc_pi_s() :  #version séquentielle
    n = 10000 #valeur arbitraire du nombre de points générés dans le carré
    L = []  #liste qui va contenir nos n points
    q = 0   #nombre de points dans le carré qui sont aussi dans le cercle unité

    for i in range(n) :
        x = (-1)**rd.randint(1,2) * rd.rand()
        y = (-1)**rd.randint(1,2) * rd.rand()
        L.append([x,y])

        if np.sqrt(x**2 + y**2) <= 1 :
            q+=1
    r = q/n
    #print(4*r)
    return 4*r


#### version avec parallélisation MPI
import time

def random_coordonnee() : 
    return (-1)**rd.randint(1,2) * rd.rand()
def ajoute_liste(elem,L) :
    L.append(elem)
    return L
def est_dans_cercle(X,q):
    if np.sqrt(X[0]**2 + X[1]**2) <= 1 :
        q+=1
    return q

def calc_pi_p() :  #version parallèle
    n = 10000 #valeur arbitraire du nombre de points générés dans le carré
    L = []  #liste qui va contenir nos n points
    q = 0   #nombre de points dans le carré qui sont aussi dans le cercle unité

    for i in range(n) :
        x = random_coordonnee()
        y = random_coordonnee()
        ajoute_liste([x,y],L)
        q = est_dans_cercle([x,y],q)

    r = q/n
    #print(4*r)
    return 4*r

##Mesure du temps :
S = 0
P = 0
for i in range(1000):
    deb_s = time.perf_counter()
    calc_pi_s()
    fin_s = time.perf_counter()
    S+=(fin_s-deb_s)/1000

    deb_p=time.perf_counter()
    calc_pi_p()
    fin_p = time.perf_counter()
    P+=(fin_p - deb_p)/100

print(S,P)



