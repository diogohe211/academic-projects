import os
import numpy as np
from scipy.spatial import distance
import librosa
import warnings
import pandas as pd
import sounddevice as sd

from save_data import save_data

if __name__ == "__main__":
    print("Nome: ")
    nome = input()
    musicas = np.array(pd.read_csv("../Results/escolhas.txt", header=None).values).flatten()
    
    fName_query = "../Queries/MT0000414517.mp3"
    sr = 22050
    mono = True
    warnings.filterwarnings("ignore")
    y_query, fs_query = librosa.load(fName_query, sr=sr, mono=mono)

    sons = []
    for musica in musicas:
        y, fs = librosa.load("../MER_audio_taffc_dataset/audio/" + musica, sr=sr, mono=mono)
        sons.append((y, sr))

    tam = len(musicas)
    i = 0
    avaliacao = np.zeros(tam)
    while True:
        print("Musica atual: ", i)
        print("Selecione opcao (\"help\" para ajuda): ", end=" ")
        op = input()
        if op == "help":
            print("\thelp -> ajuda")
            print("\tt -> tocar musica atual")
            print("\tq -> tocar musica da query")
            print("\ta -> Avaliar musica atual")
            print("\tp -> ir para a musica anterior")
            print("\tn -> ir para a musica seguinte")
            print("\tg -> guardar avaliacao (quando estiver terminada)")
            print("\tm -> mostrar as avaliacoes (0 -> não avaliado)")
            print("\t[numero] -> ir para a musica numero [numero]")
        elif op == "t":
            sd.play(sons[i][0], sons[i][1], blocking=False)
        elif op == "q":
            sd.play(y_query, fs_query, blocking=False)
        elif op == "a":
            while True:
                print("Avaliacao atual: ", int(avaliacao[i]))
                print("Selecione valor (1 a 5, -1 para voltar, 0 para limpar): ", end=" ")
                try:
                    val = int(input())
                    if val < -1 or val > 5:
                        print("Valor inválido")
                    else:
                        if val != -1:
                            avaliacao[i] = val
                            i = (i + 1) % tam
                        break
                except ValueError:
                    print("Valor tem de ser um número")
        elif op == "p":
            i = (i-1 + tam) % tam
        elif op == "n":
            i = (i + 1) % tam
        elif op == "g":
            if 0 in avaliacao:
                print("Faltam avaliar as musicas: ", end=" ")
                for j in range(tam):
                    if avaliacao[j] == 0:
                        print(j, " ", end=" ")
                print()
            else:
                print("A guardar avaliacao..")
                save_data(avaliacao, "../Results/avaliacao_" + nome + ".txt", "%d")
                print("Terminou")
                break
        elif op == "m":
            for i in range(tam):
                print(f"{i}\t{int(avaliacao[i])}\t{musicas[i]}")
        else:
            try:
                inp = int(op)
                if inp >= tam or inp < 0:
                    print("Número de música inválido")
                else:
                    i = inp
            except ValueError:
                print("Opcao invalida")
