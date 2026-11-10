import os
import numpy as np
import librosa
import warnings
import pandas as pd

from calcula_features import calcula_features

from save_data import save_data

if __name__ == "__main__":
    avaliacao_dir = "../Results/"
    avaliacao_files = [f for f in os.listdir(avaliacao_dir) if f.startswith('avaliacao')]

    all_avaliacoes = None
    for avaliacao in avaliacao_files:
        if all_avaliacoes is None:
            all_avaliacoes = np.atleast_2d(np.genfromtxt(avaliacao_dir + avaliacao, delimiter=",").T)
        else:
            all_avaliacoes = np.vstack((all_avaliacoes, np.genfromtxt(avaliacao_dir + avaliacao, delimiter=",").T))

    print(all_avaliacoes)

    media_Musicas = all_avaliacoes.mean(axis=0)
    desvioPadrao_Musicas = all_avaliacoes.std(axis=0)

    print("Media Musicas:")
    print(media_Musicas)

    print("\nDesvio Padrao Musicas:")
    print(desvioPadrao_Musicas)

    all_avaliacoes = np.reshape(all_avaliacoes.T, (4, all_avaliacoes.shape[1] * all_avaliacoes.shape[0] // 4))

    media_Metricas = all_avaliacoes.mean(axis=1)
    desvioPadrao_Metricas = all_avaliacoes.std(axis=1)

    print("\nMedia Metricas:")
    print(media_Metricas)

    print("\nDesvio Padrao Metricas")
    print(desvioPadrao_Metricas)

    precision = np.array(all_avaliacoes > 2.5).sum(axis=1)
    print(precision)

    print()
    print("Precision Euclidian: ", precision[0]*100 / 44)
    print("Precision Manhattan: ", precision[1]*100 / 44)
    print("Precision Cosine: ", precision[2]*100 / 44)
    print("Precision Metadata: ", precision[3]*100 / 44)
