#include "funcionesFs.h"

int main(){

	if(verificarExistenciaDeArchivo(configuracionFS))
		conf = levantarConfiguracion(configuracionFS);
	else{
		printf("No se puede cargar la configuracion");
		return EXIT_FAILURE;
	}

	printf("%s\n", conf->punto_montaje);

	destruirConfiguracionFS(conf);
	return EXIT_SUCCESS;
}

