/*
  Sofía Almeida Bruno
  Sistemas concurrentes y distribuidos
  Práctica 3. Implementación de algoritmos distribuidos con MPI
  Implementación del problema de los filósofos (con camarero)
*/

#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

const int
  num_filosofos   = 5,
  num_procesos    = (2 * num_filosofos) + 1,
  etiq_liberar    = 0,
  etiq_ocupar     = 1,
  etiq_sentarse   = 2,
  etiq_levantarse = 3,
  id_camarero 	  = 10;


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

// ---------------------------------------------------------------------

void funcion_filosofos(int id) {
  int id_ten_izq = (id+1)              % (num_procesos - 1), //id. tenedor izq.
    id_ten_der = (id+num_procesos-2) % (num_procesos - 1); //id. tenedor der.

  while (true) {
    // Se sienta
    cout << "Filósofo " << id << " se quiere sentar" << endl;
    MPI_Ssend(&id, 1, MPI_INT, id_camarero, etiq_sentarse, MPI_COMM_WORLD);
    
    if(id != 0) {
      // Solicita tenedor izquierdo
      cout << "Filósofo " << id << " solicita ten. izq." << id_ten_izq << endl;
      MPI_Ssend(&id, 1, MPI_INT, id_ten_izq, etiq_ocupar, MPI_COMM_WORLD);
    
      // Solicita tenedor derecho 
      cout << "Filósofo " << id << " solicita ten. der." << id_ten_der << endl;
      MPI_Ssend(&id, 1, MPI_INT, id_ten_der, etiq_ocupar, MPI_COMM_WORLD);
    } else {
      // Solicita tenedor derecho 
      cout << "Filósofo " << id << " solicita ten. der." << id_ten_der << endl;
      MPI_Ssend(&id, 1, MPI_INT, id_ten_der, etiq_ocupar, MPI_COMM_WORLD);

      // Solicita tenedor izquierdo
      cout << "Filósofo " << id << " solicita ten. izq." << id_ten_izq << endl;
      MPI_Ssend(&id, 1, MPI_INT, id_ten_izq, etiq_ocupar, MPI_COMM_WORLD);
    }
    
    // Come
    cout << "Filósofo " << id << " comienza a comer" << endl;
    sleep_for(milliseconds(aleatorio<10,100>()));

    // Libera tenedor izquierdo
    cout << "Filósofo " << id << " suelta ten. izq. " << id_ten_izq << endl;
    MPI_Ssend(&id, 1, MPI_INT, id_ten_izq, etiq_liberar, MPI_COMM_WORLD);

    // Soltar tenedor derecho
    cout << "Filósofo " << id << " suelta ten. der. " << id_ten_der << endl;
    MPI_Ssend(&id, 1, MPI_INT, id_ten_der, etiq_liberar, MPI_COMM_WORLD);

    // Se levanta
    cout << "Filósofo " << id << " se quiere levantar" << endl;
    MPI_Ssend(&id, 1, MPI_INT, id_camarero, etiq_levantarse, MPI_COMM_WORLD);
    
    // Piensa
    cout << "Filósofo " << id << " comienza a pensar" << endl;
    sleep_for(milliseconds(aleatorio<10,100>()));
  }
}
// ---------------------------------------------------------------------

void funcion_tenedores(int id) {
  int valor, id_filosofo;  // valor recibido, identificador del filósofo
  MPI_Status estado;       // metadatos de las dos recepciones

  while (true) {
    // Recibe una petición de cualquier filósofo
    MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_ocupar, MPI_COMM_WORLD, &estado);
    id_filosofo = estado.MPI_SOURCE;
    cout << "Ten. " << id << " ha sido cogido por filo. " << id_filosofo << endl;

    // Recibe la liberación de filósofo 'id_filosofo'
    MPI_Recv(&valor, 1, MPI_INT, id_filosofo, etiq_liberar, MPI_COMM_WORLD, &estado);
    cout << "Ten. " << id << " ha sido liberado por filo. " << id_filosofo << endl;
  }
}
// ---------------------------------------------------------------------
void funcion_camarero(int id) {
  int valor, id_filosofo, etiq; // valor recibido, identificador del filósofo, etiqueta recibida
  int cont = 0; // número de filósofos sentados
  MPI_Status estado; // metadatos de las recepciones
  
  while(true) {
    // Determinar qué mensajes puede recibir
    if(cont == 0) 	// Si la mesa está vacía
      etiq = etiq_sentarse;
    else if (cont == 4) // Si la mesa está llena
      etiq = etiq_levantarse;
    else 		// Si no está ni vacía ni llena
      etiq = MPI_ANY_TAG;

    // Recibir un mensaje
    MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq, MPI_COMM_WORLD, &estado);

    // Procesar el mensaje recibido
    switch(estado.MPI_TAG) {
    case etiq_sentarse:
      cont++;
      cout << "Filósofo " << estado.MPI_SOURCE << " se sienta" << endl;
      break;
    case etiq_levantarse:
      cout << "Filósofo " << estado.MPI_SOURCE << " se levanta" << endl;
      cont--;
      break;
    }
  }
}

// ---------------------------------------------------------------------

int main(int argc, char** argv) {
  int id_propio, num_procesos_actual;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &id_propio);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procesos_actual);

  if (num_procesos == num_procesos_actual) {
    // ejecutar la función correspondiente a 'id_propio'
    if (id_propio % 2 == 0)         // si es par
      if(id_propio != id_camarero)  // si no es 10
	funcion_filosofos(id_propio); // es un filósofo
      else
	funcion_camarero(id_propio); // es el camarero
    else                               // si es impar
      funcion_tenedores(id_propio); //   es un tenedor
  }
  else {
    if (id_propio == 0) {
      // solo el primero escribe error, indep. del rol
      cout << "el número de procesos esperados es:    " << num_procesos << endl
	   << "el número de procesos en ejecución es: " << num_procesos_actual << endl
	   << "(programa abortado)" << endl;
    }
  }

  MPI_Finalize();
  return 0;
}

// ---------------------------------------------------------------------
