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

void trabajarConexionCPU(){
	int socket_servidor_kernel = createServer(IP,config->puerto_CPU,BACKLOG);
	if(socket_servidor_kernel){
		printf("Esperando conexion CPU...\n");
	}
	int numero_maximo_socket;
	int newSocket;

	fd_set read_fds;
	fd_set socket_master;

	FD_ZERO(&read_fds);
	FD_ZERO(&socket_master);
	FD_SET(socket_servidor_kernel,&socket_master);
	numero_maximo_socket = socket_servidor_kernel;
	int iterador_sockets;
	void* paquete;

	while(1){
		read_fds = socket_master;

		if(select(numero_maximo_socket + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}
	for(iterador_sockets = 0; iterador_sockets <= numero_maximo_socket; iterador_sockets++) {
		if(FD_ISSET(iterador_sockets, &read_fds)) {
			if(iterador_sockets == socket_servidor_kernel) {

				newSocket = acceptSocket(socket_servidor_kernel);

				if(newSocket == -1) {
					perror("Error al aceptar");
				} else {
					FD_SET(newSocket, &socket_master);
					if(newSocket > numero_maximo_socket) numero_maximo_socket = newSocket;
				}
			} else {
				//Gestiono cada conexi�n -> Recibo los programas y creo sus PCB.
				int tipo_mensaje; //Para que la funcion recibir_string lo reciba
				int check = recibir_paquete(iterador_sockets, &paquete, &tipo_mensaje);

				//Chequeo de errores
				if (check == 0) {
					printf("Se cerro el socket %d\n", iterador_sockets);
					close(iterador_sockets);
					FD_CLR(iterador_sockets, &socket_master);
				}

				if(check == -1){
					perror("recv");
					close(iterador_sockets);
					FD_CLR(iterador_sockets, &socket_master);
				}
				// Fin chequeo de errores

				if(check > 0) {
					switch(tipo_mensaje){
						case HANDSHAKE_CPU:{
							printf("conexion con cpu establecida");
							enviar_paquete_vacio(HANDSHAKE_KERNEL,iterador_sockets);
							enviarTamanioStack(iterador_sockets);
							break;
						}
						default:{ printf("Se recibio un codigo no valido\n");
							break;
						}
					}
				}
			}
		}
	}
	}
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


proceso_en_espera_t* crearProceso(int consola_fd, char* source){

	proceso_en_espera_t* proc = malloc(sizeof(proceso_en_espera_t));
	proc->socketConsola = consola_fd;
	proc->codigo = malloc(strlen(source));
	memcpy(proc->codigo, source, strlen(source));

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

	void* paquete;
		int tipo_mensaje;

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
	int* previo = dictionary_get(dic, key);
	*previo = *previo + 1;

	return atoi(previo);
}

int semaforoWait(t_dictionary* dic, char* key){
	int* previo = dictionary_get(dic, key);
	*previo = *previo - 1;

	return atoi(previo);
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

void agregarNuevaCPU(t_list* lista, int socketCPU){
	cpu_t* nuevaCPU = malloc(sizeof(cpu_t));
	nuevaCPU->socket = socketCPU;
	nuevaCPU->pcb = NULL;

	list_add(lista, nuevaCPU);
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

void actualizarReferenciaPCB(int id, pcb_t* pcb){

	bool condicion(cpu_t* cpu){
		return cpu->socket == id ? true : false;
	}

	cpu_t* cpu = list_find(listaCPUs, condicion);
	cpu->pcb = pcb;
}


cpu_t* obtenerCpuLibre(){

	bool estaLibre(cpu_t* cpu){
		return cpu->pcb == NULL ? true : false;
	}

	return list_find(listaCPUs, estaLibre);

}

void planificarCortoPlazo(){

	while(1){

		sem_wait(&semCPUs);
		sem_wait(&sem_cola_ready);
		cpu_t* cpu = obtenerCpuLibre();
		pcb_t* pcb = queue_pop(colaReady);

		//envio pcb a la cpu, aun no implementado
		//enviarPCB(cpu, pcb);

	}
}

void planificarLargoPlazo(){

	sem_wait(&sem_cola_new); //si la cola esta vacio bloqueo
	sem_wait(&mutex_cola_new);
	proceso_en_espera_t* proc = queue_pop(colaNew);
	sem_post(&mutex_cola_new);

	//hago peticion a memoria, si se rechaza alerto a consola y el grado de multiProg sigue igual
	//si acepta pongo en cola ready y creo pcb;

	//creo el pedido para la memoria
	t_pedido_iniciar pedido;
	int pid = asignarPid();
	pedido.pid = pid;
	pedido.cant_pag = config->stack_Size;

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
	}
	if(resultado == OP_OK){
		//aviso a consola que se acepto
		header.type = PID_PROGRAMA;
		header.length = sizeof(int);
		memcpy(paquete, &pid, header.length);
		sendSocket(proc->socketConsola, &header, paquete);

		//mando a memoria el codigo
		header.type = ENVIO_CODIGO;
		header.length = strlen(proc->codigo);
		memcpy(paquete, proc->codigo, header.length);
		sendSocket(socketConexionMemoria, &header, paquete);

		//creo pcb y paso el proceso a ready
		pcb_t* pcb = crearPCB(proc->codigo, pid);
		sem_wait(&mutex_cola_ready);
		queue_push(colaReady, pcb);
		sem_post(&mutex_cola_ready);

		//destruyo el proceso en espera;
		free(proc->codigo);
		free(proc);
	}

}

