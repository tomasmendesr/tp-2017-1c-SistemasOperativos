#include "funcionesMemoria.h"

void crearConfig(int argc, char* argv[]){
	char* pathConfig=string_new();

	if(argc>1){
		string_append(&pathConfig,argv[1]);
	}
		else string_append(&pathConfig,configuracionMemoria);
	if(verificarExistenciaDeArchivo(pathConfig)){
		config = levantarConfiguracionMemoria(pathConfig);
	}else{
		printf("No se pudo levantar archivo de configuracion\n");
		exit(EXIT_FAILURE);
	}

    printf("soy la memoria\n");
}
t_config_memoria* levantarConfiguracionMemoria(char* archivo) {

        t_config_memoria* config = malloc(sizeof(t_config_memoria));
        t_config* configMemoria;

        configMemoria = config_create(archivo);

        config->puerto = malloc(MAX_LEN_PUERTO);
        strcpy(config->puerto, config_get_string_value(configMemoria, "PUERTO"));

        config->marcos = config_get_int_value(configMemoria, "MARCOS");
        config->marcos_Size = config_get_int_value(configMemoria, "MARCOS_SIZE");
        config->entradas_Cache = config_get_int_value(configMemoria, "ENTRADAS_CACHE");
        config->entradas_Cache = config_get_int_value(configMemoria, "CACHE_X_PROC");

        config->reemplazo_cache = malloc(MAX_LEN_PUERTO);
        strcpy(config->reemplazo_cache, config_get_string_value(configMemoria, "REEMPLAZO_CACHE"));

        config->retardo_Memoria = config_get_int_value(configMemoria, "RETARDO_MEMORIA");

        config_destroy(configMemoria);
        return config;

}

void destruirConfiguracionMemoria(t_config_memoria* config){
	free(config->puerto);
	free(config->reemplazo_cache);
	free(config);
}

void inicializarMemoria(){

	const int memSize = config->marcos * config->marcos_Size;

	//Creo el bloque de memoria principal
	memoria = malloc(memSize);

	//Creo la cache
	cache = malloc(sizeof(t_entrada_cache) * config->entradas_Cache);

	//Reviso los mallocs
	if(memoria == NULL || cache == NULL){
		log_error(log, "No pude reservar memoria para cache y/o memoria");
		exit(EXIT_FAILURE);
	}

	//Setteo la memoria con \0
	memset(memoria,'\0',memSize);

	//Creo las entradas de la tabla invertida
	int i;
	for(i=0;i<config->marcos * 2;i++){//*2 porque son 2 ints, pid y nroPag
		((int*)memoria)[i] = -1;
	} //Esto hay que revisar que funcione correctamente

	//Imprimo el contenido de la memoria
	FILE* memFile = fopen("memDump","w");
	for(i=0;i<memSize;i++){
		fputc(memoria[i],memFile);
		fputc(memoria[i],stdout);
	}
	fclose(memFile);
}

//funciones interfaz
void levantarInterfaz(){
	//creo los comandos y el parametro
	comando* comandos = malloc(sizeof(comando)*4);

	strcpy(comandos[0].comando, "retardo");
	comandos[0].funcion = retardo;
	strcpy(comandos[1].comando, "dump");
	comandos[1].funcion = dump;
	strcpy(comandos[2].comando, "flush");
	comandos[2].funcion = flush;
	strcpy(comandos[3].comando, "size");
	comandos[3].funcion = size;

	interface_thread_param* params = malloc(sizeof(interface_thread_param));
	params->comandos = comandos;
	params->cantComandos = 4;

	//Lanzo el thread
	pthread_t threadInterfaz;
	pthread_attr_t atributos;
	pthread_attr_init(&atributos);
	pthread_attr_setdetachstate(&atributos, PTHREAD_CREATE_DETACHED);

	pthread_create(&threadInterfaz, &atributos, (void*)interface, params);

	return;
}
void retardo(char* comando, char* param){
        printf("retardo\n");
}
void dump(char* comando, char* param){
        printf("dump\n");
}
void flush(char* comando, char* param){
        printf("flush\n");
}
void size(char* comando, char* param){
        printf("size\n");
}

