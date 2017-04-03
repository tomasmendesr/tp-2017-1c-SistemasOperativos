#include "funcionesKernel.h"

t_config_kernel* levantarConfiguracionKernel(char* archivo_conf) {

	t_config_kernel* conf = malloc(sizeof(t_config_kernel));
	t_config* configNucleo;
	char** varGlob, semID, semInit;

	configNucleo = config_create(archivo_conf);
	conf->puerto_CPU = config_get_int_value(configNucleo, "PUERTO_CPU");
	conf->puerto_PROG = config_get_int_value(configNucleo, "PUERTO_PROG");
	conf->ip_Memoria = config_get_string_value(configNucleo, "IP_MEMORIA");
	conf->puerto_Memoria = config_get_int_value(configNucleo, "PUERTO_MEMORIA");
	conf->ip_FS = config_get_string_value(configNucleo, "IP_FS");
	conf->puerto_FS = config_get_int_value(configNucleo, "PUERTO_FS");
	conf->quantum = config_get_int_value(configNucleo, "QUANTUM");
	conf->quantum_Sleep = config_get_int_value(configNucleo, "QUANTUM_SLEEP");
	conf->algoritmo = config_get_string_value(configNucleo, "ALGORITMO");
	conf->grado_MultiProg = config_get_int_value(configNucleo, "GRADO_MULTIPROG");

	semID = config_get_array_value(configNucleo, "SEM_ID");
	semInit = config_get_array_value(configNucleo, "SEM_INIT");
	conf->semaforos = crearDiccionarioConValue(semID,semInit);

	varGlob = config_get_array_value(configNucleo, "SHARED_VARS");
	conf->variablesGlobales = crearDiccionario(varGlob);

	conf->stack_Size = config_get_int_value(configNucleo, "STACK_SIZE");


	return conf;
}

t_dictionary* crearDiccionarioConValue(char** array, char** valores){

	t_dictionary* dic = malloc(sizeof(t_dictionary));
	char** auxArray = array;
	char** auxValores = valores;

	while(auxArray != NULL){
		dictionary_put(dic, auxArray, auxValores);
		auxArray++;
		auxValores++;
	}

	return dic;
}

t_dictionary* crearDiccionario(char** array){

	t_dictionary* dic = malloc(sizeof(t_dictionary));
	char** auxArray = array;

	while(auxArray != NULL){
		dictionary_put(dic, auxArray, 0);
		auxArray++;
	}

	return dic;
}
