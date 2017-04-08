#include "funcionesFs.h"

int main(){

	if(verificarExistenciaDeArchivo(configuracionFS))
		conf = levantarConfiguracion(configuracionFS);
	else{
		printf("No se puede cargar la configuracion");
		return EXIT_FAILURE;
	}

	// conexion kernel
	socketEscucha = createServer(IP, conf->puertoEscucha, BACKLOG);

	printf("Esperando conexion del kernel.......");
	socketConexionKernel = acceptSocket(socketEscucha);

	int operacion = 0;
	void* paquete_vacio;

	recibir_paquete(socketConexionKernel, &paquete_vacio, &operacion);

	if (operacion == HANDSHAKE_KERNEL) {
		enviar_paquete_vacio(HANDSHAKE_FS,socketConexionKernel);

		recibir_paquete(socketConexionKernel, &paquete_vacio, &operacion);

		if(operacion == HANDSHAKE_KERNEL){
			printf("kernel conectado");
		}
	}
	// conexion kernel


	destruirConfiguracionFS(conf);
	return EXIT_SUCCESS;
}

