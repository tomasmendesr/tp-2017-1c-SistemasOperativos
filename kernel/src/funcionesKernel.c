#include "funcionesKernel.h"

void crearConfig(int argc, char* argv[]){
	char* pathConfig=string_new();

	if(argc>1)string_append(&pathConfig,argv[1]);
		else string_append(&pathConfig,configuracionKernel);
	if(verificarExistenciaDeArchivo(pathConfig)){
		config = levantarConfiguracionKernel(pathConfig);
	}else{
		printf("No se pudo levantar archivo de configuracion");
		exit(EXIT_FAILURE);
	}

	printf("Configuracion levantada correctamente\n");
	return;
}

t_config_kernel* levantarConfiguracionKernel(char* archivo_conf) {

        t_config_kernel* conf = malloc(sizeof(t_config_kernel));
        t_config* configNucleo;
        char** varGlob, **semID, **semInit;

        configNucleo = config_create(archivo_conf);

        conf->puerto_CPU = malloc(MAX_LEN_PUERTO);
        strcpy(conf->puerto_CPU, config_get_string_value(configNucleo, "PUERTO_CPU"));

        conf->puerto_PROG = malloc(MAX_LEN_PUERTO);
        strcpy(conf->puerto_PROG, config_get_string_value(configNucleo, "PUERTO_PROG"));

        conf->ip_Memoria = malloc(MAX_LEN_IP);
        strcpy(conf->ip_Memoria, config_get_string_value(configNucleo, "IP_MEMORIA"));

        conf->puerto_Memoria = malloc(MAX_LEN_PUERTO);
        strcpy(conf->puerto_Memoria, config_get_string_value(configNucleo, "PUERTO_MEMORIA"));

        conf->ip_FS = malloc(MAX_LEN_IP);
        strcpy(conf->ip_FS, config_get_string_value(configNucleo, "IP_FS"));

        conf->puerto_FS = malloc(MAX_LEN_PUERTO);
        strcpy(conf->puerto_FS, config_get_string_value(configNucleo, "PUERTO_FS"));

        conf->quantum = config_get_int_value(configNucleo, "QUANTUM");
        conf->quantum_Sleep = config_get_int_value(configNucleo, "QUANTUM_SLEEP");

        conf->algoritmo = malloc(MAX_LEN_PUERTO);
        strcpy(conf->algoritmo, config_get_string_value(configNucleo, "ALGORITMO"));

        conf->grado_MultiProg = config_get_int_value(configNucleo, "GRADO_MULTIPROG");

        semID = config_get_array_value(configNucleo, "SEM_ID");
        semInit = config_get_array_value(configNucleo, "SEM_INIT");
        conf->semaforos = crearDiccionarioConValue(semID,semInit);

        varGlob = config_get_array_value(configNucleo, "SHARED_VARS");
        conf->variablesGlobales = crearDiccionario(varGlob);

        conf->stack_Size = config_get_int_value(configNucleo, "STACK_SIZE");

        config_destroy(configNucleo);
        return conf;
}

void destruirConfiguracionKernel(t_config_kernel* config){
	free(config->algoritmo);
	free(config->ip_FS);
	free(config->ip_Memoria);
	free(config->puerto_CPU);
	free(config->puerto_FS);
	free(config->puerto_Memoria);
	free(config->puerto_PROG);
	free(config);
}

t_dictionary* crearDiccionarioConValue(char** array, char** valores){

        t_dictionary* dic = dictionary_create();
        int j = 0;

        while(array[j] != NULL){
                dictionary_put(dic, array[j], valores);
                j++;
        }

        return dic;
}

t_dictionary* crearDiccionario(char** array){

        t_dictionary* dic = dictionary_create();
        int j = 0;

        while(array[j] != NULL){
                dictionary_put(dic, array[j], 0);
                j++;
        }

        return dic;
}

void establecerConexiones(){
	//TODO: todo.
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
