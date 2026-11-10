# Image Compression — Multimedia

Academic practical assignment exploring the main stages of JPEG-style image compression and reconstruction in Python.

## Features

- RGB / YCbCr colour conversion and image padding.
- Chroma subsampling and interpolation comparisons.
- Discrete cosine transform (DCT) and inverse DCT.
- Quality-dependent quantisation and dequantisation.
- Differential coding of DC coefficients (DPCM).
- Image reconstruction and MSE, RMSE, SNR and PSNR measurements.
- Visualisation of intermediate channels, coefficients and reconstruction differences.

## Files

- `EntregaFinalCodigo.py`: implementation and demonstration.
- `airport.bmp`: sample image used by the demonstration.
- Included PDF: original practical assignment report and authorship information.

## Run

Use Python 3.9 or newer. Install the dependencies:

```bash
python -m pip install numpy matplotlib scipy opencv-python
```

Run from this directory so that the script can find the sample image:

```bash
python EntregaFinalCodigo.py
```

The demonstration opens successive Matplotlib figures; close each window to continue. The default configuration uses 8 × 8 blocks, 4:2:2 subsampling and a quality factor of 75. These parameters can be changed in `main()`.

## Default configuration

| Parameter in `main()` | Default | Purpose |
| --- | --- | --- |
| `filename` | `airport.bmp` | Input image, relative to the working directory |
| `BSx`, `BSy` | `8`, `8` | DCT block dimensions |
| `ratio` | `(4, 2, 2)` | Chroma subsampling |
| `qf` | `75` | Quantisation quality factor |

The demonstration compares five interpolation methods and selects the one with the smallest standard deviation of the reconstruction difference. It prints intermediate matrices and final quality metrics to the terminal.

Keep 8 × 8 blocks for the complete pipeline, since quantisation and differential coding use this block size. Use quality factors from 1 to 100.

## Skills demonstrated

Image processing, numerical programming with NumPy, frequency-domain transforms, lossy compression, reconstruction analysis and scientific visualisation.

## Scope and credits

This is an educational implementation of transform and reconstruction stages; it does not export a complete JPEG file. The original source, report and image are preserved. Consult the report for the original authors and retain the source references in the code. Syntax was checked during packaging; full execution was not tested.
