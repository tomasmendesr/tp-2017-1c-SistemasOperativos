#include <stdio.h>
#include "funcionesMemoria.h"

int main(){

	t_config_memoria* config = levantarConfiguracionMemoria("confMemoria.init");

	printf("soy la memoria\n");

	free(config);
	return 0;
}


