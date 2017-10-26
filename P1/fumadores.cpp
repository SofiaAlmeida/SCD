/*
  Sofía Almeida Bruno
  Sistemas concurrentes y distribuidos
  Práctica 1. 
  Fumadores
*/


#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;


const int n_fumadores = 7;
vector<Semaphore> sem;
Semaphore sem_est = 0;


//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio() {
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero() {
  int ing;
  while(true) {
    ing = aleatorio<0, n_fumadores-1>(); // Genera un ingrediente
    sem_signal(sem[ing]); // Lo pone en el mostrador
    sem_wait(sem_est);	// Espera a que el mostrador esté vacío
  }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
     sem_wait(sem[num_fumador]); // Espera el ingrediente
     sem_signal(sem_est); // Coger ingrediente
     fumar(num_fumador); // Fumar
   }
}

//----------------------------------------------------------------------

int main() {
  for(int i = 0; i < n_fumadores; ++i)
    sem.push_back(Semaphore(0));
   thread fumadores[n_fumadores];
   
   for(int i = 0; i < n_fumadores; ++i)
     fumadores[i] = thread(funcion_hebra_fumador, i);
   thread estanquero = thread(funcion_hebra_estanquero);

   for(int i = 0; i < n_fumadores; ++i)
     fumadores[i].join();
   estanquero.join();
}
