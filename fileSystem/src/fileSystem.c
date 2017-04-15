#include "funcionesFs.h"

int main(int argc, char** argv){

	logger = log_create(getenv("../logFS"),"FS", 1, 0);

	char* pathConfig = string_new();

	if (argc>1)string_append(&pathConfig, argv[1]);
		else string_append(&pathConfig, configuracionFS);
	if(verificarExistenciaDeArchivo(configuracionFS))
		conf = levantarConfiguracion(configuracionFS);
	else {
		log_error(logger, "No pudo levantarse el archivo de configuracion");
		exit(EXIT_FAILURE);
	}
	printf("Configuracion levantada correctamente\n");

	esperarConexionKernel();

	destruirConfiguracionFS(conf);
	return EXIT_SUCCESS;
}

void esperarConexionKernel(){
	socketEscucha = createServer(IP, conf->puertoEscucha, BACKLOG);
	if(socketEscucha != -1){
		printf("Esperando conexion del kernel...\n");
		log_info(logger,"Esperando conexion del kernel...");
	}else{
		printf("Error al levantar el servidor\n");
		log_error(logger,"Error al levantar el servidor");
	}
	socketConexionKernel = acceptSocket(socketEscucha);

	if(recibirHanshake(socketConexionKernel, HANDSHAKE_KERNEL, HANDSHAKE_FS)){
		printf("Conexion con kernel establecida\n");
		log_info(logger, "Conexion con kernel establecida");
	}
}
