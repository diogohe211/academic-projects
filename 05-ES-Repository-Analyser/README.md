# Repository Analyser

Application for exploring source-code metrics and repository history. A Python backend exposes data for JavaScript views including tables, bubble charts, histograms and scatter plots.

## Requirements

Python 3, Git, Flask, flask-cors, PyDriller, GitPython and complexipy. The frontend uses HTML, CSS and JavaScript.

## Run

Configure the repository path used by MainBackEnd before starting the server. Install dependencies and launch the backend from back_end.

```sh
python -m pip install flask flask-cors pydriller GitPython complexipy
python BackEndServer.py
```

Serve front_end in another terminal and open its index.html.

```sh
python -m http.server 8000
```

Check the frontend API URLs against the backend address.

## Project notes

Tests are supplied as back_end/ServerTest.py and back_end/testMainBackEnd.py. Test execution requires their dependencies and configured fixtures. This is a team project; contributions include visualisation and metrics interfaces.
