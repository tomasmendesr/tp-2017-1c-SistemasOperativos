#include "funcionesKernel.h"



t_config_kernel* levantarConfiguracionKernel(char* archivo_conf) {

        t_config_kernel* conf = malloc(sizeof(t_config_kernel));
        t_config* configNucleo;
        char** varGlob, **semID, **semInit;

        configNucleo = config_create(archivo_conf);
        conf->puerto_CPU = config_get_string_value(configNucleo, "PUERTO_CPU");
        conf->puerto_PROG = config_get_string_value(configNucleo, "PUERTO_PROG");
        conf->ip_Memoria = config_get_string_value(configNucleo, "IP_MEMORIA");
        conf->puerto_Memoria = config_get_string_value(configNucleo, "PUERTO_MEMORIA");
        conf->ip_FS = config_get_string_value(configNucleo, "IP_FS");
        conf->puerto_FS = config_get_string_value(configNucleo, "PUERTO_FS");
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

        t_dictionary* dic = dictionary_create();
        int j = 0;

        while(array[j] != NULL){
                dictionary_put(dic, array, valores);
                j++;
        }

        return dic;
}

t_dictionary* crearDiccionario(char** array){

        t_dictionary* dic = dictionary_create();
        int j = 0;

        while(array[j] != NULL){
                dictionary_put(dic, array, 0);
                j++;
        }

        return dic;
}

//funciones interfaz
void levantarInterfaz(){
	//creo los comandos y el parametro
	comando* comandos = malloc(sizeof(comando)*6);

	strcpy(comandos[0].comando, "list");
	comandos[0].funcion = listProcesses;
	strcpy(comandos[1].comando, "info");
	comandos[1].funcion = processInfo;
	strcpy(comandos[2].comando, "tablaArchivos");
	comandos[2].funcion = getTablaArchivos;
	strcpy(comandos[3].comando, "grMulti");
	comandos[3].funcion = gradoMultiprogramacion;
	strcpy(comandos[4].comando, "kill");
	comandos[4].funcion = killProcess;
	strcpy(comandos[5].comando, "stopPlan");
	comandos[5].funcion = stopPlanification;

	interface_thread_param* params = malloc(sizeof(interface_thread_param));
	params->comandos = comandos;
	params->cantComandos = 6;

	//Lanzo el thread
	pthread_t threadInterfaz;
	pthread_attr_t atributos;
	pthread_attr_init(&atributos);
	pthread_attr_setdetachstate(&atributos, PTHREAD_CREATE_DETACHED);

	pthread_create(&threadInterfaz, &atributos, interface, params);

	return;
}
void listProcesses(char* comando, char* param){
        printf("listProcesses\n");
}
void processInfo(char* comando, char* param){
        printf("process info\n");
}
void getTablaArchivos(char* comando, char* param){
        printf("get tabla archivos\n");
}
void gradoMultiprogramacion(char* comando, char* param){
        printf("gradoMultiprogramacion\n");
}
void killProcess(char* comando, char* param){
        printf("killProcess\n");
}
void stopPlanification(char* comando, char* param){
        printf("stopPlanification\n");
}