#include "funcionesKernel.h"

int main(){

	t_config_kernel* conf = levantarConfiguracionKernel("confKernel.init");

	printf("soy el kernel");

	free(conf);
	return 0;
}


