# Práctica 3

## Problema del productor consumidor

Implementamos el problema del productor consumidor utilizando MPI.
Tendremos un proceso buffer que mediará las comunicaciones entre productor y consumidor.

Para compilar y ejecutar se utiliza `make pcm`.

## Problema de la cena de los filósofos

En este problema tenemos 5 filósofos que en un bucle infinito comen y luego piensan. Para comer, se disponen en una mesa circular donde hay un tenedor entre cada dos filósofos, cuando un filósofo come usa en exclusión mutua sus dos tenedores adyacentes.

En la primera versión del programa `filosofos-interb.cpp` (compilar y ejecutar con `make fi`), cada filósofo solicita su tenedor izquierdo, a continuación, el derecho y entonces comienza a comer. Este programa no está libre de interbloqueo ya que, si todos cogieran su tenedor izquierdo, ninguno podría coger su tenedor derecho. Para solucionarlo uno de los filósofos comenzará tomando su tenedor derecho, esta versión se encuentra en filosofos.cpp (compilada y ejecutada mediante `make f`). La versión `filosofos-cam.cpp` (`make fc`) resuelve el problema añadiendo un nuevo proceso camarero, que dará permiso a los filósofos para sentarse cuando haya menos de 5 sentados y para levantarse en cualquier momento.
