#include "funcionesCpu.h"

int crearLog() {
	logger = log_create(getenv("/home/utnso/tp-2017-1c-Dirty-Cow/cpu/logCpu"),
					"cpu", 1, 0);
	if (logger) {
		return 1;
	} else {
		return 0;
	}
}

void crearConfig(int argc, char* argv[]){
	char* pathConfig=string_new();

	if(argc>1)string_append(&pathConfig,argv[1]);
		else string_append(&pathConfig,configuracionCPU);
	if(verificarExistenciaDeArchivo(pathConfig)){
		config = levantarConfiguracionCPU(pathConfig);
	}else{
		log_error(logger,"No se pudo levantar archivo de configuracion");
		exit(EXIT_FAILURE);
	}
	log_info(logger,"Configuracion levantada correctamente\n");
	return;
}

t_config_cpu* levantarConfiguracionCPU(char* archivo) {

        t_config_cpu* conf = malloc(sizeof(t_config_cpu));
        t_config* configCPU;

        configCPU = config_create(archivo);
        conf->puerto_Kernel = malloc(MAX_LEN_PUERTO);
        conf->puerto_Kernel = config_get_string_value(configCPU, "PUERTO_KERNEL");
        conf->puerto_Memoria = malloc(MAX_LEN_PUERTO);
        conf->puerto_Memoria = config_get_string_value(configCPU, "PUERTO_MEMORIA");
        conf->ip_Memoria = malloc(MAX_LEN_IP);
        conf->ip_Memoria = config_get_string_value(configCPU, "IP_MEMORIA");
        conf->ip_Kernel = malloc(MAX_LEN_IP);
        conf->ip_Kernel = config_get_string_value(configCPU, "IP_KERNEL");


        return conf;
}

int conexionConKernel(){
	socketConexionKernel = createClient(config->ip_Kernel, config->puerto_Kernel);
	if (socketConexionKernel != -1) {
		printf("Cliente a kernel creado\n");
	}else{
		return -1;
	}

	//------------Envio de mensajes al servidor------------
	enviar_paquete_vacio(HANDSHAKE_CPU, socketConexionKernel);
	int operacion = 0;
	void* paquete_vacio;

	recibir_paquete(socketConexionKernel, &paquete_vacio, &operacion);

	if (operacion == HANDSHAKE_KERNEL) {
		log_info(logger, "Conexion con Kernel establecida! :D \n");
		return 0;
	} else {
		log_error(logger,"El Kernel no devolvio handshake :( \n");
		return -1;
	}
}
int conexionConMemoria(){
	socketConexionMemoria = createClient(config->ip_Memoria, config->puerto_Memoria);
	if (socketConexionMemoria != -1) {
		log_info(logger, "Cliente a Memoria creado\n");
	}else{
		return -1;
	}

	//------------Envio de mensajes al servidor------------
	enviar_paquete_vacio(HANDSHAKE_CPU, socketConexionMemoria);
	int operacion = 0;
	void* paquete_vacio;

	recibir_paquete(socketConexionMemoria, &paquete_vacio, &operacion);

	if (operacion == HANDSHAKE_MEMORIA) {
		log_info(logger, "Conexion con Memoria establecida! :D \n");
		return 0;
	} else {
		log_error(logger,"La Memoria no devolvio handshake :( \n");
		return -1;
	}
}


void atenderKernel(){
	void* paquete;
	int bytes;
	int tipo_mensaje;

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
		case ASIG_VAR_COMPARTIDA:
			recibirAsignacionVariableCompartida(paquete);
			break;
		case SIGNAL_SEMAFORO:
			recibirSignalSemaforo(paquete);
			break;
		// Mensajes de memoria
		case ENVIAR_TAMANIO_PAGINA_A_CPU:
			recibirTamanioPagina(paquete);
			break;
		case ENVIAR_INSTRUCCION_A_CPU:
			recibirInstruccion(paquete);
			break;
		}
}

void recibirTamanioStack(void* paquete){}

void recibirPCB(void* paquete){}

void recibirValorVariableCompartida(void* paquete){}

void recibirAsignacionVariableCompartida(void* paquete){}

void recibirSignalSemaforo(void* paquete){}

void recibirTamanioPagina(void* paquete){}

void recibirInstruccion(void* paquete){}
