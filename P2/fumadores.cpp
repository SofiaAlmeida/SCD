/*
  Sofía Almeida Bruno
  Sistemas concurrentes y distribuidos
  Práctica 2
  Problema de los fumadores usando Monitores SU
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

constexpr int num_fumadores = 3; // número de fumadores
mutex mtx;

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

//-------------------------------------------------------------------
// Monitor Estanco, semántica SU
//-------------------------------------------------------------------
class MEstancoSU : public HoareMonitor {
private:
  int mostrador; // -1 -> mostrador vacío
                 //  0 -> papel
                 //  1 -> cerillas
                 //  2 -> tabaco
  CondVar cestanquero, // cola estanquero, espera mostrador vacío
    cfumador[num_fumadores]; // una cola para cada fumador
public:
  MEstancoSU(); // constructor
  void poner_ingrediente(int ing);
  void obtener_ingrediente(int i);
  void esperar_recogida();
};

// ---------------------------------------------------------------------

MEstancoSU::MEstancoSU() {
  mostrador = -1; // inicializamos el mostrador vacío
  // creamos las colas correspondientes
  cestanquero = newCondVar(); 
  for(int i = 0; i < num_fumadores; ++i)
    cfumador[i] = newCondVar();
}

// ---------------------------------------------------------------------

void MEstancoSU::poner_ingrediente(int ing) {
  mostrador = ing; // Estanquero pone ingrediente en el mostrador
  
  mtx.lock();
  cout << "En el mostrador pongo ingrediente: " << ing << endl;
  mtx.unlock();
  
  cfumador[ing].signal(); // Avisa al fumador correspondiente
}

// ---------------------------------------------------------------------

void MEstancoSU::obtener_ingrediente(int i) {
  // Fumador espera a que esté su ingrediente
  if(mostrador != i)
    cfumador[i].wait();
  
  mtx.lock();
  cout << "Fumador " << i << " obtiene ingrediente" << endl;
  mtx.unlock();
  
  mostrador = -1; // Retira el ingrediente
  cestanquero.signal(); // Avisa al estanquero
}

// ---------------------------------------------------------------------

void MEstancoSU::esperar_recogida() {
  // Estanquero espera a que el mostrador esté vacío
  if(mostrador != -1)
    cestanquero.wait();
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(MRef<MEstancoSU> monitor) {
  int ing;
  while(true) {
    ing = aleatorio<0, num_fumadores-1>(); // Genera un ingrediente
    monitor->poner_ingrediente(ing); // Lo pone en el mostrador
    monitor->esperar_recogida(); // Espera a que el mostrador esté vacío
  }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar(int num_fumador) {

  // calcular milisegundos aleatorios de duración de la acción de fumar)
  chrono::milliseconds duracion_fumar(aleatorio<20,200>());

  // informa de que comienza a fumar
  mtx.lock();
  cout << "Fumador " << num_fumador << "  :"
       << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
  mtx.unlock();
  // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
  this_thread::sleep_for(duracion_fumar);

  // informa de que ha terminado de fumar
  mtx.lock();
  cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
  mtx.unlock();
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador

void  funcion_hebra_fumador(int num_fumador, MRef<MEstancoSU> monitor) {
  while(true) {
    monitor->obtener_ingrediente(num_fumador);
    fumar(num_fumador); // Fumar
  }
}

int main() {
  auto monitor = Create<MEstancoSU>();
  
  // Creamos y lanzamos las hebras fumadores y estanquero
  thread fumadores[num_fumadores];   
  for(int i = 0; i < num_fumadores; ++i)
    fumadores[i] = thread(funcion_hebra_fumador, i, monitor);
  thread estanquero = thread(funcion_hebra_estanquero, monitor);

  for(int i = 0; i < num_fumadores; ++i)
    fumadores[i].join();
  estanquero.join();
}
