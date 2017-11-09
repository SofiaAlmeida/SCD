/*
  Sofía Almeida Bruno
  Sistemas concurrentes y distribuidos
  Seminario 2
  Problema del Problema del Productor-Consumidor usando Monitores SU
  Estrategia LIFO
*/


#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include "HoareMonitor.hpp"

using namespace std;
using namespace HM;

constexpr int num_items = 40, // número de items a producir/consumir
  tam_vec = 10;
mutex mtx; // mutex de escritura en pantalla
unsigned cont_prod[num_items], // contadores de verificación: producidos
  cont_cons[num_items]; // contadores de verificación: consumidos

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template<int min, int max> int aleatorio() {
  static default_random_engine generador((random_device())());
  static uniform_int_distribution<int> distribucion_uniforme(min, max);
  return distribucion_uniforme(generador);
}

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato()
{
   static int contador = 0 ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx.lock();
   cout << "producido: " << contador << endl << flush ;
   mtx.unlock();
   cont_prod[contador] ++ ;
   return contador++ ;
}

//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   if ( num_items <= dato )
   {
      cout << " dato === " << dato << ", num_items == " << num_items << endl ;
      assert( dato < num_items );
   }
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx.lock();
   cout << "                  consumido: " << dato << endl ;
   mtx.unlock();
}

//----------------------------------------------------------------------

void ini_contadores()
{
   for(unsigned i = 0 ; i < num_items ; i++)
   {  cont_prod[i] = 0;
      cont_cons[i] = 0;
   }
}

//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << flush ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//-------------------------------------------------------------------
// Monitor productor-consumidor, semántica SU
//-------------------------------------------------------------------
class MProdConsSU : public HoareMonitor {
private:
  int buffer[tam_vec], // buffer donde almacenamos los valores producidos
    primera_libre; // índice en el vector de la primera celda libre
  CondVar cola_prod, // espera productor hasta que primera_libre < tam_vec
    cola_cons; // espera consumidor hasta que primera_libre > 0
public:
  MProdConsSU(); // constructor
  void escribir(int v); 
  int leer();
};

// ---------------------------------------------------------------------

MProdConsSU::MProdConsSU() {
  primera_libre = 0;
  cola_prod = newCondVar();
  cola_cons = newCondVar();
}

// ---------------------------------------------------------------------

void MProdConsSU::escribir(int v) {
  if (primera_libre == tam_vec) // Si no hay espacio para producir, espera
    cola_prod.wait();

  assert(primera_libre < tam_vec);
  buffer[primera_libre++] = v; // Escribe en buffer el valor y actualiza índice
  cola_cons.signal();
}	

// ---------------------------------------------------------------------

int MProdConsSU::leer() {
  if(primera_libre == 0) // Espera a que se produzcan valores
    cola_cons.wait();
    
  // Operación de lectura, actualizando estado del monitor
  assert(0 < primera_libre);
  int res = buffer[--primera_libre];
  cola_prod.signal();
  return res;
}

// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora(MRef<MProdConsSU> monitor, int n_prod) {
  int it = num_items / n_prod;
  for(unsigned i = 0; i < it; i++) {
    int valor = producir_dato();
    monitor->escribir(valor);
  }
}
// ---------------------------------------------------------------------

void funcion_hebra_consumidora(MRef<MProdConsSU> monitor, int n_cons) {
  int it = num_items / n_cons;
  for(unsigned i = 0; i < it; i++) {
    int valor = monitor->leer();
    consumir_dato(valor);
  }
}

// -----------------------------------------------------------------------------

int main() {
  cout << "-------------------------------------------------------------------------------" << endl
       << "Problema de los productores-consumidores (varios prod/cons, Monitor SU, buffer LIFO). " << endl
       << "-------------------------------------------------------------------------------" << endl
       << flush ;

  int n_productores = 2, // número de productores
    n_consumidores = 2; // número de consumidores
  thread productores[n_productores];
  thread consumidores[n_consumidores];
  auto monitor = Create<MProdConsSU>();

    // Lanzamos las hebras productoras y consumidoras
  for(int i = 0; i < n_productores; ++i)
    productores[i] = thread(funcion_hebra_productora, monitor, n_productores);
  for(int i = 0; i < n_consumidores; ++i)
    consumidores[i] = thread(funcion_hebra_consumidora, monitor, n_consumidores);

  // Esperamos a la finalización de todas las hebras
  for(int i = 0; i < n_productores; ++i)
    productores[i].join();
  for(int i = 0; i < n_consumidores; ++i)
    consumidores[i].join();

  // comprobar que cada item se ha producido y consumido exactamente una vez
  test_contadores() ;
}
