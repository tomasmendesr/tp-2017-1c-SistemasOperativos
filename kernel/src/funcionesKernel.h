/*
 * kernel.h
 *
 *  Created on: 4/4/2017
 *      Author: utnso
 */

#ifndef FUNCIONESKERNEL_H_
#define FUNCIONESKERNEL_H_

#define IP "127.0.0.1"
#define BACKLOG 10
#define configuracionKernel "../confKernel.init"
#define MAX_LEN_PUERTO 6
#define MAX_LEN_IP 20

#include <stdio.h>
#include <stdlib.h>
#include <commons/sockets.h>
#include <string.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include <commons/interface.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/cosas.h>
#include <stdbool.h>
#include <pthread.h>
//#include "plp.h"
//#include "pcp.h"
#include "pcb.h"
#include "operaciones.h"

typedef struct {
	char* puerto_PROG;
	char* puerto_CPU;
	char* ip_Memoria;
	char* puerto_Memoria;
	char* ip_FS;
	char* puerto_FS;
	int quantum;
	int quantum_Sleep;
	char* algoritmo;
	int grado_MultiProg;
	t_dictionary* semaforos;
	t_dictionary* variablesGlobales;
	int stack_Size;

} t_config_kernel;

typedef struct{
	uint32_t pid;
	uint32_t cant_pag;
}t_pedido_iniciar;

typedef struct{
	int socket; //funciona como id del cpu
	pcb_t* pcb;
} cpu_t;

typedef struct{
	int socketConsola;
	char* codigo;
	int pid;
}proceso_en_espera_t;


typedef struct{
	uint32_t pid;
	uint8_t estado;
	uint32_t cantRafagas;
	uint32_t cantSyscalls;
	uint32_t cantOpPrivi;
	uint32_t cantPaginasHeap;
	uint32_t cantAlocar;
	uint32_t cantLiberar;
}info_estadistica_t;

enum enum_estado{
	NEW = 1,
	READY = 2,
	FINISH = 3,
	EXEC = 4
};

void inicializarColas();
void inicializaciones(void);
void crearConfig(int argc, char* argv[]);
t_config_kernel* levantarConfiguracionKernel(char* archivo_conf);
void destruirConfiguracionKernel(t_config_kernel* config);
t_dictionary* crearDiccionarioConValue(char** array, char** valores);
t_dictionary* crearDiccionario(char** array);
void modificarValorDiccionario(t_dictionary* dic, char* key, void* data);
int semaforoSignal(t_dictionary* dic, char* key);
int semaforoWait(t_dictionary* dic, char* key);
int leerVariableGlobal(t_dictionary* dic, char* key);
void escribirVariableGlobal(t_dictionary* dic, char* key, void* nuevoValor);

void establecerConexiones();
int conexionConFileSystem();
int conexionConMemoria();
void trabajarConexionCPU();
void enviarTamanioStack(int fd);

void conectarConServidores();
void escucharConexiones();
void aceptarNuevaConexion(int socketEscucha, fd_set* set);

//Mensajes con consola
void trabajarConexionConsola();
void procesarMensajeConsola(int consola_fd, int mensaje, char* package);
proceso_en_espera_t* crearProcesoEnEspera(int consola_fd, char* package);
int asignarPid();


//Funciones de interfaz
void levantarInterfaz();
void listProcesses(char* comando, char* param);
void processInfo(char* comando, char* param);
void getTablaArchivos(char* comando, char* param);
void gradoMultiprogramacion(char* comando, char* param);
void killProcess(char*,char*);
void stopPlanification(char*,char*);

//Funciones de manejo de CPUs
void agregarNuevaCPU(t_list* lista, int socketCPU);
void liberarCPU(cpu_t* cpu);
void eliminarCPU(t_list* lista, int socketCPU);
void actualizarReferenciaPCB(int id, pcb_t* pcb);
cpu_t* obtenerCpuLibre();

//Planificacion
void lanzarHilosPlanificacion();
pthread_t hiloPLP;
pthread_t hiloPCP;

//Planificar Corto Plazo
void planificarCortoPlazo();
void enviarPcbCPU(pcb_t* pcb, int socketCPU);

//Planificacion Largo Plazo
void planificarLargoPlazo();
void alertarConsolaProcesoAceptado(int* pid, int socketConsola);
void envioCodigoMemoria(char* codigo);

//estadisticas (para consola del kernel)
void crearInfoEstadistica(int pid);
void estadisticaAumentarRafaga(int pid);
void estadisticaAumentarSyscall(int pid);
void estadisticaAumentarOpPriviligiada(int pid);
void estadisticaAumentarAlocar(int pid);
void estadisticaAumentarLiberar(int pid);
void estadisticaCambiarEstado(int pid, uint8_t nuevoEstado);


//Variables Globales
t_config_kernel* config;
int socketConexionFS;
int socketConexionMemoria;
int max_pid;
int cantProcesosSistema;
int pagina_size;
t_log* logger;
sem_t sem_cola_ready;
sem_t sem_cola_new;
sem_t mutex_cola_ready;
sem_t mutex_cola_new;
sem_t sem_multi;
sem_t semCPUs;
t_list* listaCPUs;
t_list* listadoEstadistico;
bool planificacionActivada;
pthread_mutex_t lockPlanificacion;
pthread_cond_t lockCondicionPlanificacion;

fd_set master;
fd_set setConsolas;
fd_set setCPUs;
int socketEscuchaCPUs;
int socketEscuchaConsolas;
int max_fd;

//int PAG_SIZE;

//Colas procesos
t_queue *colaNew, *colaReady, *colaFinished;

#endif /* FUNCIONESKERNEL_H_ */
