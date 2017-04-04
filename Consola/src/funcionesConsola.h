
#include <commons/config.h>
#include "interface.h"

#ifndef SRC_FUNCIONESCONSOLA_H_
#define SRC_FUNCIONESCONSOLA_H_


typedef struct{
	char* ip_Kernel;
	int puerto_Kernel;

}t_config_consola;

t_config_consola* levantarConfiguracionConsola(char * archivo);


//Funciones de interfaz
void levantarInterfaz();
void iniciarPrograma(char* comando, char* param);
void finalizarPrograma(char* comando, char* param);
void desconectarConsola(char* comando, char* param);
void limpiarMensajes(char* comando, char* param);

#endif /* SRC_FUNCIONESCONSOLA_H_ */
