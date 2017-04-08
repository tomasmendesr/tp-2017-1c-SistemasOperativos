#include "funcionesFs.h"

int main(){

	if(verificarExistenciaDeArchivo(configuracionFS))
		conf = levantarConfiguracion(configuracionFS);
	else{
		printf("No se puede cargar la configuracion");
		return EXIT_FAILURE;
	}

	esperarConexionKernel();

	destruirConfiguracionFS(conf);
	return EXIT_SUCCESS;
}

void esperarConexionKernel(){
	socketEscucha = createServer(IP, conf->puertoEscucha, BACKLOG);

		printf("Esperando conexion del kernel.......\n");
		socketConexionKernel = acceptSocket(socketEscucha);

		int operacion = 0;
		void* paquete_vacio;

		recibirHanshake(socketConexionKernel, HANDSHAKE_KERNEL, HANDSHAKE_FS);

}
