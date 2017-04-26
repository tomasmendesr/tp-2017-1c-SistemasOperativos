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

	//Setteo las paginas ocupadas por la tabla como ya usadas
	for(i=0;i<cantEntradas;i++){
		((t_entrada_tabla*)memoria)[i].pag = 0;
		((t_entrada_tabla*)memoria)[i].pid = 0;
	}

	/*Imprimo el contenido de la memoria en un archivo dump
	FILE* memFile = fopen("memDump","w");
	for(i=0;i<memSize;i++){
		fputc(memoria[i],memFile);
	}
	fclose(memFile); ESTO SE DEBERIA BORRAR UNA VEZ QUE NOS ASEGUREMOS QUE FUNCIONA 10puntos*/

}

void requestHandlerKernel(int fd){

	void* paquete = NULL;
	int bytes;
	int tipo_mensaje;

	for(;;){
		bytes = recibir_info(fd, &paquete, &tipo_mensaje);
		if(bytes <= 0){
			log_error(logger, "Desconexion del kernel. Terminando...");
			close(fd);
			exit(1);
		}

		switch(tipo_mensaje){
		case INICIAR_PROGRAMA:
			iniciarPrograma(fd, (t_pedido_iniciar*)paquete);
			break;

		case FINALIZAR_PROGRAMA:
			finalizarPrograma((t_pedido_finalizar*)paquete);
			break;

		case ASIGNAR_PAGINAS:
//			asignarPaginas();
			break;

		default:
			log_warning(logger, "Mensaje Recibido Incorrecto");
		}

		free(paquete);
		paquete = NULL;
	}
}

void requestHandlerCpu(int fd){

	void* paquete;
	int tipo_mensaje;
	int bytes;

	for(;;){
		//Recibo mensajes de cpu y hago el switch
		bytes = recibir_info(fd, &paquete, &tipo_mensaje);
		if(bytes <= 0){
			log_error(logger, "Desconexion del Cpu. Terminando...");
			close(fd);
			exit(1);
		}

		switch(tipo_mensaje){
			case SOLICITUD_BYTES:
				solicitudBytes(fd, (t_pedido_memoria*)paquete);
				break;

			case GRABAR_BYTES:
				grabarBytes(fd, (t_pedido_memoria*)paquete);
				break;
			case OK:
				break;
			default:
				log_warning(logger, "Mensaje Recibido Incorrecto");
		}

		free(paquete);
		paquete = NULL;
	}
}

int iniciarPrograma(int fd, t_pedido_iniciar* pedido){

	if( asignarPaginas(pedido->pid,pedido->cant_pag) == -1){
		//No se puede, aviso a kernel que no hay lugar
		enviarRespuesta(fd, SIN_ESPACIO);
		log_warning(logger, "No hay espacio");
		return -1;
	}

	//Envio el ok al kernel y espero el resto del codigo
	enviarRespuesta(fd, OP_OK);

	char* codigo = NULL;
	int tipo;

	if( recibir_paquete(fd, &codigo, &tipo) <= 0 ){
		log_error(logger, "Error al recibir el codigo");
		free(codigo);
		return -1;
	}

	//Recibi el codigo ok. Copio el codigo en las paginas correspondientes
	int i;
	for(i=0;i<pedido->cant_pag;i++){
		memcpy(memoria + buscarFrame(pedido->pid,i)*frame_size,
				codigo + i*frame_size,
				frame_size);
	}

	return 0;
}

int finalizarPrograma(t_pedido_finalizar* pid){

	int i;
	for(i=0;i<cant_frames;i++){
		if(tabla_pag[i].pid == *pid){
			tabla_pag[i].pid = -1;
			tabla_pag[i].pag = -1;
		}
	}

	return 0;
}

/* Asigna la cantidad de paginas al proceso especifico
 * Devuelve 0 en caso de poder asignar las paginas
 * -1 en caso de no haber frames disponibles para realizar la operacion */
int asignarPaginas(int pid, int cantPag){

	if(framesLibres() < cantPag)
		return -1;

	int maxPag = 0;
	int i;

	//Saco la max pagina de un frame
	for(i=0;i<cant_frames;i++){
		if(tabla_pag[i].pid == pid && tabla_pag[i].pag > maxPag)
			maxPag = tabla_pag[i].pag;
	}

	//Asigno las paginas que me piden
	for(i=0;i<cantPag;i++){

		if(tabla_pag[i].pid == -1 || tabla_pag[i].pag == -1){
			tabla_pag[i].pid = pid;
			tabla_pag[i].pag = maxPag;
			maxPag++;
		}
	}

	return 0;
}

int solicitudBytes(int fd, t_pedido_memoria* pedido){

	if(pedidoIncorrecto(pedido)){
		enviarRespuesta(fd, SEGMENTATION_FAULT);
		return -1;
	}

	char* buf = malloc(pedido->size);

	if(buf == NULL){
		log_error(logger,"no pude reservar memoria");
		enviarRespuesta(fd, QUILOMBO);
		return -1;
	}

	if( leer(pedido->pid,pedido->pag,pedido->offset,pedido->size,buf) == 0 ){
		header_t header;
		header.type = RESPUESTA_BYTES;
		header.length = pedido->size;

		if( sendSocket(fd, &header, buf) <= 0 ){
			log_error(logger,"error al enviar respuesta");
			enviarRespuesta(fd, SEGMENTATION_FAULT);
			free(buf);
			return -1;
		}
	}else{
		log_error(logger,"No se pudo leer de memoria");
		enviarRespuesta(fd, SEGMENTATION_FAULT);
		free(buf);
		return -1;
	}

	free(buf);
	return 0;
}

int grabarBytes(int fd, t_pedido_memoria* pedido){

	if(pedidoIncorrecto(pedido)){
		enviarRespuesta(fd, SEGMENTATION_FAULT);
		return -1;
	}

	char* buf = malloc(pedido->size);

	if(buf == NULL){
		log_error(logger,"no pude reservar memoria");
		enviarRespuesta(fd, QUILOMBO);
		return -1;
	}

	if( recvAll(fd,buf,pedido->size,0) <= 0 ){
		log_error(logger, "Error al recibir info a escribir");
		free(buf);
		enviarRespuesta(fd, SEGMENTATION_FAULT);
		return -1;
	}

	if( escribir(pedido->pid,pedido->pag,pedido->offset,buf,pedido->size) == -1){
		log_error(logger, "Error al intentar escribir en memoria");
		free(buf);
		enviarRespuesta(fd, SEGMENTATION_FAULT);
		return -1;
	}

	free(buf);
	enviarRespuesta(fd, OP_OK);
	return 0;
}

void enviarRespuesta(int fd, int respuesta){

	header_t header;
	header.type = respuesta;
	header.length = 0;
	sendSocket(fd, &header, &header);
}

bool pedidoIncorrecto(t_pedido_memoria* pedido){
	return pedido->offset > frame_size;
}

int framesLibres(){

	int i, cant = 0;
	for(i=0;i<cant_frames;i++){
		if( tabla_pag[i].pid == -1 &&
			tabla_pag[i].pag == -1  )
			cant++;
	}
	return cant;
}

/* Busqueda secuencial, despues implementamos hash */
int buscarFrame(int pid, int pag){

	int i;
	for(i=0;i<config->marcos;i++){
		if( tabla_pag[i].pid == pid &&
			tabla_pag[i].pag == pag  )
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
	char* pos_leer;

	while(cant_leida < size){

		if( leerCache(pid,pag,&pos_leer) == -1 ){//No Esta en cache, debo leer de memoria
			frame = buscarFrame(pid,pag);
			if(frame == -1)
				return -1;

			pos_leer = memoria + frame * frame_size;

			actualizarEntradaCache(pid, pag, pos_leer);
		}/* Al salir de este if pos_leer apunta o bien al frame de donde tengo que leer,
		  * o a donde esta cacheado el frame */

		//Me fijo cuanto tengo que leer y copio lo que esta en memoria/cache en resultado
		cant_a_leer = min(size - cant_leida, frame_size - offset);
		memcpy(resultado + cant_leida, pos_leer + offset, cant_a_leer);

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

		actualizarEntradaCache(pid, pag, memoria + frame * frame_size);

		offset = 0;
		pag++;
	}

	return 0;
}

//Funciones Cache
void increaseOpCount(){
	op_count++;
}
int cantEntradas(int pid){
	int i, cant = 0;
	for(i=0;i<cache_entradas;i++){
		if(cache[i].pid == pid)
			cant++;
	}

	return cant;
}

bool buscarEntrada(int pid, int pag){
	int i;
	for(i=0;i<cache_entradas;i++){
		if(cache[i].pid == pid && cache[i].pag == pag){
			return i;
		}
	}
	return -1;
}

int entradaAReemplazar(int pid){

	if( cantEntradas(pid) == max_entradas ){
		return reemplazoLocal(pid);
	}else return reemplazoGlobal();
}

int reemplazoLocal(int pid){
	int i;
	int entrada; //A reemplazar
	int minTime = ULONG_MAX;

	for(i=0;i<cache_entradas;i++){
		if( cache[i].time_used < minTime && cache[i].pid == pid){
			entrada = i;
			minTime = cache[i].time_used;
		}
	}

	return entrada;
}

int reemplazoGlobal(){
	//Recorro buscando una entrada libre
	int i;
	for(i=0;i<cache_entradas;i++){
		if(cache[i].pid == -1 || cache[i].pag -1){
			return i;
		}
	}

	//Sali del for => no hay entrada libre. Debo reemplazar
	int entrada; //A reemplazar
	int minTime = ULONG_MAX;

	for(i=0;i<cache_entradas;i++){
		if( cache[i].time_used < minTime){
			entrada = i;
			minTime = cache[i].time_used;
		}
	}

	return entrada;
}

int leerCache(int pid, int pag, char** contenido){

	increaseOpCount();

	int i;
	for(i=0;i<cache_entradas;i++){//Si alguna entrada coincide, pongo el valor de content en contenido
		if(cache[i].pid == pid && cache[i].pag == pag){
			*contenido = cache[i].content;
			cache[i].time_used = op_count;
			return 0;
		}
	}
	return -1;
}

void actualizarEntradaCache(int pid, int pag, char* frame){

	increaseOpCount();

	int entrada = buscarEntrada(pid, pag);

	if(entrada != -1){
		memcpy(cache[entrada].content,frame,frame_size);
	}else{ //tengo que reemplazar una entrada
		entrada = entradaAReemplazar(pid);
		cache[entrada].pid = pid;
		cache[entrada].pag = pag;
		memcpy(cache[entrada].content,frame,frame_size);
	}

	cache[entrada].time_used = op_count;

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

	if(strlen(param) == 0){
		//Dump de todas las estructuras
		dumpAll();
	}

	if( !strcmp(param, "cache") ){
		//Dump de cache
		dumpCache();
	}

	if( !strcmp(param,"tabla") ){
		//Dump de tabla de paginas
		dumpTable();
	}

	if( !strcmp(param,"memory") ){
		//Dump de contenido de memoria
		int pid;
		dumpMemory(pid);
	}
}
void dumpAll(){}
void dumpCache(){

	FILE* dumpFile = fopen("cacheDump","a");

	if(dumpFile == NULL){
		log_error(logger, "No se pudo abrir el archivo de dump");
		return;
	}

	//Escribo el header del dump
	fprintf(dumpFile,"\n----Dump de cache: %s----\n",getTimeStamp());
	fprintf(dumpFile,"Tiempo actual: %lu\n", op_count);

	//Escribo el contenido de cada entrada
	int i,j;
	for(i=0;i<cache_entradas;i++){
		fprintf(dumpFile,"entrada n°: %i, pid: %i, pag: %i, time_used: %lu\ncontenido: ",
				cache[i].pid,cache[i].pag,cache[i].time_used);

		for(j=0;j<frame_size;j++)
			fputc(cache[i].content[j],dumpFile);
		fputc("\n",dumpFile);
	}

	fclose(dumpFile);
	return;
}
void dumpTable(){}
void dumpMemory(int pid){}
void flush(char* comando, char* param){
        printf("flush\n");
}
void size(char* comando, char* param){
        printf("size\n");
}

char* getTimeStamp(){
	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	return asctime(timeinfo);
}
