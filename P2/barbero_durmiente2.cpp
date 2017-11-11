/*
  Sofía Almeida Bruno
  Sistemas concurrentes y distribuidos
  Práctica 2
  Problema del barbero durmiente usando Monitores SU
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

const int n_clientes = 10; // número de clientes
const int aforo_max = 7; // número máximo de clientes en la sala de espera

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
// Monitor Barberia, semántica SU
//-------------------------------------------------------------------
class MBarberiaSU : public HoareMonitor {
private:
  int n; // número de clientes esperando
  bool cortando; // true si hay un cliente al que le están cortando el pelo
  CondVar sala_espera, // cola donde están los clientes de la sala de espera
    sala_corte, // cola donde están los clientes cortándose el pelo
    barbero; // cola en la que el barbero duerme
public:
  MBarberiaSU(); // constructor
  void cortar_pelo(int i);
  void siguiente_cliente();
  void fin_cliente();
};

// ---------------------------------------------------------------------

MBarberiaSU::MBarberiaSU() {
  n = 0; 
  cortando = false;
  sala_espera = newCondVar();
  sala_corte = newCondVar();
  barbero = newCondVar();
}

// ---------------------------------------------------------------------

void MBarberiaSU::cortar_pelo(int i) {
  cout << "Cliente " << i << " entra a la barbería" << endl;
   
  if(n == aforo_max) {
    cout << "	   Cliente " << i << " encuentra la barbería llena" << endl;
  }
  else {
    if(n != 0 || cortando) {
      n++;
      sala_espera.wait(); // Espera a que el barbero esté libre
      n--; // Sale de la sala de espera
    }

    barbero.signal(); // Despierta al barbero
    cortando = true; // Actualiza valor

    cout << "	       	Cliente " << i << " va a cortarse el pelo" << endl;
  
    sala_corte.wait(); // Espera a que el barbero termine de cortar
  
    cout << "		Cliente " << i << " termina de cortarse el pelo" << endl;
  }
  
  cout << "					Cliente " << i << " sale de la barbería" << endl;
}

// ---------------------------------------------------------------------

void MBarberiaSU::siguiente_cliente() {
  if(sala_espera.empty()) // Si no hay clientes esperando
    barbero.wait(); // Duerme hasta que lo despierte un cliente
}

// ---------------------------------------------------------------------

void MBarberiaSU::fin_cliente() {
  sala_corte.signal(); // Avisa al cliente de que ha terminado
  cortando = false; // Actualiza valor
  sala_espera.signal();
}

//----------------------------------------------------------------------
// Función que simula la acción de cortar el pelo, como un retardo aleatorio de la hebra
void cortar_pelo_cliente() {
  // calcular milisegundos aleatorios de duración de la acción de cortar el pelo)
  chrono::milliseconds duracion_cortar(aleatorio<20,200>());
  
  // espera bloqueada un tiempo igual a 'duracion_cortar' milisegundos
  this_thread::sleep_for(duracion_cortar);
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del barbero

void funcion_hebra_barbero(MRef<MBarberiaSU> monitor) {
  while(true) {
    monitor->siguiente_cliente();
    cortar_pelo_cliente();
    monitor->fin_cliente();
  }
}

//-------------------------------------------------------------------------
// Función que simula la espera de los clientes fuera de la barbería
void esperar_fuera_barberia() {
  // calcular milisegundos aleatorios de duración de la acción de esperar fuera de la barbería
  chrono::milliseconds duracion_espera(aleatorio<20,200>());
  
  // espera bloqueada un tiempo igual a 'duracion_espera' milisegundos
  this_thread::sleep_for(duracion_espera);
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del cliente

void  funcion_hebra_cliente(int num_cliente, MRef<MBarberiaSU> monitor) {
  while(true) {
    monitor->cortar_pelo(num_cliente);
    esperar_fuera_barberia(); 
  }
}

int main() {
  auto monitor = Create<MBarberiaSU>();
  
  // Creamos y lanzamos las hebras clientes y barbero
  thread clientes[n_clientes];   
  for(int i = 0; i < n_clientes; ++i)
    clientes[i] = thread(funcion_hebra_cliente, i, monitor);
  thread barbero = thread(funcion_hebra_barbero, monitor);

  for(int i = 0; i < n_clientes; ++i)
    clientes[i].join();
  barbero.join();
}
