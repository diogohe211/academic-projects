# Lossless Image Codec

Python image-compression experiment combining run-length coding and Huffman coding. The script decompresses its output and compares the reconstructed image array with the input.

## Requirements

Python 3, NumPy and matplotlib. The local huffmancodec.py module accompanies the project.

## Run

```sh
python -m pip install numpy matplotlib
python TP2.py
```

Enter the input image filename and the desired compressed output filename when prompted. The original coursework used BMP images.

## Project notes

Run from this directory with the input images available. The output reports file sizes and whether the reconstructed array matches the original. Retain the third-party credits in huffmancodec.py.
