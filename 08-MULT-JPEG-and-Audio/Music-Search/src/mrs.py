"""
Created on Tue Apr  6 13:03:06 2021

@author: rpp
"""

import librosa  # https://librosa.org/    #sudo apt-get install -y ffmpeg (open mp3 files)
import librosa.display
import librosa.beat
import sounddevice as sd  # https://anaconda.org/conda-forge/python-sounddevice
import warnings
import numpy as np
import matplotlib.pyplot as plt
import os
from scipy import stats
import pandas as pd

from calcula_features import calcula_features
from save_data import save_data


# Exercício 2.1.1. (extrair features)
def extract_features(audio_dir, sr, mono):
    # Armazena todas as features
    all_features = []

    # Lista de áudios
    audio_files = [f for f in os.listdir(audio_dir) if f.endswith('.mp3')]
    audio_files.sort()  # Garantir consistência na ordem

    for i, audio_file in enumerate(audio_files):
        print(f"A processar áudio {i + 1}/{len(audio_files)} : {audio_file}")

        # Carregar o áudio
        file_path = os.path.join(audio_dir, audio_file)
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")
            y, sr = librosa.load(file_path, sr, mono)
        stats_features = calcula_features(y, sr)

        # Adicionar estatísticas das features
        all_features.append(stats_features)

    return np.array(all_features), audio_files


# Exercício 2.1.3 (normalizar valores)
def normalize(data):
    mini = np.min(data, axis=0)
    maxi = np.max(data, axis=0)
    dif = maxi - mini
    auxMini = np.tile(mini, (data.shape[0], 1))
    dif = np.tile(dif, (data.shape[0], 1))
    dif = np.where(dif == 0, 1, dif)
    novaData = data - auxMini
    novaData = novaData/dif
    return np.vstack((mini, maxi, novaData))


if __name__ == "__main__":
    plt.close('all')

    # --- Load file
    fName = "../Queries/MT0000414517.mp3"
    sr = 22050
    mono = True
    warnings.filterwarnings("ignore")
    y, fs = librosa.load(fName, sr=sr, mono=mono)

    # --- Play Sound
    sd.play(y, sr, blocking=False)

    # --- Plot sound waveform
    plt.figure()
    librosa.display.waveshow(y)

    # --- Plot spectrogram
    Y = np.abs(librosa.stft(y))
    Ydb = librosa.amplitude_to_db(Y, ref=np.max)
    fig, ax = plt.subplots()
    img = librosa.display.specshow(Ydb, y_axis='linear', x_axis='time', ax=ax)
    ax.set_title('Power spectrogram')
    fig.colorbar(img, ax=ax, format="%+2.0f dB")

    # --- Extract features
    sc = librosa.feature.spectral_centroid(y=y)  # default parameters: sr = 22050 Hz, mono, window length = frame length = 92.88 ms e hop length = 23.22 ms
    sc = sc[0, :]
    times = librosa.times_like(sc)
    plt.figure(), plt.plot(times, sc)
    plt.xlabel('Time (s)')
    plt.title('Spectral Centroid')

    # EXERCÍCIO 2
    # 900 áudios
    audio_directory = "../MER_audio_taffc_dataset/audio"
    # audio_directory = "../Queries"

    # Extração de features
    features_array, audio_files = extract_features(audio_directory)

    output_not_normalized_features_file = "../Results/featuresNotNorm.csv"

    save_data(features_array, output_not_normalized_features_file)

    # Normalizar features no intervalo [0,1]
    normalized_features_array = normalize(features_array)

    # Ficheiro de output
    output_normalized_features_file = "../Results/featuresNorm.csv"

    # Gravar features
    save_data(normalized_features_array, output_normalized_features_file)

    print(f"\nFeatures extraídas e gravadas em {output_not_normalized_features_file} e {output_normalized_features_file}!")
