# Práctica 2

## Problema de los fumadores

Volveremos a implementar el problema de los fumadores (explicado en `P1`) en este caso utilizando la clase HoareMonitor.

Para compilar y ejecutar `fumadores.cpp` basta escribir el comando `make fu`.

## Problema del barbero durmiente

Aquí se implementa la solución para una barbería en la que un único barbero va atiendo clientes en una sala de corte, mientras los clientes que llegan y lo encuentran ocupado esperan en una sala de espera, cuando no haya clientes el barbero se duerme hasta que llegue un cliente que lo despierte. Al igual que en el problema anterior, se utiliza la clase HoareMonitor, además tendremos una hebra barbero y muchas hebras clientes. Esta implementación se encuentra en `barbero_durmiente.cpp` y se compila y ejecuta mediante `make bd`.

Además, tenemos `barbero_durmiente2.cpp` que añade dos variaciones al problema anterior. Por un lado, se establece un aforo máximo en la sala de espera, así, los clientes que la encuentren llena no podrán quedarse en la barbería. Por otro, se añaden varios barberos más. La ejecución y compilación se realiza con `make bd2`.
