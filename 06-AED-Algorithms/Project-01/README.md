# Missing Integer

Three approaches to finding a missing integer: exhaustive search, sorting and sum comparison, and an arithmetic-series calculation.

## Requirements

Python 3. Uses only the standard library.

## Run

```sh
python proj1.py
```

For a first run, reduce the dataset sizes in the main block.

## Project notes

The algorithms infer the interval from the observed minimum and maximum. A missing endpoint cannot be recovered this way. The random generator can remove an endpoint. The original exhaustive benchmark uses large inputs.
