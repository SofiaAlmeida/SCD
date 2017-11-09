# Práctica 1

## Problema del productor consumidor

Este problema consiste en diseñar un programa en el que una hebra produzca datos (uno a uno) en memoria que otra hebra consuma (también uno a uno).

En este caso se ha realizado una implementación donde los datos que se producen van a una cola (`prodcons-FIFO.cpp`) y otra donde van a una pila (`prodcons-LIFO.cpp`). Además, se ha trabajado con varias hebras productoras y varias consumidoras. 

Para compilar se utiliza `g++ -std=c++11 -I. -o ./bin/prodcons-LIFO prodcons-LIFO.cpp Semaphore.cpp -lpthread`

Alternativamente, se puede utilizar el `makefile` proporcionado, estos programas se compilan y ejecutan con las reglas `pcl` y `pcf`.

## Problema de los fumadores

En este problema consideramos un estanco con tres fumadores y un estanquero. Cada fumador representa una hebra que realiza la función `fumar` en bucle infinito, para poder fumar necesita un ingrediente, que proporcionará el estanquero. El estanquero produce suministros, también infinitamente, de forma aleatoria, cada vez que lo suministra se queda esperando a que lo cojan antes de producir otro.

Este problema se encuentra resuelto en el archivo `fumadores.cpp` y se puede compilar de forma similar a los anteriores:

`g++ -std=c++11 -I. -o ./bin/fumadores fumadores.cpp Semaphore.cpp -lpthread`

Se implementa una variante donde si el número total de cigarros fumados es par, los fumadores avisan al estanquero después de fumar, y si es impar, lo hacen antes.

Utilizando el `makefile` las reglas que compilan y ejecutan estos programas son `fu` y `fu2`.
