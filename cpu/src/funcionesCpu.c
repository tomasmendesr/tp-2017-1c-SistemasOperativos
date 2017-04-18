#include "funcionesCpu.h"

bool cerrarCPU = false;
bool huboStackOver = false;

int crearLog(void){
	logger = log_create(getenv("/home/utnso/tp-2017-1c-Dirty-Cow/cpu/logCpu"),"cpu", 1, 0);
	if(logger)
		return 1;
	else
		return 0;
}

void crearConfig(int argc, char* argv[]){
	if(argc>1){
		if(verificarExistenciaDeArchivo(argv[1]))
			config=levantarConfiguracionCPU(argv[1]);
		else{
			log_error(logger,"La ruta especificada es incorrecta");
			exit(EXIT_FAILURE);
		}
	}
	else if(verificarExistenciaDeArchivo(configuracionCPU)){
		config=levantarConfiguracionCPU(configuracionCPU);
		log_info(logger,"Configuracion levantada correctamente");
	}
	else if(verificarExistenciaDeArchivo(string_substring_from(configuracionCPU,3))){
		config=levantarConfiguracionCPU(string_substring_from(configuracionCPU,3));
		log_info(logger,"Configuracion levantada correctamente");
	}
	else{
		log_error(logger,"No pudo levantarse el archivo de configuracion");
		exit(EXIT_FAILURE);
	}
}

t_config_cpu* levantarConfiguracionCPU(char* archivo) {

        t_config_cpu* conf = malloc(sizeof(t_config_cpu));
        t_config* configCPU;

        configCPU = config_create(archivo);
        conf->puerto_Kernel = malloc(strlen(config_get_string_value(configCPU, "PUERTO_KERNEL"))+1);
        strcpy(conf->puerto_Kernel, config_get_string_value(configCPU, "PUERTO_KERNEL"));
        conf->puerto_Memoria = malloc(strlen(config_get_string_value(configCPU, "PUERTO_MEMORIA"))+1);
        strcpy(conf->puerto_Memoria, config_get_string_value(configCPU, "PUERTO_MEMORIA"));
        conf->ip_Memoria = malloc(strlen(config_get_string_value(configCPU, "IP_MEMORIA"))+1);
        strcpy(conf->ip_Memoria, config_get_string_value(configCPU, "IP_MEMORIA"));
        conf->ip_Kernel = malloc(strlen(config_get_string_value(configCPU, "IP_KERNEL"))+1);
        strcpy(conf->ip_Kernel, config_get_string_value(configCPU, "IP_KERNEL"));
        config_destroy(configCPU);

        return conf;
}

void freeConf(t_config_cpu* config){
	free(config->ip_Kernel);
	free(config->ip_Memoria);
	free(config->puerto_Kernel);
	free(config->puerto_Memoria);
	free(config);
}

int conexionConKernel(void){

	int operacion;
	void* paquete_vacio;

	socketConexionKernel=createClient(config->ip_Kernel,config->puerto_Kernel);

	if(socketConexionKernel == -1){
		log_error(logger,"No pudo conectarse a Kernel");
		return -1;
	}else{
		log_info(logger,"Cliente a Kernel creado");
	}

	enviar_paquete_vacio(HANDSHAKE_CPU, socketConexionKernel);
	recibir_paquete(socketConexionKernel, &paquete_vacio, &operacion);

	if(operacion==HANDSHAKE_KERNEL){
		log_info(logger,"Conexion establecida con Kernel! :D");
	}else{
		log_info(logger,"El Kernel no devolvio handshake :(");
		return -1;
	}
	return 0;
}

int conexionConMemoria(void){

	int operacion;
	void* paquete_vacio;

	socketConexionMemoria = createClient(config->ip_Memoria, config->puerto_Memoria);
	if(socketConexionMemoria == -1){
		log_error(logger,"No pudo conectarse a Memoria");
		return -1;
	}else{
		log_info(logger,"Cliente a Memoria creado");
	}
	enviar_paquete_vacio(HANDSHAKE_CPU, socketConexionMemoria);
	recibir_paquete(socketConexionMemoria, &paquete_vacio, &operacion);

	if(operacion == HANDSHAKE_MEMORIA){
		log_info(logger,"Conexion establecida con Memoria! :D");
	}else{
		log_info(logger,"La Memoria no devolvio handshake :(");
		return -1;
	}
	return 0;
}

void procesarProgramas(void){
	inicializarFunciones();
	levantarArchivo("facil.ansisop"); // leo programa y me cargo un pcb a lo villero
	analizadorLinea("variables a", funciones, funcionesKernel);
}

void atenderKernel(){
	void* paquete;
	int bytes;
	int tipo_mensaje;

	procesarProgramas();
	bytes = recibir_info(socketConexionKernel, &paquete, &tipo_mensaje);
	if(bytes <= 0){
		log_error(logger, "Desconexion del kernel. Terminando...");
		close(socketConexionKernel);
		exit(1);
	}
	switch (tipo_mensaje) {
	// Mensajes del kernel
	case TAMANIO_STACK_PARA_CPU:
			recibirTamanioStack(paquete);
			break;
		case EXECUTE_PCB:
			recibirPCB(paquete);
			break;
		case VALOR_VAR_COMPARTIDA:
			recibirValorVariableCompartida(paquete);
			break;
		case SIGNAL_SEMAFORO:
			recibirSignalSemaforo(paquete);
			break;
		// Mensajes de memoria
		case ENVIAR_TAMANIO_PAGINA_A_CPU:
				recibirTamanioPagina(paquete);
				break;
	}
}

void recibirTamanioStack(void* paquete){}

void recibirPCB(void* paquete){}

void recibirValorVariableCompartida(void* paquete){}

void recibirAsignacionVariableCompartida(void* paquete){}

void recibirSignalSemaforo(void* paquete){}

void recibirTamanioPagina(void* paquete){}

void levantarArchivo(char*path){
		FILE* file;
	 	int file_fd, file_size;
	 	struct stat stats;

	 	file = fopen(path, "r");
	 	file_fd = fileno(file);

	 	fstat(file_fd, &stats);
	 	file_size = stats.st_size;

	 	char* buffer = malloc(file_size+1);
	 	memset(buffer, '\0',file_size+1);
	 	fread(buffer,file_size,1,file);
	 	pcb = crearPCB(buffer);
}

t_pcb_* crearPCB(char* programa) {
	log_debug(logger, "Se crea un PCB para el Programa Solicitado.");
	t_metadata_program* datos;

	//Obtengo la metadata utilizando el preprocesador del parser
	datos = metadata_desde_literal(programa);

	uint32_t tamanioPCB = 11 * sizeof(uint32_t);
	tamanioPCB += datos->instrucciones_size
			* (sizeof(t_puntero_instruccion) + sizeof(size_t));
	tamanioPCB += tamanioStack;
	if (datos->cantidad_de_etiquetas == 0
			&& datos->cantidad_de_funciones == 0) {
	} else {
		tamanioPCB += datos->etiquetas_size;
	}
	t_pcb_* pcb = malloc(tamanioPCB);

	pcb->pid = 1;

		pcb->stackPointer = 0;
		pcb->programCounter = datos->instruccion_inicio;
		pcb->codigo = datos->instrucciones_size;
		t_list * pcbStack = list_create();
		pcb->indiceStack = pcbStack;
		pcb->tamanioEtiquetas = datos->etiquetas_size;
		//Cargo Indice de Codigo
		t_list * listaIndCodigo = llenarLista(datos->instrucciones_serializado,
				datos->instrucciones_size);
		pcb->indiceCodigo = listaIndCodigo;
		if (datos->cantidad_de_etiquetas > 0
				|| datos->cantidad_de_funciones > 0) {
			char* indiceEtiquetas = malloc(datos->etiquetas_size);
			indiceEtiquetas = datos->etiquetas;
			pcb->etiquetas = indiceEtiquetas;
		} else {

			pcb->etiquetas = NULL;
		}
		free(datos);
		free(programa);

		return pcb;
}


t_list* llenarLista(t_intructions * indiceCodigo, t_size cantInstruc) {
	t_list * lista = list_create();
	int b = 0;
	for (b = 0; b < cantInstruc; b++) {
		t_indice_codigo* linea = malloc(sizeof(t_indice_codigo));
		linea->offset = indiceCodigo[b].start;
		linea->size = indiceCodigo[b].offset;
		list_add(lista, linea);
	}
	return lista;
}

