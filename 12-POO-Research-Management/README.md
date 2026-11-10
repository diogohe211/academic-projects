# Research Group and Publication Manager

Java console application modelling researchers, research groups and publications through object-oriented classes and inheritance. Data is loaded from text files and persisted using Java object serialisation.

## Requirements

A JDK with javac and java available on PATH.

## Run

From this directory, compile and run:

```sh
javac -encoding UTF-8 -d bin src/*.java
java -cp bin projetopoo.MainProjetoPoo
```

On first use, provide Investigadores.txt, Publicações.txt and Grupos de Investigação.txt in this directory, using the format expected by Cisuc.loadDataFromTxt. A saved Cisuc.obj is loaded on later runs.

## Project notes

The original input records are absent from this portfolio copy. Use locally prepared demonstration data.
