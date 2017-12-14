/*
  Sofía Almeida Bruno
  Sistemas concurrentes y distribuidos
  Práctica 3. Implementación de algoritmos distribuidos con MPI
  Implementación del problema del productor-consumidor con un
  proceso intermedio que gestiona un buffer finito y recibe 
  peticiones en orden arbitrario
*/

#include <iostream>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <mpi.h>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

const int
  num_items             = 20,
  tam_vector            = 10,
  num_prod		= 4,
  num_con		= 5,
  num_procesos_esperado = num_prod + num_con + 1,
  id_buffer 		= num_prod,
  etiq_prod 		= 1,
  etiq_con 		= 2,
  etiq_buf		= 3;

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
// ---------------------------------------------------------------------
// Produce los numeros en secuencia (1,2,3,....)
// y lleva espera aleatorio
int producir(int i) {
  static int contador = i * (num_items / num_prod);
  sleep_for(milliseconds(aleatorio<10,100>()));
  contador++;
  cout << "Productor " << i << " ha producido valor " << contador << endl;
  return contador;
}
// ---------------------------------------------------------------------
// Produce valores y se los envía al buffer
void funcion_productor(int n) {
  unsigned max = num_items / num_prod;
  for (unsigned int i = 0; i < max; i++) {
    // Producir valor
    int valor_prod = producir(n);
    // Enviar valor
    cout << "Productor " << n << " va a enviar valor " << valor_prod << endl;
    MPI_Ssend(&valor_prod, 1, MPI_INT, id_buffer, etiq_prod, MPI_COMM_WORLD);
  }
}
// ---------------------------------------------------------------------
// Consume el valor, incluye espera aleatoria
void consumir(int valor_cons, int n) {
  // espera bloqueada
  sleep_for(milliseconds(aleatorio<110,200>()));
  cout << "Consumidor " << n << " ha consumido valor " << valor_cons << endl;
}
// ---------------------------------------------------------------------
// 
void funcion_consumidor(int n) {
  int         peticion,
    valor_rec = 1;
  MPI_Status  estado;

  unsigned max = num_items / num_con;
  for(unsigned int i = 0; i < max; i++) {
    // Pide al buffer un valor para consumir
    MPI_Ssend(&peticion, 1, MPI_INT, id_buffer, etiq_con, MPI_COMM_WORLD);
    // Recibe el valor del buffer
    MPI_Recv (&valor_rec, 1, MPI_INT, id_buffer, etiq_buf, MPI_COMM_WORLD,&estado);
    cout << "Consumidor " << n << " ha recibido valor " << valor_rec << endl;
    // Consume el valor recibido
    consumir(valor_rec, n);
  }
}
// ---------------------------------------------------------------------
// Recibe los datos producidos y los envía a los consumidores cuando los pidan
void funcion_buffer() {
  int        buffer[tam_vector],      // buffer con celdas ocupadas y vacías
    valor,                   // valor recibido o enviado
    primera_libre       = 0, // índice de primera celda libre
    primera_ocupada     = 0, // índice de primera celda ocupada
    num_celdas_ocupadas = 0, // número de celdas ocupadas
    id_emisor_aceptable,     // identificador de emisor aceptable
    etiq;		     // etiqueta
  MPI_Status estado;                 // metadatos del mensaje recibido

  for(unsigned int i = 0; i < num_items*2; i++) {
    // Determina de quién puede recibir mensajes
     if (num_celdas_ocupadas == 0)     // Si buffer vacío
      etiq = etiq_prod;       //  solo del productor
    else if (num_celdas_ocupadas == tam_vector) // Si buffer lleno
      etiq = etiq_con;      // solo del consumidor
    else                                     // si no vacío ni lleno
      etiq = MPI_ANY_TAG;     // cualquiera
    
    // Recibe un mensaje del emisor o emisores aceptables
    MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq, MPI_COMM_WORLD, &estado);

    // Procesa el mensaje recibido
    switch(estado.MPI_TAG) {  // leer emisor del mensaje en metadatos
      case etiq_prod: // si ha sido el productor: insertar en buffer
	buffer[primera_libre] = valor;
	primera_libre = (primera_libre+1) % tam_vector;
	num_celdas_ocupadas++;
	cout << "Buffer ha recibido valor " << valor << endl;
	break;

      case etiq_con: // si ha sido el consumidor: extraer y enviarle
	valor = buffer[primera_ocupada];
	primera_ocupada = (primera_ocupada+1) % tam_vector;
	num_celdas_ocupadas--;
	cout << "Buffer va a enviar valor " << valor << endl;
	MPI_Ssend(&valor, 1, MPI_INT, estado.MPI_SOURCE, etiq_buf, MPI_COMM_WORLD);
	break;
      }
  }
}

// ---------------------------------------------------------------------
int main(int argc, char *argv[]) {
  int id_propio, num_procesos_actual;

  // inicializar MPI, leer identif. de proceso y número de procesos
  MPI_Init( &argc, &argv );
  MPI_Comm_rank(MPI_COMM_WORLD, &id_propio);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procesos_actual);

  if (num_procesos_esperado == num_procesos_actual) {
    // ejecutar la operación apropiada a 'id_propio'
    if (id_propio < num_prod)
	funcion_productor(id_propio);
    else if (id_propio == id_buffer)
      funcion_buffer();
    else
      funcion_consumidor(id_propio % num_prod);
  }
  else {
    if (id_propio == 0) { // solo el primero escribe error, indep. del rol
      cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
	   << "el número de procesos en ejecución es: " << num_procesos_actual << endl
	   << "(programa abortado)" << endl;
    }
  }

  // al terminar el proceso, finalizar MPI
  MPI_Finalize( );
  return 0;
}
