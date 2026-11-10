import librosa
import warnings
import numpy as np
import os
import scipy
from numpy.lib.stride_tricks import sliding_window_view

from save_data import save_data


def calcula_spectral_centroid(data, sr, tamJanelas, salto):
    if salto > tamJanelas:
        print("Vals tamJanelas e salto incompatíveis")
        return
    data = np.hstack((data, np.zeros(tamJanelas - data.shape[0] % tamJanelas)))
    janelas = sliding_window_view(data, tamJanelas)[::salto, :]
    janelas = janelas * np.hanning(tamJanelas)
    transform = np.abs(scipy.fft.rfft(janelas))
    aux = np.sum(transform, axis=1)
    aux = np.where(aux == 0, 1, aux)
    sc = np.sum(transform * scipy.fft.rfftfreq(tamJanelas, 1 / sr), axis=1) / aux
    return sc


def calcula_erro(spectral_centroid, sc):
    spectral_centroid = spectral_centroid[0][2:]
    sc = sc[:-(3 - (spectral_centroid.shape[0] - sc.shape[0]))]
    if spectral_centroid.shape[0] > sc.shape[0]:
        mini = sc.shape[0]
    else:
        mini = spectral_centroid.shape[0]
    spectral_centroid = spectral_centroid[:mini]
    sc = sc[:mini]
    rmse = np.sqrt(np.mean(np.square(np.abs(spectral_centroid - sc))))
    corr = np.corrcoef(spectral_centroid[:mini], sc[:mini])
    return corr[0][0], rmse


if __name__ == "__main__":
    # audio_dir = "../Queries"
    audio_dir = "../MER_audio_taffc_dataset/audio"
    sr = 22050
    mono = True
    warnings.filterwarnings("ignore")
    audio_files = [f for f in os.listdir(audio_dir) if f.endswith('.mp3')]
    audio_files.sort()  # Garantir consistência na ordem

    metrics = np.array([0, 0])
    for i, audio_file in enumerate(audio_files):
        print(f"A processar áudio {i + 1}/{len(audio_files)} : {audio_file}")

        # Carregar o áudio
        file_path = os.path.join(audio_dir, audio_file)
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")
            y, sr = librosa.load(file_path, sr=sr, mono=mono)

        spectral_centroid = librosa.feature.spectral_centroid(y=y)
        sc = calcula_spectral_centroid(y, sr, 2048, 512)
        corr, rmse = calcula_erro(np.float64(spectral_centroid), np.float64(sc))
        metrics = np.vstack((metrics, np.array([corr, rmse])))
        # print(spectral_centroid.shape[1], sc.shape[0])
    save_data(metrics[1:], "../Results/SC_all_metrics.csv")
    # print(metrics)
