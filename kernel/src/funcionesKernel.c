#include "funcionesKernel.h"

void inicializarColas(){

	colaNew = queue_create();
	colaReady = queue_create();
	colaFinished = queue_create();

	max_pid = 0;
}

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
        t_config* configKernel;
        char** varGlob, **semID, **semInit;

        configKernel = config_create(archivo_conf);

        conf->puerto_CPU = malloc(MAX_LEN_PUERTO);
        strcpy(conf->puerto_CPU, config_get_string_value(configKernel, "PUERTO_CPU"));

        conf->puerto_PROG = malloc(MAX_LEN_PUERTO);
        strcpy(conf->puerto_PROG, config_get_string_value(configKernel, "PUERTO_PROG"));

        conf->ip_Memoria = malloc(MAX_LEN_IP);
        strcpy(conf->ip_Memoria, config_get_string_value(configKernel, "IP_MEMORIA"));

        conf->puerto_Memoria = malloc(MAX_LEN_PUERTO);
        strcpy(conf->puerto_Memoria, config_get_string_value(configKernel, "PUERTO_MEMORIA"));

        conf->ip_FS = malloc(MAX_LEN_IP);
        strcpy(conf->ip_FS, config_get_string_value(configKernel, "IP_FS"));

        conf->puerto_FS = malloc(MAX_LEN_PUERTO);
        strcpy(conf->puerto_FS, config_get_string_value(configKernel, "PUERTO_FS"));

        conf->quantum = config_get_int_value(configKernel, "QUANTUM");
        conf->quantum_Sleep = config_get_int_value(configKernel, "QUANTUM_SLEEP");

        conf->algoritmo = malloc(MAX_LEN_PUERTO);
        strcpy(conf->algoritmo, config_get_string_value(configKernel, "ALGORITMO"));

        conf->grado_MultiProg = config_get_int_value(configKernel, "GRADO_MULTIPROG");

        semID = config_get_array_value(configKernel, "SEM_ID");
        semInit = config_get_array_value(configKernel, "SEM_INIT");
        conf->semaforos = crearDiccionarioConValue(semID,semInit);
        crearColasBloqueados(semID);

        varGlob = config_get_array_value(configKernel, "SHARED_VARS");
        conf->variablesGlobales = crearDiccionario(varGlob);

        conf->stack_Size = config_get_int_value(configKernel, "STACK_SIZE");


        config_destroy(configKernel);
        return conf;
}

void enviarTamanioStack(int fd){
	header_t* header=malloc(sizeof(header_t));
	header->type=TAMANIO_STACK_PARA_CPU;
	header->length=sizeof(config->stack_Size);
	sendSocket(fd,header,&config->stack_Size);
}

proceso_en_espera_t* crearProcesoEnEspera(int consola_fd, char* source){

	proceso_en_espera_t* proc = malloc(sizeof(proceso_en_espera_t));
	proc->socketConsola = consola_fd;
	proc->codigo = malloc(strlen(source));
	memcpy(proc->codigo, source, strlen(source));
	proc->pid = asignarPid();
	//pcb_t* pcb = crearPCB(source, asignarPid() );
	//pcb->consolaFd = consola_fd;

	return proc;
}

int asignarPid(){
	max_pid++;
	return max_pid;
}

int conexionConFileSystem(){

	socketConexionFS = createClient(config->ip_FS, config->puerto_FS);

	if(socketConexionFS == -1){
		return -1;
	}

//	void* paquete;
//	int tipo_mensaje;

	enviar_paquete_vacio(HANDSHAKE_KERNEL,socketConexionFS);

	printf("Conexion con fs establecida\n");

	return 1;
}

int conexionConMemoria(){

	socketConexionMemoria = createClient(config->ip_Memoria, config->puerto_Memoria);

	if(socketConexionMemoria == -1){
		return -1;
	}

	enviar_paquete_vacio(HANDSHAKE_KERNEL,socketConexionMemoria);

	int respuesta;
	void* paquete;

	recibir_paquete(socketConexionMemoria, &paquete, &respuesta);
	if(respuesta != HANDSHAKE_MEMORIA){
		return -1;
	}

	recibir_paquete(socketConexionMemoria, &paquete, &respuesta);
	pagina_size = *(int*)paquete;
	printf("tamanio de pagina: %d\n", pagina_size);
	printf("Conexion con memoria establecida\n");

	return 1;
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
        	dictionary_put(dic, array[j], valores[j]);
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

/*
 * @NAME: LevantarInterfaz.
 * @DESC: Interfaz de Consola para manejar operaciones del kernel.
 */
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

	pthread_create(&threadInterfaz, &atributos, (void*)interface, params);

	return;
}

void modificarValorDiccionario(t_dictionary* dic, char* key, void* data){
	void* previo = dictionary_get(dic, key);
	previo = data;
}

int leerVariableGlobal(t_dictionary* dic, char* key){
	if(dictionary_has_key(dic, key)){
		int* valor = dictionary_get(dic, key);
		return valor;
	}
	return NULL;
}

void escribirVariableGlobal(t_dictionary* dic, char* key, void* nuevoValor){
	if(dictionary_has_key(dic, key)){
		dictionary_remove_and_destroy(dic, key, free);
		dictionary_put(dic, key, nuevoValor);
	}
}

int semaforoSignal(t_dictionary* dic, char* key){
	char* previo = dictionary_get(dic, key);
	*previo = *previo + 1;

	return atoi(previo);
}

int semaforoWait(t_dictionary* dic, char* key){
	char* previo = dictionary_get(dic, key);
	*previo = *previo - 1;

	return atoi(previo);
}

void listProcesses(char* comando, char* param){

	int estado;

	void listarProcesos(info_estadistica_t* info){
		int cant = 0;
		if(info->estado == estado){
			printf("Proceso pid: %d\n", info->pid);
			cant++;
		}
		if(cant == 0) printf("No existen procesos que se encuentren en ese estado\n");
	}

	void listarTodos(info_estadistica_t* info){
		printf("El proceso con pid %d se encuentra en estado ", info->pid);
		switch (info->estado) {
			case NEW:
				printf("NEW\n");
				break;
			case READY:
				printf("READY\n");
				break;
			case EXEC:
				printf("EXEC\n");
				break;
			case FINISH:
				printf("FINISH\n");
				break;
			case BLOQ:
				printf("BLOQ\n");
				break;
		}
	}

	if(!strcmp(param, "")){
		list_iterate(listadoEstadistico, listarTodos);
		return;
	}

	if(!strcmp(param, "new")) estado = NEW;
	if(!strcmp(param, "ready")) estado = READY;
	if(!strcmp(param, "exec")) estado = EXEC;
	if(!strcmp(param, "finish")) estado = FINISH;
	if(!strcmp(param, "bloq")) estado = BLOQ;

	list_iterate(listadoEstadistico, listarProcesos);

}

void processInfo(char* comando, char* param){

	if(!esNumero(param)){
		printf("Ingrese un valor numerico valido para el proceso\n");
		return;
	}

	int pid = atoi(param);

	bool buscar(info_estadistica_t* info){
		return info->pid == pid ? true : false;
	}

	info_estadistica_t* info = list_find(listadoEstadistico, buscar);
	if(info == NULL){
		printf("no se encuentra ese poceso en el sistema\n");
	}else{
		printf("Cantidad rafagas: %d\n", info->cantRafagas);
		printf("Cantidad operaciones privilegiadas: %d\n", info->cantOpPrivi);
		printf("Cantidad paginas de heap: %d\n", info->cantPaginasHeap);
		printf("Cantidad acciones alocar: %d\n", info->cantAlocar);
		printf("Cantidad acciones liberar: %d\n", info->cantLiberar);
		printf("Cantidad syscalls: %d\n", info->cantSyscalls);
	}

}

void getTablaArchivos(char* comando, char* param){
        imprimirTablaGlobal();
}
void gradoMultiprogramacion(char* comando, char* param){
	if(!esNumero(param)){
		printf("Ingrese un valor valido para el grado de multiporgramacion\n");
		return;
	}
	config->grado_MultiProg = atoi(param);
	printf("Grado de Multiprogramacion cambiado con exito, ahora es %d\n", config->grado_MultiProg);
}
void killProcess(char* comando, char* param){
        if(!esNumero(param)){
        	printf("ingrese un valor valido\n");
        	return;
        }
        int pid = atoi(param);
        info_estadistica_t* info = buscarInformacion(pid);
        if(info == NULL){
        	printf("Ese proceso no se encuentra en el sistema\n");
        	return;
        }
        if(info->estado != EXEC){ //como no esta ejecutando no tengo que esperar a que cpu lo devuelva
        	eliminarEstadistica(info->pid);
        	enviar_paquete_vacio(FINALIZAR_EJECUCION, info->socketConsola);
        }else{ //debo esperar a que el cpu devuelva el pcb;
        	info->matarSiguienteRafaga = true;
        }

}
void stopPlanification(char* comando, char* param){
       if(planificacionActivada){
    	   pthread_mutex_lock(&lockPlanificacion);
    	   planificacionActivada = false;
    	   pthread_mutex_unlock(&lockPlanificacion);
    	   printf("Planificacion desactiviada\n");
       }else{
    	   pthread_mutex_lock(&lockPlanificacion);
    	   planificacionActivada = true;
    	   pthread_cond_signal(&lockCondicionPlanificacion);
    	   pthread_mutex_unlock(&lockPlanificacion);
    	   printf("Planificacion Activada\n");
       }
}

void agregarNuevaCPU(t_list* lista, int socketCPU){
	cpu_t* nuevaCPU = malloc(sizeof(cpu_t));
	nuevaCPU->socket = socketCPU;
	nuevaCPU->pcb = NULL;
	nuevaCPU->disponible = true;

	list_add(lista, nuevaCPU);
	sem_post(&semCPUs_disponibles);
}

void liberarCPU(cpu_t* cpu){
	//no implementado todavia
	//liberarPCB(cpu->pcb);

	free(cpu);
}

void eliminarCPU(t_list* lista, int socketCPU){

	bool condicion(cpu_t* cpu){
		return cpu->socket == socketCPU ? true : false;
	}

	list_remove_and_destroy_by_condition(lista, condicion, liberarCPU);
}

void actualizarReferenciaPCB(int id, t_pcb* pcb){

	bool condicion(cpu_t* cpu){
		return cpu->socket == id ? true : false;
	}

	cpu_t* cpu = list_find(listaCPUs, condicion);
	cpu->pcb = pcb;
}

cpu_t* obtenerCpuLibre(){

	bool estaLibre(cpu_t* cpu){
		return cpu->disponible;// != NULL ? false : true;
	}

	return list_find(listaCPUs, estaLibre);

}

void planificarCortoPlazo(){

	while(1){

		//Espera que halla CPUs Disponibles
		sem_wait(&semCPUs_disponibles);
		//Espera procesos para ejecutar
		sem_wait(&sem_cola_ready);

		//evaluo si la planificacion esta activada
		pthread_mutex_lock(&lockPlanificacion);
		while(!planificacionActivada){
			pthread_cond_wait(&lockCondicionPlanificacion, &lockPlanificacion);
		}
		pthread_mutex_unlock(&lockPlanificacion);

		cpu_t* cpu = obtenerCpuLibre();
		if(cpu != NULL){
			printf("Obtuve cpu libre\n");
		}else{
			return;
		}

		sem_wait(&mutex_cola_ready);
		t_pcb* pcb = queue_pop(colaReady);
		sem_post(&mutex_cola_ready);

		enviarPcbCPU(pcb, cpu->socket);
		//TODO: Agregar proceso a la lista de ejecuccion.
		estadisticaCambiarEstado(pcb->pid, EXEC);

		cpu->pcb = pcb;
	}
}

void enviarPcbCPU(t_pcb* pcb, int socketCPU){
	t_buffer_tamanio* buffer = serializar_pcb(pcb);
	header_t header;
	header.type = EXEC_PCB;
	header.length = buffer->tamanioBuffer;
	sendSocket(socketCPU, &header, buffer->buffer);

	int quantum = 0;
	header.type=EXEC_QUANTUM;
	if(!strcmp(config->algoritmo, "RR")){
		quantum = config->quantum;
	}
	header.length = sizeof(int);
	sendSocket(socketCPU, &header, &quantum);
}

void planificarLargoPlazo(){

	if(cantProcesosSistema >= config->grado_MultiProg){
		printf("el proceso debe esprar, cantidad maxima de procesos en sistema alcanzada\n");
		return;
	}

	if(queue_size(colaNew) == 0){ //no hay nada que planificar
		return;
	}

	sem_wait(&mutex_cola_new);
	proceso_en_espera_t* proc = queue_pop(colaNew);
	sem_post(&mutex_cola_new);

	//hago peticion a memoria, si se rechaza alerto a consola y el grado de multiProg sigue igual
	//si acepta pongo en cola ready y creo pcb;

	//creo el pedido para la memoria
	t_pedido_iniciar pedido;
	int pid = proc->pid;

	int cant_pag_cod = strlen(proc->codigo) / pagina_size;
	if(strlen(proc->codigo) % pagina_size > 0)
		cant_pag_cod++;

	pedido.pid = pid;
	pedido.cant_pag = config->stack_Size + cant_pag_cod;
	log_info(logger, "Envio pedido de paginas a memoria. pid:%d, cantPags: %d", pid, pedido.cant_pag);

	header_t header;
	header.type = INICIAR_PROGRAMA;
	header.length = sizeof(t_pedido_iniciar);
	sendSocket(socketConexionMemoria, &header, &pedido);

	void* paquete;
	int resultado;

	//evaluo respuesta
	recibir_paquete(socketConexionMemoria, &paquete, &resultado);

	if(resultado == SIN_ESPACIO){
		//aviso a consola que se rechazo
		enviar_paquete_vacio(proc->socketConsola, PROCESO_RECHAZADO);
		log_error(logger, "Proceso rechazado porque no hay espacio en memoria");
	}
	if(resultado == OP_OK){
		log_info(logger, "Paginas reservadas para el proceso %d", pid);
		//aviso a consola que se acepto
		alertarConsolaProcesoAceptado(&pid, proc->socketConsola);

		//mando a memoria el codigo
		envioCodigoMemoria(proc->codigo);

		//creo pcb y paso el proceso a ready
		t_pcb* pcb = crearPCB(proc->codigo,pid,proc->socketConsola);
		//proceso_t* proceso = crearProceso(pcb);
		sem_wait(&mutex_cola_ready);
		queue_push(colaReady, pcb);
		sem_post(&mutex_cola_ready);
		sem_post(&sem_cola_ready);
		estadisticaCambiarEstado(pid, READY);
		cantProcesosSistema++;

		//destruyo el proceso en espera;
		free(proc->codigo);
		free(proc);
		printf("fin plp\n");
	}

}

void alertarConsolaProcesoAceptado(int* pid, int socketConsola){
	header_t header;

	header.type = PID_PROGRAMA;
	header.length = sizeof(int);
	sendSocket(socketConsola, &header, pid);
}

void envioCodigoMemoria(char* codigo){
	header_t header;

	header.type = ENVIO_CODIGO;
	header.length = strlen(codigo)+1;
	sendSocket(socketConexionMemoria, &header, codigo);
}

void crearInfoEstadistica(int pid, uint32_t socketConsola){
	info_estadistica_t* info = malloc(sizeof(info_estadistica_t));
	info->pid = pid;
	info->cantLiberar = 0;
	info->cantAlocar = 0;
	info->cantOpPrivi = 0;
	info->cantPaginasHeap = 0;
	info->cantRafagas = 0;
	info->cantSyscalls = 0;
	info->estado = NEW;
	info->socketConsola = socketConsola;
	info->matarSiguienteRafaga = false;

	list_add(listadoEstadistico, info);
}

info_estadistica_t* buscarInformacion(int pid){

	bool buscar(info_estadistica_t* info){
		return info->pid == pid ? true : false;
	}

	return list_find(listadoEstadistico, buscar);

}

void estadisticaAumentarRafaga(int pid){
	info_estadistica_t* info = buscarInformacion(pid);
	info->cantRafagas++;
}
void estadisticaAumentarSyscall(int pid){
	info_estadistica_t* info = buscarInformacion(pid);
	info->cantSyscalls++;
}
void estadisticaAumentarOpPriviligiada(int pid){
	info_estadistica_t* info = buscarInformacion(pid);
	info->cantOpPrivi++;
}
void estadisticaAumentarAlocar(int pid){
	info_estadistica_t* info = buscarInformacion(pid);
	info->cantAlocar++;
}
void estadisticaAumentarLiberar(int pid){
	info_estadistica_t* info = buscarInformacion(pid);
	info->cantLiberar++;
}
void estadisticaCambiarEstado(int pid, uint8_t nuevoEstado){
	info_estadistica_t* info = buscarInformacion(pid);
	info->estado = nuevoEstado;
}
void aumentarEstadisticaPorSocketAsociado(int socket, void(*estadistica)(int pid)){
	cpu_t* cpu = obtener_cpu_por_socket_asociado(socket);
	estadistica(cpu->pcb->pid);
}
void eliminarEstadistica(int pid){

	bool buscar(info_estadistica_t* info){
		return info->pid == pid ? true : false;
	}

	list_remove_and_destroy_by_condition(listadoEstadistico, buscar, free);
}


void crearColasBloqueados(char** semaforos){
	int j = 0;

	bloqueos = dictionary_create();

	while(semaforos[j] != NULL){
		dictionary_put(bloqueos, semaforos[j], queue_create());
		j++;
	}

}

void desbloquearProceso(char* semaforo){
	t_queue* cola = (t_queue*)dictionary_get(bloqueos, semaforo);
	t_pcb* pcb = queue_pop(cola);

	queue_push(colaReady, pcb);
	estadisticaCambiarEstado(pcb->pid, READY);

	sem_post(&sem_cola_ready);
}

void bloquearProceso(char* semaforo, t_pcb* pcb){

	t_queue* cola = (t_queue*)dictionary_get(bloqueos, semaforo);
	queue_push(cola, pcb);

	estadisticaAumentarRafaga(pcb->pid);
	estadisticaCambiarEstado(pcb->pid, BLOQ);
}


int getArchivoFdMax(){
	max_archivo_fd++;
	return max_archivo_fd;
}

void crearEntradaArchivoProceso(int proceso){
	entrada_tabla_archivo_proceso* entrada = malloc(sizeof(entrada_tabla_archivo_proceso));
	entrada->proceso = proceso;
	entrada->archivos = list_create();

	list_add(processFileTable, entrada);
}

void agregarArchivo_aProceso(int proceso, char* file, char* permisos){

	bool buscar(entrada_tabla_archivo_proceso* entrada){
		return entrada->proceso == proceso ? true : false;
	}

	bool buscarArchivo(entrada_tabla_globlal_archivo* entrada){
		return !strcmp(entrada->archivo, file) ? true : false;
	}

	entrada_tabla_archivo_proceso* entrada = list_find(processFileTable, buscar);

	entrada_tabla_globlal_archivo* entradaGlobal = list_find(globalFileTable, buscarArchivo);

	if(entradaGlobal == NULL){ // no existe
		entradaGlobal = malloc(sizeof(entradaGlobal));
		memcpy(entradaGlobal->archivo, file, strlen(file));
		entradaGlobal->vecesAbierto = 1;
		entradaGlobal->ubicacion = list_size(globalFileTable);

		list_add(globalFileTable, entradaGlobal);
	}else{ //existe en la tabla global
		entradaGlobal->vecesAbierto++;
	}

	archivo* archivo = malloc(sizeof(archivo));
	archivo->flags = permisos;
	archivo->fd = getArchivoFdMax(); //aca tengo que pasarselo a la cpu
	archivo->globalFD = entradaGlobal->ubicacion; //ver esto que es una paja
	list_add(entrada->archivos, archivo);

}

void eliminarFd(int fd, int proceso){

	bool buscarPorProceso(entrada_tabla_archivo_proceso* entrada){
		return entrada->proceso == proceso ? true : false;
	}

	bool eliminar(archivo* archivo){
		return archivo->fd == fd ? true : false;
	}

	entrada_tabla_archivo_proceso* entrada = list_find(processFileTable, buscarPorProceso);
	list_remove_by_condition(entrada->archivos, eliminar);
	entrada_tabla_globlal_archivo* entradaGlobal = list_get(globalFileTable, entradaGlobal->ubicacion);
	entradaGlobal->vecesAbierto--;

	if(entradaGlobal->vecesAbierto == 0){
		list_remove_and_destroy_element(globalFileTable,entradaGlobal->ubicacion, free);
		free(entradaGlobal);
	}
	free(entrada);

}

void imprimirTablaGlobal(){

	void imprimirData(entrada_tabla_globlal_archivo* entrada){
		printf("Nombre del file: %s\n", entrada->archivo);
		printf("Cantidad de veces abierto: %s\n", entrada->vecesAbierto);
	}

	list_iterate(globalFileTable, imprimirData);

}

