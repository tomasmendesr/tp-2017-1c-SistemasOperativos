#include "funcionesFs.h"

void crearConfig(int argc, char* argv[]){
	if(argc>1){
		if(verificarExistenciaDeArchivo(argv[1]))
			conf=levantarConfiguracion(argv[1]);
		else{
			log_error(logger,"La ruta especificada es incorrecta");
			exit(EXIT_FAILURE);
		}
	}
	else if(verificarExistenciaDeArchivo(configuracionFS)){
		conf=levantarConfiguracion(configuracionFS);
		log_info(logger,"Configuracion levantada correctamente");
	}
	else if(verificarExistenciaDeArchivo(string_substring_from(configuracionFS,3))){
		conf=levantarConfiguracion(string_substring_from(configuracionFS,3));
		log_info(logger,"Configuracion levantada correctamente");
	}
	else{
		log_error(logger,"No pudo levantarse el archivo de configuracion");
		exit(EXIT_FAILURE);
	}
}

t_config_FS* levantarConfiguracion(char* archivo){

	t_config_FS* conf = malloc(sizeof(t_config_FS));

	t_config* configFS = config_create(archivo);

	conf->puertoEscucha = malloc(strlen(config_get_string_value(configFS, "PUERTO"))+1);
	strcpy(conf->puertoEscucha, config_get_string_value(configFS, "PUERTO"));

	conf->punto_montaje = malloc(strlen(config_get_string_value(configFS, "PUNTO_MONTAJE"))+1);
	strcpy(conf->punto_montaje, config_get_string_value(configFS, "PUNTO_MONTAJE"));

	conf->tamanio_bloque = config_get_int_value(configFS, "TAMANIO_BLOQUE");

	conf->cantidad_bloques = config_get_int_value(configFS, "CANTIDAD_BLOQUES");

	config_destroy(configFS);

	return conf;
}

void destruirConfiguracionFS(t_config_FS* conf){
	free(conf->puertoEscucha);
	free(conf->punto_montaje);
	free(conf);
}

void procesarMensajesKernel(){
	int tipo_mensaje; //Para que la funcion recibir_string lo reciba
	void* paquete;
	int check = recibir_paquete(socketConexionKernel, &paquete, &tipo_mensaje);

	if (check <= 0) {
		log_error(logger, "El kernel se desconecto");
		close(socketConexionKernel);
		exit(1);
	}

	switch (tipo_mensaje) {
		case CREAR_ARCHIVO:
			crearArchivo(paquete);
			break;
		case BORRAR_ARCHIVO:
			borrarArchivo(paquete);
			break;
		case GUARDAR_DATOS:
			guardarDatos(paquete);
			break;
		case OBTENER_DATOS:
			obtenerDatos(paquete);
			break;
		default:
			log_warning(logger, "se recivio una operacion invalida");
			break;
	}

}

bool validarArchivo(char* path){
	//no implementado aun
	return false;
}

void crearArchivo(void* package){
	//no implementado aun
}

void borrarArchivo(void* package){
	//no implementado aun
}

void guardarDatos(void* package){
	//no implementado aun
}

void obtenerDatos(void* package){
	//no implementado aun
}

void mkdirRecursivo(char* path){

	char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",path);
    len = strlen(tmp);
    if(tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for(p = tmp + 1; *p; p++)
        if(*p == '/') {
        	*p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    mkdir(tmp, S_IRWXU);
}

int buscarBloqueLibre(){
	int j = 0;
	bool res = false;
	while(!res){
		if(!bitarray_test_bit(bitarray, j)) // cero esta libre
			res = true;
	}

}

char** obtenerNumeroBloques(char* path){
	t_config* c = config_create(path);
	char** bloques = config_get_array_value(c, "BLOQUES");
	config_destroy(c);

	return bloques;
}

int obtenerNumBloque(char* path, int offset){
	char** bloques = obtenerNumeroBloques(strcat(ARCHIVOS_PATH , path));

	int numBloque = (offset / conf->tamanio_bloque);
	return atoi(bloques[numBloque]);
}
