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
		//cache[i].pag = -1; no alcanza con el pid?
		cache[i].content = malloc(config->marcos_Size);
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
//		((t_entrada_tabla*)memoria)[i].pag = -1; con el pid no alcanza?
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
//	header_t header;
	int bytes;
	int resultado;
	int tipo_mensaje;
	t_operacion_pag* st_pag;

	for(;;){
		bytes = recibir_info(*fd, &paquete, &tipo_mensaje);
		if(bytes < 0){
			log_error(logger, "Desconexion del kernel. Terminando...");
			close(*fd);
			exit(1);
		}

		switch(tipo_mensaje){
		case INICIAR_PROGRAMA:
			st_pag=paquete;
			if((resultado = iniciarPrograma(st_pag->pid ,st_pag->cantPag))==-1){
				log_info(logger, "No se pudo iniciar el programa en memoria");
				enviar_paquete_vacio(MEMORY_OVERLOAD,*fd);
				free(paquete);
			}
			else{
				/*enviar confirmacion*/
				free(paquete);
			}
			break;

		case FINALIZAR_PROGRAMA:

			if((resultado=finalizarPrograma(*(int*)paquete))==-1){
				log_info(logger, "El programa no finalizo correctamente");
				free(paquete);
			}
			else{
				/*Enviar confirmacion*/
				free(paquete);
			}
			break;

		case ASIGNAR_PAGINAS:
			st_pag=paquete;
			if((resultado=asignarPaginas(st_pag->pid, st_pag->cantPag))==-1){
				enviar_paquete_vacio(MEMORY_OVERLOAD,*fd);
				free(paquete);
			}
			else{
				/*Enviar confirmacion*/
				free(paquete);
			}
			break;

		default:
			log_warning(logger, "Mensaje Recibido Incorrecto");
		}
	}
}

void requestHandlerCpu(int* fd){

	void* paquete;
//	header_t header;
	int tipo_mensaje;
	int bytes;
	int resultado;
	void* buff;
	t_operacion_bytes* st_bytes;

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
			st_bytes=paquete;
			buff=malloc(st_bytes->size);
			resultado=solicitudBytes(st_bytes->pid, st_bytes->pag, st_bytes->offset, st_bytes->size, buff);
			if(resultado==-1){
				/* actualizar protocolo */
				enviar_paquete_vacio(SEGMENTATION_FAULT,*fd);
				free(buff);
				free(paquete);
			}
			else{
				enviar_info(*fd,RESPUESTA_BYTES,st_bytes->size,buff);
				free(buff);
				free(paquete);
			}
			break;

			case GRABAR_BYTES:
			st_bytes=paquete;
			bytes = recibir_info(*fd, &paquete, &tipo_mensaje);
			if(bytes < 0){
				log_error(logger, "Desconexion del Cpu. Terminando...");
				close(*fd);
				exit(1);
			}
			resultado=grabarBytes(st_bytes->pid, st_bytes->pag, st_bytes->offset, st_bytes->size, paquete);
			if(resultado==-1){
				/* actualizar protocolo */
				enviar_paquete_vacio(SEGMENTATION_FAULT,*fd);
				free(buff);
				free(paquete);
				free(st_bytes);
			}
			else{
				/* no se bien que significa respuesta_bytes */
				enviar_info(*fd,RESPUESTA_BYTES,st_bytes->size,buff);
				free(buff);
				free(paquete);
				free(st_bytes);
			}
			break;

		default:
			log_warning(logger, "Mensaje Recibido Incorrecto");
		}
	}
}

int iniciarPrograma(int pid, int cantPag){

	int i, frame;
	for(i=0;i<cantPag;i++){
		if((frame = primerFrameLibre()) == -1)
		 return -1;
		((t_entrada_tabla*)memoria)[frame].pid=pid;
		((t_entrada_tabla*)memoria)[frame].pag=i;
	}
	return EXIT_SUCCESS;
}

int asignarPaginas(int pid, int cantPag){
/* buscar la manera de distinguirlas */

	int i, frame;
	for(i=0;i<cantPag;i++){
		if((frame = primerFrameLibre()) == -1)
		 return -1;
		((t_entrada_tabla*)memoria)[frame].pid=pid;
		((t_entrada_tabla*)memoria)[frame].pag=i;
	}
	return EXIT_SUCCESS;
}

int finalizarPrograma(int pid){
	/*entre otras cosas elimina las entradas en la tabla invertida*/
	int frame = buscarPaginas(pid,0);
	while(frame!=-1){
		((t_entrada_tabla*)memoria)[frame].pid = -1;
		frame=buscarPaginas(pid,frame);
	}
	return EXIT_SUCCESS;
}

int solicitudBytes(int pid, int pag, int offset, int size, void* buff){

	int frameMemoria,frameCache;
	frameCache=buscarPagCache(pid,pag);
	if(frameCache>=0)
	memcpy(buff,cache[frameCache].content+offset,size);
	else{
		frameMemoria=buscarFrame(pid,pag);
		if(framesLibresCache()>0){
			frameCache=primerFrameLibreCache();
			cache[frameCache].pid=pid;
			cache[frameCache].pag=pag;
			memcpy((void*)cache[frameCache].content,memoria+frameMemoria*config->marcos_Size,config->marcos_Size);
		}
		else{
			//aplicar algoritmo LRU para realizar el reemplazo
		}
		memcpy(buff,cache[frameCache].content+offset,size);
	}
	return size;
}

int grabarBytes(int pid, int pag, int offset, int size, void* buff){

	int frameMemoria,frameCache;
	frameCache=buscarPagCache(pid,pag);
	if(frameCache>=0){
		memcpy(cache[frameCache].content+offset,buff,size);
		frameMemoria=buscarFrame(pid,pag);
		memcpy(memoria+frameMemoria*config->marcos_Size+offset,buff,size);
	}
	else{
		if((frameMemoria=buscarFrame(pid,pag))>=0)
			memcpy(memoria+frameMemoria*config->marcos_Size+offset,buff,size);
		else
			return -1;
	}
	return size;
}

int primerFrameLibre(){

	int i;
	for(i=0;i<config->marcos;i++){
		if( ((t_entrada_tabla*)memoria)[i].pid == -1)
			return i;
	}
	return -1;
}

int primerFrameLibreCache(){

	int i;
	for(i=0;i<config->marcos;i++){
		if( cache[i].pid == -1)
			return i;
	}
	return -1;
}

/* creo que no hace no hace falta darle valor negativo a las paginas */
int framesLibres(){

	int i, cant = 0;
	for(i=0;i<config->marcos;i++){
		if( ((t_entrada_tabla*)memoria)[i].pid == -1 &&
			((t_entrada_tabla*)memoria)[i].pag == -1  )
			cant++;
	}
	return cant;
}

/*lo mismo que la de arriba*/
int framesLibresCache(){

	int i, cant = 0;
	for(i=0;i<config->entradas_Cache;i++){
		if( cache[i].pid == -1 &&
			cache[i].pag == -1  )
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

int buscarPaginas(int pid, int frame){

	int i;
	for(i=frame;i<config->marcos;frame++){
		if(((t_entrada_tabla*)memoria)[frame].pid == pid)
			return i;
	}

	return -1;
}

int buscarPagCache(int pid, int pag){

	int i;
	for(i=0;i<config->entradas_Cache;i++){
		if( cache[i].pid == pid &&
			cache[i].pag == pag )
			return i;
	}
	return -1;
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

