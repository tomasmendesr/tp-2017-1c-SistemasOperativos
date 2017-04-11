#include "funcionesFs.h"

int main(){

	if(verificarExistenciaDeArchivo(configuracionFS))
		conf = levantarConfiguracion(configuracionFS);
	else{
		printf("No se pudo cargar la configuracion\n");
		return EXIT_FAILURE;
	}

	esperarConexionKernel();

	destruirConfiguracionFS(conf);
	return EXIT_SUCCESS;
}

void esperarConexionKernel(){
	socketEscucha = createServer(IP, conf->puertoEscucha, BACKLOG);
	if(socketEscucha != -1){
		printf("Esperando conexion del kernel...\n");
	}else{
		printf("Error al levantar el servidor\n");
	}
	socketConexionKernel = acceptSocket(socketEscucha);

	recibirHanshake(socketConexionKernel, HANDSHAKE_KERNEL, HANDSHAKE_FS);
	printf("Conexion con kernel establecida\n");
}
