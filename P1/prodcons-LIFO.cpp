/*
  Sofía Almeida Bruno
  Sistemas concurrentes y distribuidos
  Práctica 1. 
  Productor-consumidor LIFO
*/

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "Semaphore.h"

using namespace std;
using namespace SEM;

//**********************************************************************
// variables compartidas

const int num_items = 40,   // número de items
  tam_vec = 10;   // tamaño del buffer
unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
  cont_cons[num_items] = {0}; // contadores de verificación: consumidos
int buffer_lifo[tam_vec] = {0}; // buffer en el que almacenamos los datos producidos
int primera_libre = 0; // índice en el vector de la primera celda libre
Semaphore sem_productor = tam_vec; // semáforo con las celdas libres
Semaphore sem_consumidor = 0; // semáforo con los datos producidos (por tanto, que se pueden consumir)
Semaphore SC = 1; // semáforo binario para la sección crítica

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio() {
  static default_random_engine generador((random_device())());
  static uniform_int_distribution<int> distribucion_uniforme(min, max);
  return distribucion_uniforme(generador);
}

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato() {
  static int contador = 0;
  this_thread::sleep_for(chrono::milliseconds(aleatorio<20,100>()));

  cout << "producido: " << contador << endl << flush;

  cont_prod[contador]++;
  return contador++;
}
//----------------------------------------------------------------------

void consumir_dato(unsigned dato) {
  assert(dato < num_items);
  cont_cons[dato]++;
  this_thread::sleep_for(chrono::milliseconds(aleatorio<20,100>()));

  cout << "                  consumido: " << dato << endl;
   
}


//----------------------------------------------------------------------

void test_contadores() {
  bool ok = true;
  cout << "comprobando contadores ....";
  for(unsigned i = 0; i < num_items; i++) {
    if (cont_prod[i] != 1) {
      cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl;
      ok = false;
    }
    if (cont_cons[i] != 1) {
      cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl;
      ok = false;
    }
  }
  if (ok)
    cout << endl << flush << "solución (aparentemente) correcta." << endl << flush;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora(int n_prod) {
  int it = num_items / n_prod;
  for(unsigned i = 0; i < it; i++) {
    int dato = producir_dato() ;
    // Para poder escribir tiene que haber espacio en el buffer
    sem_wait(sem_productor);

    // Sección Crítica
    sem_wait(SC);
    buffer_lifo[primera_libre] = dato; // Escribimos dato en el buffer
    primera_libre++; // Actualizamos índice
    sem_signal(SC);
    
    sem_signal(sem_consumidor); // Indicamos al consumidor que puede consumir un dato
  }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(int n_cons) {
  int it = num_items / n_cons;
  for( unsigned i = 0 ; i < it; i++ ) {
      int dato ;
      // Cogemos dato del buffer
      sem_wait(sem_consumidor); // Espera a que se haya producido un dato

      // Sección Crítica
      sem_wait(SC);
      primera_libre--; // Tomamos el índice con el último valor producido
      dato = buffer_lifo[primera_libre]; 
      sem_signal(SC);
      
      sem_signal(sem_productor); // Indicamos que este espacio ya está libre
      consumir_dato(dato) ;
    }
}
//----------------------------------------------------------------------

int main()
{
  cout << "--------------------------------------------------------" << endl
       << "Problema de los productores-consumidores (solución LIFO)." << endl
       << "--------------------------------------------------------" << endl
       << flush ;
   
  int n_productores = 2, n_consumidores = 2;	
  thread productores[n_productores];
  thread consumidores[n_consumidores];

  // Lanzamos las hebras productoras y consumidoras
  for(int i = 0; i < n_productores; ++i)
    productores[i] = thread(funcion_hebra_productora, n_productores);
  for(int i = 0; i < n_consumidores; ++i)
    consumidores[i] = thread(funcion_hebra_consumidora, n_consumidores);

  // Esperamos a la finalización de todas las hebras
  for(int i = 0; i < n_productores; ++i)
    productores[i].join();
  for(int i = 0; i < n_consumidores; ++i)
    consumidores[i].join();

  test_contadores();
}
