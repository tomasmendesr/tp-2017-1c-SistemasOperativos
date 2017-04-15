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
		log_info(logger,"No se pudo levantar archivo de configuracion\n");
		exit(EXIT_FAILURE);
	}
	log_info(logger,"Se levanto la configuracion correctamente\n");
	printf("Se levanto la configuracion correctamente\n");

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
	int i;
	bool noEspacioCache = false;

	//Creo el bloque de memoria principal
	memoria = malloc(memSize);

	//Creo la cache
	cache = malloc(sizeof(t_entrada_cache) * config->entradas_Cache);

	//Inicializo la cache
	for(i=0;i<config->entradas_Cache;i++){
		cache[i].pid = -1;
		cache[i].pag = -1;
		cache[i].content = malloc(frame_size);
		cache[i].time_used = 0;
		if(cache[i].content == NULL) noEspacioCache = true;
	}

	//Reviso los mallocs
	if(memoria == NULL || cache == NULL || noEspacioCache){
		log_error(logger, "No pude reservar memoria para cache y/o memoria");
		exit(EXIT_FAILURE);
	}

	//Setteo la memoria con \0
	memset(memoria,'\0',memSize);

	//Creo las entradas de la tabla invertida
	for(i=0;i<config->marcos;i++){
		((t_entrada_tabla*)memoria)[i].pag = -1;
		((t_entrada_tabla*)memoria)[i].pid = -1;
	}

	int cantEntradas = (sizeof(t_entrada_tabla) * config->marcos) / config->marcos_Size;
	if( ((sizeof(t_entrada_tabla) * config->marcos) % config->marcos_Size) > 0)
		cantEntradas++;

	printf("\n--------------------\ncantidad de frames ocupados por tabla: %i\n", cantEntradas);

	/*Imprimo el contenido de la memoria en un archivo dump
	FILE* memFile = fopen("memDump","w");
	for(i=0;i<memSize;i++){
		fputc(memoria[i],memFile);
	}
	fclose(memFile); ESTO SE DEBERIA BORRAR UNA VEZ QUE NOS ASEGUREMOS QUE FUNCIONA 10puntos*/

}

void requestHandlerKernel(int* fd){

	void* paquete;
	int bytes;
	int tipo_mensaje;

	for(;;){
		bytes = recibir_info(*fd, &paquete, &tipo_mensaje);
		if(bytes < 0){
			log_error(logger, "Desconexion del kernel. Terminando...");
			close(*fd);
			exit(1);
		}

		switch(tipo_mensaje){
		case INICIAR_PROGRAMA:
			iniciarPrograma();
			break;

		case FINALIZAR_PROGRAMA:
			finalizarPrograma();
			break;

		case ASIGNAR_PAGINAS:
			asignarPaginas();
			break;

		default:
			log_warning(logger, "Mensaje Recibido Incorrecto");
		}
	}
}

void requestHandlerCpu(int* fd){

	void* paquete;
	int tipo_mensaje;
	int bytes;

	for(;;){
		//Recibo mensajes de cpu y hago el switch
		bytes = recibir_info(*fd, &paquete, &tipo_mensaje);
		if(bytes <= 0){
			log_error(logger, "Desconexion del Cpu. Terminando...");
			close(*fd);
			exit(1);
		}

		switch(tipo_mensaje){

			case SOLICITUD_BYTES:
				solicitudBytes();
				break;

			case GRABAR_BYTES:
				grabarBytes();
				break;

			default:
				log_warning(logger, "Mensaje Recibido Incorrecto");
		}
	}
}

int iniciarPrograma(int pid, int cantPag){

	return 0;
}

int asignarPaginas(int pid, int cantPag){

	return 0;
}

int finalizarPrograma(int pid){

	return 0;
}

int solicitudBytes(int pid, int pag, int offset, int size, void* buff){

	return 0;
}

int grabarBytes(int pid, int pag, int offset, int size, void* buff){

	return 0;
}

int framesLibres(){

	int i, cant = 0;
	for(i=0;i<cant_frames;i++){
		if( ((t_entrada_tabla*)memoria)[i].pid == -1 &&
			((t_entrada_tabla*)memoria)[i].pag == -1  )
			cant++;
	}
	return cant;
}


/* Busqueda secuencial, despues implementamos hash */
int buscarFrame(int pid, int pag){

	int i;
	for(i=0;i<config->marcos;i++){
		if( ((t_entrada_tabla*)memoria)[i].pid == pid &&
			((t_entrada_tabla*)memoria)[i].pag == pag  )
			return i;
	}
	return -1; //No encontro en la tabla de paginas la entrada
}

/* Esta funcion necesita que le pasen en resultado, un puntero con al menos
 * al menos size bytes reservados. El invocador debe hacer el free despues de utilizado.
 */
int leer(int pid, int pag, int offset, int size, char* resultado){

	int frame;
	int cant_leida = 0;
	int cant_a_leer;

	while(cant_leida < size){

		frame = buscarFrame(pid,pag);
		if(frame == -1)
			return -1;

		//Me fijo cuanto tengo que leer y copio lo que esta en memoria en resultado
		cant_a_leer = min(size - cant_leida, frame_size - offset);
		memcpy(resultado + cant_leida, memoria + frame * frame_size + offset, cant_a_leer);

		offset = 0;
		pag++;
	}

	return 0;
}

int escribir(int pid, int pag, int offset, char* contenido, int size){

	int frame;
	int cant_escrita = 0;
	int cant_a_escribir;

	while(cant_escrita < size){

		frame = buscarFrame(pid,pag);
		if(frame == -1)
			return -1;

		cant_a_escribir = min(size - cant_escrita, frame_size - offset);
		memcpy(memoria + frame * frame_size + offset, contenido + cant_escrita, cant_a_escribir);

		offset = 0;
		pag++;
	}

	return 0;
}

//Funciones Cache
bool hayEntradasLibres(){
	int i;
	for(i=0;i<cache_entradas;i++){
		if(cache[i].pag == -1 || cache[i].pid == -1){
			return true;
		}
	}
	return false;
}

bool existeEntrada(int pid, int pag){
	int i;
	for(i=0;i<cache_entradas;i++){
		if(cache[i].pid == pid && cache[i].pag == pag){
			return true;
		}
	}
	return false;
}

int entradaAReemplazar(){

	//Recorro buscando una entrada libre
	int i;
	for(i=0;i<cache_entradas;i++){
		if(cache[i].pid == -1 || cache[i].pag -1){
			return i;
		}
	}

	//Sali del for => no hay entrada libre. Debo reemplazar
	int entrada; //A reemplazar
	int minTime = LONG_MAX;

	for(i=0;i<cache_entradas;i++){
		if( cache[i].time_used < minTime){
			entrada = i;
			minTime = cache[i].time_used;
		}
	}

	return entrada;
}

int leerCache(int pid, int pag, char** contenido){

	int i;
	for(i=0;i<cache_entradas;i++){//Si alguna entrada coincide, pongo el valor de content en contenido
		if(cache[i].pid == pid && cache[i].pag == pag){
			*contenido = cache[i].content;
			return 0;
		}
	}
	return -1;
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

