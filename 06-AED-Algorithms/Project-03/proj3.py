import random
import time 



# Insertion sort
def insertionSort(arr):

    #print("Array inicial:")
    #printArray(arr)
         
    for i in range(1, len(arr)):
        key = arr[i]
        j = i - 1 #elemento anterior ao atual
        # Move os elementos do arr[0..i-1], que são maiores que key,para uma posição à frente de sua posição atual
        while j >= 0 and key < arr[j]:
            arr[j + 1] = arr[j]
            j -= 1
        arr[j + 1] = key
        
        
#Heap sort O(n log n)
def heapify(arr, n, i): #heap tree max
    maior = i  
    esquerda = 2 * i + 1
    direita = 2 * i + 2

    if esquerda < n and arr[esquerda] > arr[maior]:
        maior = esquerda

    if direita < n and arr[direita] > arr[maior]:
        maior = direita

    if maior != i: #Se algum dos filhos for maior que o pai
        arr[i], arr[maior] = arr[maior], arr[i]
        heapify(arr, n, maior) # recursivamente o heapify no novo índice onde foi colocado o pai para garantir max heap




def heapSort(arr):
    n = len(arr)
    # Construir max heap
    for i in range(n // 2 - 1, -1, -1): #começa do último nó pai e vai até a raiz
        heapify(arr, n, i) 

    # Extrair elementos do heap colocando o maior elemento no final do array e reduzindo o tamanho do heap
    for i in range(n - 1, 0, -1):
        arr[i], arr[0] = arr[0], arr[i]
        heapify(arr, i, 0) #garanrte que o heap é mantido após a troca
    




def quickSort(arr): #O(n log n) em média, O(n^2) no pior caso
    if len(arr) <= 1: # Se o array tiver 0 ou 1 elemento, já está ordenado
        return arr
    else:
        pivo = random.choice(arr)  # escolhe pivô aleatório
        menores = [x for x in arr if x < pivo] #elementos menores que o pivô
        iguais = [x for x in arr if x == pivo] #elementos iguais ao pivô
        maiores = [x for x in arr if x > pivo] #elementos maiores que o pivô
        return quickSort(menores) + iguais + quickSort(maiores) #Ordena recursivamente os subconjuntos.

def printArray(arr):
    for i in range(len(arr)):
        print(arr[i], end=" ")
    print()


def gerar_conjuntos(n=10000):
    unique = random.sample(range(1, n * 10), n)
    poucos_repetidos = [random.choice(unique) for _ in range(n)]
    return {
        'A': sorted(poucos_repetidos),
        'B': sorted(poucos_repetidos, reverse=True),
        'C': random.sample(poucos_repetidos, len(poucos_repetidos)),
    }
    

def imprimir_resultados(resultados):
    for tipo, dados in resultados.items():
        print(f"\nEstrutura: {tipo}")
        for conjunto, tempo in dados.items():
            print(f"  Conjunto {conjunto}: Tempo = {tempo:.4f}s")
    
def testar_insercao(conjuntos):
    resultados = {'Insertion': {},'Heap': {}, 'Quick': {}}
    for nome, dados in conjuntos.items():
        
        # Insertion Sort
        dados_insertion = dados.copy()
        print(f"Testando o conjunto {nome} com Insertion Sort...")  
        inicio = time.time()
        insertionSort(dados_insertion) 
        fim = time.time()
        resultados['Insertion'][nome] = (fim - inicio)

        # Heap Sort
        print(f"Testando o conjunto {nome} com Heap Sort...")
        dados_heap = dados.copy()
        inicio = time.time()
        heapSort(dados_heap)
        fim = time.time()
        resultados['Heap'][nome] = (fim - inicio)

        # Quick Sort
        print(f"Testando o conjunto {nome} com Quick Sort...")
        dados_quick = dados.copy()
        inicio = time.time()
        _ = quickSort(dados_quick)
        fim = time.time()
        resultados['Quick'][nome] = (fim - inicio)
    return resultados

    
if __name__ == "__main__":
    tamanhos = [100000, 200000, 500000, 1000000]
    for n in tamanhos:
        print(f"\nConjuntos com {n} elementos")
        conjuntos = gerar_conjuntos(n)
        resultados = testar_insercao(conjuntos)
        imprimir_resultados(resultados)