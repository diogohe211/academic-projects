# Evolutionary Lunar Lander

Evolutionary optimisation of neural-network weights for Gymnasium LunarLander-v3. The implementation includes selection, crossover, mutation and fitness evaluation, with a separate test mode for evolved controllers.

## Requirements

Python 3, NumPy and Gymnasium with its Box2D dependencies.

## Run

```sh
python -m pip install numpy "gymnasium[box2d]"
python NE-LunarLander-alunos.py
```

Before running, choose the mode in the main block. Set evolve = True to train and generate log0.txt through log4.txt. Set evolve = False to evaluate those saved files. Set render_mode = "human" for visual evaluation.

## Project notes

The supplied script defaults to evaluation mode and requires existing log files. Training can be lengthy. 
