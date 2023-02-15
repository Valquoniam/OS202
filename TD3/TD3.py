
def main():
    arr = 
    bucket_sort(arr)



def bucket_sort(arr):
    
    # Initialisation de bucket : La liste des buckets (chaque bucket est vide initialement)
    n = len(arr)
    bucket = [[] for k in range(n)] # On a n buckets
    
    # Remplissage des différents buckets
    for i in range(n):
        index = int(n * arr[i] / (max(arr) + 1))
        bucket[index].append(arr[i])
        
    #Tri dans chaque bucket
    for i in range(n):
        bucket[i].sort()
        
    #On remet proprement le résultats du tri de chaque bucket
    result = []
    for i in range(n):
        result += bucket[i]
    return result

if __name__ == '__main__':
    main()