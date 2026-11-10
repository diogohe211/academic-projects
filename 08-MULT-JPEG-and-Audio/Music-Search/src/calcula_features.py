import librosa  # https://librosa.org/    #sudo apt-get install -y ffmpeg (open mp3 files)
import numpy as np
from scipy import stats
import pandas as pd


def calcula_features(y, sr):
    # Dicionário para armazenar features de um áudio
    features = {}

    # 1. Extração de Features Espectrais
    # MFCC (primeiras 13 bandas)
    mfcc = librosa.feature.mfcc(y=y, n_mfcc=13)
    features['mfcc'] = np.atleast_2d(np.array(mfcc))

    # Spectral Centroid
    spectral_centroid = librosa.feature.spectral_centroid(y=y)
    features['spectral_centroid'] = np.atleast_2d(np.array(spectral_centroid))

    # Spectral Bandwidth
    spectral_bandwidth = librosa.feature.spectral_bandwidth(y=y)
    features['spectral_bandwidth'] = np.atleast_2d(np.array(spectral_bandwidth))

    # Spectral Contrast (primeiras 7 linhas)
    spectral_contrast = librosa.feature.spectral_contrast(y=y)
    features['spectral_contrast'] = np.atleast_2d(np.array(spectral_contrast))

    # Spectral Flatness
    spectral_flatness = librosa.feature.spectral_flatness(y=y)
    features['spectral_flatness'] = np.atleast_2d(np.array(spectral_flatness))

    # Spectral Rolloff
    spectral_rolloff = librosa.feature.spectral_rolloff(y=y)
    features['spectral_rolloff'] = np.atleast_2d(np.array(spectral_rolloff))

    # 2. Extração de Features Temporais
    # F0 (Pitch) - Fundamental frequency (F0) estimation using probabilistic YIN (pYIN)
    # f0, voiced_flag, voiced_probs = librosa.pyin(y, fmin=float(librosa.note_to_hz('C2')), fmax=float(librosa.note_to_hz('C7')))
    # features['f0'] = np.atleast_2d(np.array(f0[voiced_flag]))
    f0 = librosa.yin(y, fmin=20, fmax=sr/2)
    features['f0'] = np.atleast_2d(f0)

    # RMS (Root Mean Square)
    rms = librosa.feature.rms(y=y)
    features['rms'] = np.atleast_2d(np.array(rms))

    # Zero Crossing Rate
    zcr = librosa.feature.zero_crossing_rate(y)
    features['zcr'] = np.atleast_2d(np.array(zcr))

    # 3. Outras features
    # Tempo (BPM - batidas por minuto)
    time, _ = librosa.beat.beat_track(y=y)
    features['time'] = np.array(time)

    # No total, 190 features para cada música

    # Estatísticas para cada feature
    stats_features = np.array([])

    for feature_name, feature_data in features.items():
        if feature_name == 'time':
            stats_features = np.hstack((stats_features, feature_data))
        else:
            stats_features = np.hstack((stats_features, calculate_statistics(feature_data)))

    return stats_features


# Exercício 2.1.2. (calcular 7 estatísticas típicas sobre as features)
def calculate_statistics(data):
    # data = data[~np.isnan(data)]
    # data = data[np.isfinite(data)]
    if np.any(np.isnan(data)) or not np.all(np.isfinite(data)):
        print("Dados inválidos")
    if len(data) == 0:
        return [0] * 7  # Dados inválidos → devolve zeros

    # Calcula 7 estatísticas típicas
    # (média, desvio padrão, assimetria - skewness, curtose - kurtosis, mediana, máximo, mínimo)
    media = np.mean(data, axis=1)
    desvio = np.std(data, axis=1)
    skew = stats.skew(data, axis=1)
    curtose = stats.kurtosis(data, axis=1)
    mediana = np.median(data, axis=1)
    maximo = np.max(data, axis=1)
    minimo = np.min(data, axis=1)
    return np.vstack((media, desvio, skew, curtose, mediana, maximo, minimo)).flatten('F')
