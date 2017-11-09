# Problema del Productor-Consumidor usando Monitores SU

Implementación usando la clase HoareMonitor del problema del productor consumidor con buffer acotado y múltiples productores/consumidores.

Dos implementaciones: una con la implementación que usa un objeto monitor basado en la estrategia LIFO (`prodcons_SU_LIFO.cpp`) y otra con un monitor que siga la estrategia FIFO (`prodcons_SU_FIFO.cpp`). 

Se compila usando `g++ -std=c++11 -I. -o prodcons_SU_LIFO prodcons_SU_LIFO.cpp HoareMonitor -lpthread` o las reglas `pcl` y `pcf` del `makefile`.
	
