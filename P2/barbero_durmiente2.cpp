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
const int n_barberos = 2; // número de barberos
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
  int b; // número de barberos ocupados
  CondVar sala_espera, // cola donde están los clientes de la sala de espera
    silla[n_barberos], // cola donde están los clientes cortándose el pelo
    barberos[n_barberos]; // cola en la que el barbero duerme
public:
  MBarberiaSU(); // constructor
  void cortar_pelo(int i);
  void siguiente_cliente(int i);
  void fin_cliente(int i);
};

// ---------------------------------------------------------------------

MBarberiaSU::MBarberiaSU() {
  n = 0;
  b = 0;
  sala_espera = newCondVar();
  for(int i = 0; i < n_barberos; ++i) {
    barberos[i] = newCondVar();
    silla[i] = newCondVar();
  }
}

// ---------------------------------------------------------------------

void MBarberiaSU::cortar_pelo(int i) {
  cout << "Cliente " << i << " entra a la barbería" << endl;
   
  if(n == aforo_max) {
    cout << "	   Cliente " << i << " encuentra la barbería llena" << endl;
  }
  else {
    if(b == n_barberos) {
      n++; // Entra en la sala de espera
      sala_espera.wait(); // Espera a que el barbero esté libre
      n--; // Sale de la sala de espera
    }

    int j;
    
    for(j = 0; !silla[j].empty(); j++);
    
    barberos[j].signal(); // Despierta al barbero
    b++; // Actualiza valor
    
    cout << "	       	Cliente " << i << " va a cortarse el pelo con el barbero " << j << endl;
  
    silla[j].wait(); // Espera a que el barbero termine de cortar
  
    cout << "		Cliente " << i << " termina de cortarse el pelo" << endl;
  }
  
  cout << "					Cliente " << i << " sale de la barbería" << endl;
}

// ---------------------------------------------------------------------

void MBarberiaSU::siguiente_cliente(int i) {
  if(sala_espera.empty()) // Si no hay clientes esperando
    barberos[i].wait(); // Duerme hasta que lo despierte un cliente
  cout << endl;
}

// ---------------------------------------------------------------------

void MBarberiaSU::fin_cliente(int i) {
  silla[i].signal(); // Avisa al cliente de que ha terminado
  b--;
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

void funcion_hebra_barbero(int n_barbero, MRef<MBarberiaSU> monitor) {
  while(true) {
    monitor->siguiente_cliente(n_barbero);
    cortar_pelo_cliente();
    monitor->fin_cliente(n_barbero);
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
  thread barberos[n_barberos];
  for(int i = 0; i < n_barberos; ++i)
    barberos[i] = thread(funcion_hebra_barbero, i, monitor);
  
  for(int i = 0; i < n_clientes; ++i)
    clientes[i].join();
  for(int i = 0; i < n_barberos; ++i)
    barberos[i].join();
}
