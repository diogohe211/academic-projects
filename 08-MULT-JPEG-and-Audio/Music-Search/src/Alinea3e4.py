import os
import numpy as np
from scipy.spatial import distance
import librosa
import warnings
import pandas as pd

from calcula_features import calcula_features

from save_data import save_data

# EXERCÍCIO 3.1. - Implementação de métricas de similaridade
# (distância Euclidiana, distância de Manhattan, distância do Coseno)


# Calcula a distância euclidiana entre dois vetores
def euclidean_distance(x, y):
    x = np.atleast_2d(x)
    y = np.atleast_2d(y)
    return np.sqrt(np.sum(np.square(x - y), axis=1))


# Calcula a distância de Manhattan entre dois vetores
def manhattan_distance(x, y):
    x = np.atleast_2d(x)
    y = np.atleast_2d(y)
    return np.sum(np.abs(x - y), axis=1)


# Calcula a distância do cosseno entre dois vetores
def cosine_distance(x, y):
    x = np.atleast_2d(x)
    y = np.atleast_2d(y)
    return 1 - (np.sum(x*y, axis=1)/(np.sqrt(np.sum(np.square(x), axis=1))*np.sqrt(np.sum(np.square(y), axis=1))))


# EXERCÍCIO 3.2. - Processamento da Query
def process_query(y_query, sr_query, bd, min_vals, max_vals):
    stats_features = calcula_features(y_query, sr_query) 
    dif = max_vals - min_vals
    dif = np.where(dif == 0, 1, dif)
    stats_features = (stats_features - min_vals)/dif

    de = euclidean_distance(stats_features, bd)
    dm = manhattan_distance(stats_features, bd)
    dc = cosine_distance(stats_features, bd)
    
    save_data(de.T, "../Results/ranking_euclidean.csv")
    save_data(dm.T, "../Results/ranking_manhattan.csv")
    save_data(dc.T, "../Results/ranking_cosseno.csv")
    
    return de, dm, dc


# EXERCÍCIO 3.3. - Criar ranking de similaridade
def create_ranking(de, dm, dc, audio_files):
    print("- Rankings:")
    de_aux = de.argsort()
    dm_aux = dm.argsort()
    dc_aux = dc.argsort()

    de_ranking = audio_files[de_aux]
    dm_ranking = audio_files[dm_aux]
    dc_ranking = audio_files[dc_aux]

    de = de[de_aux]
    dm = dm[dm_aux]
    dc = dc[dc_aux]

    print("\nRanking: Euclidean-------------")
    print(de_ranking[:11])
    print(de[:11])
    print("\nRanking: Manhattan-------------")
    print(dm_ranking[:11])
    print(dm[:11])
    print("\nRanking: Cosine-------------")
    print(dc_ranking[:11])
    print(dc[:11])

    return de_ranking[:11], dm_ranking[:11], dc_ranking[:11]


# EXERCÍCIO 4.1.1 - Criar ranking de similaridade baseado em metadados
def process_metadata(metadados, audio_file_name):

    query_line = metadados[metadados[:, 0] == audio_file_name].flatten()

    # Compara artista
    res = (metadados[:, 1] == query_line[1]).astype(np.int32)

    moodsQuery = set(str(query_line[2]).split("; "))
    genresQuery = set(str(query_line[3]).split("; "))

    # Para cada metadado, obtém os vários moods
    # Incrementa a pontuação deste de acordo com o tamanho da interseção entre os seus moods e os da query
    for i in range(metadados.shape[0]):
        res[i] += len(list(set(str(metadados[i, 2]).split("; ")) & moodsQuery))

    # Para cada metadado, obtém os vários géneros
    # Incrementa a pontuação deste de acordo com o tamanho da interseção entre os seus géneros e os da query
    for i in range(metadados.shape[0]):
        res[i] += len(list(set(str(metadados[i, 3]).split("; ")) & genresQuery))

    save_data(res.T, "../Results/ranking_metadados.csv", '%d')

    return res


def create_ranking_metadata(md, audio_files):
    md_aux = len(md) - 1 - np.argsort(md[::-1], stable=True)[::-1]

    md_ranking = audio_files[md_aux]

    md = md[md_aux]

    print("\nRanking: Metadata-------------")
    print(md_ranking[:11])
    print(md[:11])

    return md_ranking[:11]


def calculate_precision(de_ranking, dm_ranking, dc_ranking, md_ranking):
    de_ranking = set(de_ranking)
    dm_ranking = set(dm_ranking)
    dc_ranking = set(dc_ranking)
    md_ranking = set(md_ranking)
    total = len(md_ranking)
    print()
    print("Precision de: %.1f" % (len(list(de_ranking & md_ranking))*100/total))
    print("Precision dm: %.1f" % (len(list(dm_ranking & md_ranking))*100/total))
    print("Precision dc: %.1f" % (len(list(dc_ranking & md_ranking))*100/total))


if __name__ == "__main__":
    warnings.filterwarnings("ignore")
    sr = 22050
    mono = True
    # Carregar o ficheiro de features normalizadas (900 músicas)
    all_data = np.genfromtxt("../Results/featuresNorm.csv", delimiter=",")
    min_vals = all_data[0]
    max_vals = all_data[1]
    data = all_data[2:]  # [n_songs x n_features]

    # Obter a lista dos ficheiros
    audio_dir = "../MER_audio_taffc_dataset/audio"
    audio_files = sorted([f for f in os.listdir(audio_dir) if f.endswith(".mp3")])
    audio_files = np.array(audio_files)

    # Definir query
    query_file = "../Queries/MT0000414517.mp3"
    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        y, sr = librosa.load(query_file, sr=sr, mono=mono)

    de, dm, dc = process_query(y, sr, data, min_vals, max_vals)

    de_ranking, dm_ranking, dc_ranking = create_ranking(de, dm, dc, audio_files)

    metadados = np.array(pd.read_csv("../panda_dataset_taffc_metadata.csv", delimiter=",").values)
    metadados = metadados[:, (0, 1, 9, 11)]

    nome_audio = (query_file.split("/")[-1]).split(".")[0]
    md = process_metadata(metadados, nome_audio)

    md_ranking = create_ranking_metadata(md, audio_files)

    calculate_precision(de_ranking, dm_ranking, dc_ranking, md_ranking)
