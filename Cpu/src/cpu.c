#include <stdio.h>
#include "funcionesCpu.h"

int main(){

	t_config_cpu* config = levantarConfiguracionCPU("confCpu.init");

	printf("soy la cpu");

	free(config);
	return 0;
}


