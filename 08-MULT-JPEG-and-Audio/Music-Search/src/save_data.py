import numpy as np


# Exercício 2.1.4. (criar e gravar em ficheiro o array com as features extraídas)
def save_data(data, path: str, format='%.6f'):
    np.savetxt(path, data, fmt=format, delimiter=',')
