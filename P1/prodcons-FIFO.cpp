/*
  Sofía Almeida Bruno
  Sistemas concurrentes y distribuidos
  Práctica 1. 
  Productor-consumidor FIFO
*/


#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
// variables compartidas


const int num_items = 40 ,   // número de items
	  tam_vec   = 10 ;   // tamaño del buffer
unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
          cont_cons[num_items] = {0}; // contadores de verificación: consumidos
int buffer_fifo[tam_vec] = {0};
int primera_libre = 0; // índice del vector de la primera celda libre
int primera_ocupada = 0; // índice del vector de la primera celda ocupada
Semaphore sem_productor = tam_vec; // semáforo con las celdas libres
Semaphore sem_consumidor = 0; // semáforo con los datos producidos (por tanto, que se pueden consumir)
Semaphore sc1 = 1;
Semaphore sc2 = 1;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio() {
  static default_random_engine generador((random_device())());
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato()
{
   static int contador = 0 ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "producido: " << contador << endl << flush ;

   cont_prod[contador] ++ ;
   return contador++ ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;
   
}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0; i < num_items; i++)
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora(int n_prod) {
  int iteraciones = num_items / n_prod; 
  for(unsigned i = 0; i < iteraciones; i++) {
      int dato = producir_dato();
      //FIFO
      // Escribe dato en el buffer e incrementa contador
      sem_wait(sem_productor);

      // Sección crítica, debemos garantizar que primera_libre tenga el valor adecuado
      sem_wait(sc1);
      buffer_fifo[primera_libre] = dato;
      primera_libre = (primera_libre + 1) % tam_vec;
      sem_signal(sc1);
      
      sem_signal(sem_consumidor);
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(int n_cons) {
  int iteraciones = num_items / n_cons;
   for( unsigned i = 0 ; i < iteraciones; i++ )
   {
      int dato ;
      // Almacenamos en dato el siguiente dato del buffer
      sem_wait(sem_consumidor);

      // Sección crítica, garantizamos que dos consumidores no consumen el mismo valor y que todos son consumidos
      sem_wait(sc2);
      dato = buffer_fifo[primera_ocupada];
      primera_ocupada = (primera_ocupada + 1) % tam_vec;
      sem_signal(sc2);
      sem_signal(sem_productor);
      consumir_dato( dato ) ;
    }
}
//----------------------------------------------------------------------

int main() {
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;
   
   int n_productores = 2, n_consumidores = 2;	
   thread productores[n_productores];
   thread consumidores[n_consumidores];
   
   for(int i = 0; i < n_productores; ++i)
     productores[i] = thread(funcion_hebra_productora, n_productores);
   for(int i = 0; i < n_consumidores; ++i)
     consumidores[i] = thread(funcion_hebra_consumidora, n_consumidores);

   for(int i = 0; i < n_productores; ++i)
     productores[i].join();
   for(int i = 0; i < n_consumidores; ++i)
     consumidores[i].join();

   test_contadores();
}
