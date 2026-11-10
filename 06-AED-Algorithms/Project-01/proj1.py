#Projeto 1 
#Diogo da Silva Henriques 2019222685
import time
import random 
def exaustivo(array):
    n=len(array)
    min, max = array[0], array[0]
    missing = None
    sum_arr = 0
    for num in array: #O(n)
        if num < min:
            min = num
        if num > max:
            max = num
        sum_arr += num
        
        
    full_array = list(range(min, max+1, 1)) #complete array O(n)
    
    
    for num in full_array: #O(n^2)
        found=False
        for n in array:
            if num == n:
                found=True
                
        if not found:
            missing = num
            break
           
    
    return missing 

def ordered_solution(array):
    sorted_array = sorted(array) #O(nlog(n))
    n = len(sorted_array)
    max = sorted_array[n-1] #O(1)
    min = sorted_array[0] #O(1)
    full_array = list(range(min, max+1, 1)) #O(n)
    
    
    total_complete_array=0
    total_real_array = 0
    for number in array: #O(n)
        total_real_array += number
    
    for number in full_array: #O(n)
        total_complete_array += number
        
    missing = total_complete_array-total_real_array    
    
    return missing #total O(n*log(n))


def sum_of_integers(min, max):
    if min >= max: return 0
    n= max-min+1
    total = (n* (max+min) )//2
    return total
    
def optimized_solution(array):
    
    max_val = max(array) #O(n)
    min_val = min(array) #O(n)
    total_complete_array = sum_of_integers(min_val,max_val) #O(1)
    total_real_array = sum(array) #O(n)
    missing = total_complete_array - total_real_array #O(1)
    
    return missing

def exec_time(function,array):
    start_time = time.time()
    result = function(array)
    end_time = time.time()
    execution_time = end_time - start_time
    return result, execution_time

def generate_random_sequence(size):
    test_array = list(range(1, size + 2))  # Criamos uma sequência completa
    test_array.remove(random.randint(1, size + 1))  # Removemos um número aleatório
    random.shuffle(test_array) #baralhar array de teste 
    
    return test_array

def main():  
    array_sizes= [100000, 200000,50000,10000,300000]
    repeat=5
    
    
    for size in array_sizes:
        total_time_exaustivo=0
        total_time_optimized=0
        total_time_ordered=0
        exaustivo_times=[]
        optimized_times=[]
        ordered_times=[]
        print("Array Size:",size)
        for i in range(repeat):          
            input_array = generate_random_sequence(size)
            missing_exaustivo,time_exaustivo = exec_time(exaustivo,input_array)
            missing_ordered,time_ordered = exec_time(ordered_solution,input_array)
            missing_optimized,time_optimized = exec_time(optimized_solution,input_array)   
            print ("(Solução Exaustiva) Elemento em falta:", missing_exaustivo,"||Tempo de execução:",time_exaustivo)
            print ("(Solução Ordenada) Elemento em falta:", missing_ordered,"||Tempo de execução:",time_ordered)
            print ("(Solução Otimizada)Elemento em falta:", missing_optimized,"||Tempo de execução:",time_optimized)
            exaustivo_times.append(time_exaustivo)
            optimized_times.append(time_optimized)
            ordered_times.append(time_ordered)
        for time in exaustivo_times:
            total_time_exaustivo+= time
        for time in optimized_times:
            total_time_optimized+= time
        for time in ordered_times:
            total_time_ordered+= time
        media_exaustivo=total_time_exaustivo/repeat
        media_ordered=total_time_ordered/repeat
        media_optimized=total_time_optimized/repeat
        print("Após",repeat, "execuções, Média de tempo de execução (Exaustivo):",media_exaustivo,"segundos para tamanho:",size)
        print("Após",repeat, "execuções, Média de tempo de execução (Ordenada):",media_ordered,"segundos para tamanho:",size)
        print("Após",repeat, "execuções, Média de tempo de execução (Otimizada):",media_optimized,"segundos para tamanho:",size)




if __name__ == '__main__':
    main()