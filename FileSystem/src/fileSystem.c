#include <stdio.h>
#include "funcionesFileSystem.h"

int main(){

	t_config_fs* conf = levantarConfiguracionFileSystem("confFileSystem.init");

	printf("soy el file system");


	free(conf);
	return 0;
}


