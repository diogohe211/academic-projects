# Music Similarity Search

Audio feature extraction and similarity ranking using Euclidean, Manhattan and cosine distances. Additional scripts compare metadata rankings and analyse subjective listening assessments.

## Requirements

Python 3, NumPy, SciPy, pandas, librosa, matplotlib and sounddevice. Audio decoding and playback depend on the local system libraries.

## Run

Restore MER_audio_taffc_dataset/audio, Queries, the metadata CSV files and any assessment inputs referenced in the scripts. Create Results. Run commands from src so relative paths resolve correctly.

```sh
python mrs.py
python Alinea3e4.py
```

Alinea22.py evaluates the spectral-centroid implementation. toca_musica.py supports listening assessments; Alinea42.py analyses the assessment results.

## Project notes

The code uses an older positional librosa.load call in mrs.py. Check compatibility with the installed librosa version. Audio and metadata were omitted from this portfolio copy.
