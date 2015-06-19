#!/bin/bash

# Semplice script che compila ed esegue un file sorgente
# linkando le librerie SFML.

clang++ -std=c++1y -O0 \
		-lsfml-system -lsfml-window -lsfml-graphics \
		"${@:2}" ./$1 -o /tmp/$1.temp && /tmp/$1.temp