#include "funcionesCpu.h"

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

void asignar(t_puntero direccion_variable, t_valor_variable valor){
	printf("asignar!\n");
	return;
}
t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){
	printf("asignarVariableCompartida!\n");
	return 0;
}
t_puntero definirVariable(t_nombre_variable identificador_variable){
	printf("definirVariable!\n");
	return 0;
}
t_valor_variable dereferenciar(t_puntero direccion_variable){
	printf("dereferenciar!\n");
	return 0;
}
void finalizar(void){
	printf("finalizar!\n");
	return;
}
void irAlLabel(t_nombre_etiqueta t_nombre_etiqueta){
	printf("irAlLabel!\n");
	return;
}
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
	printf("llamarConRetorno!\n");
	return;
}
void llamarSinRetorno(t_nombre_etiqueta etiqueta){
	printf("llamarSinRetorno!\n");
	return;
}
t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable){
	printf("obtenerPosicionVariable!\n");
	return 0;
}
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){
	printf("obtenerValorCompartida!\n");
	return 0;
}
void retornar(t_valor_variable retorno){
	printf("retornar!\n");
	return;
}

t_descriptor_archivo abrir(t_direccion_archivo direccion, t_banderas flags){
	printf("abrir!\n");
	return 0;
}
void borrar(t_descriptor_archivo direccion){
	printf("borrar!\n");
}
void cerrar(t_descriptor_archivo descriptor_archivo){
	printf("cerrar!\n");
}
void escribir(t_descriptor_archivo descriptor_archivo, void* informacion, t_valor_variable tamanio){
	printf("escribir!\n");
}
void leer(t_descriptor_archivo descriptor_archivo, t_puntero informacion, t_valor_variable tamanio){
	printf("leer!\n");
}
void liberar(t_puntero puntero){
	printf("liberar!\n");
}
void moverCursor(t_descriptor_archivo descriptor_archivo, t_valor_variable posicion){
	printf("moverCursor!\n");
}
t_puntero reservar(t_valor_variable espacio){
	printf("reservar!\n");
	return 0;
}
void signal(t_nombre_semaforo identificador_semaforo){
	printf("signal!\n");
}
void wait(t_nombre_semaforo identificador_semaforo){
	printf("wait!\n");
}

void inicializarFunciones(void){

	funciones = malloc(sizeof(AnSISOP_funciones));
	funcionesKernel = malloc(sizeof(AnSISOP_funciones));

	funciones->AnSISOP_asignar = asignar;
	funciones->AnSISOP_asignarValorCompartida = asignarValorCompartida;
	funciones->AnSISOP_definirVariable = definirVariable;
	funciones->AnSISOP_dereferenciar = dereferenciar;
	funciones->AnSISOP_finalizar = finalizar;
	funciones->AnSISOP_irAlLabel = irAlLabel;
	funciones->AnSISOP_llamarConRetorno = llamarConRetorno;
	funciones->AnSISOP_llamarSinRetorno = llamarSinRetorno;
	funciones->AnSISOP_obtenerPosicionVariable = obtenerPosicionVariable;
	funciones->AnSISOP_obtenerValorCompartida = obtenerValorCompartida;
	funciones->AnSISOP_retornar = retornar;
	funcionesKernel->AnSISOP_abrir = abrir;
	funcionesKernel->AnSISOP_borrar = borrar;
	funcionesKernel->AnSISOP_cerrar = cerrar;
	funcionesKernel->AnSISOP_escribir = escribir;
	funcionesKernel->AnSISOP_leer = leer;
	funcionesKernel->AnSISOP_liberar = liberar;
	funcionesKernel->AnSISOP_moverCursor = moverCursor;
	funcionesKernel->AnSISOP_reservar = reservar;
	funcionesKernel->AnSISOP_signal = signal;
	funcionesKernel->AnSISOP_wait = wait;
}

void procesarProgramas(void){
	inicializarFunciones();


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
