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
	int check;

	while(1){

		check = recibir_paquete(socketConexionKernel, &paquete, &tipo_mensaje);

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

}

bool validarArchivo(char* path){
	return verificarExistenciaDeArchivo(path);
}

void crearArchivo(void* package){
	char* pathArchivo = generarPathArchivo(package);

	int bloqueLibre = buscarBloqueLibre();

	escribirValorBitarray(1, bloqueLibre); //pongo el bloque como ocupado

	char* p = generarPathArchivo(pathArchivo);

	FILE* archivo = fopen(p, "a");

	fprintf(archivo, "TAMANIO=0\n");
	fprintf(archivo, "BLOQUES=[%d]\n", bloqueLibre);

	fclose(archivo);

}

void borrarArchivo(void* package){
	char* path_archivo = (char*)package;

	if(validarArchivo(path_archivo)){
		enviar_paquete_vacio(ARCHIVO_INEXISTENTE, socketConexionKernel);
	}else{

		char* path = generarPathArchivo(path_archivo);

		t_config* data = config_create(path);
		char** bloques = config_get_array_value(data, "BLOQUES");
		config_destroy(data);

		int j = 0;
		while(bloques[j] != NULL){
			escribirValorBitarray(0, bloques[j]); //libero los bloques
			j++;
		}

		//borro el archivo
		unlink(path);
		enviar_paquete_vacio(BORRAR_ARCHIVO_OK, socketConexionKernel);
	}
}

void guardarDatos(void* package){
	pedido_guardar_datos* pedido = deserializar_pedido_guardar_datos(package);

	if(validarArchivo(pedido->path)){
		enviar_paquete_vacio(ARCHIVO_INEXISTENTE, socketConexionKernel);
		return;
	}

	char** bloques = obtenerNumeroBloques(pedido->path);
	int cantBloques = cantidadBloques(bloques);

	int offsetReal = pedido->offset, j=0;
	int restoBloque, bytesEscritos = 0, bloque;

	while(offsetReal > conf->tamanio_bloque){
		offsetReal -= conf->tamanio_bloque;
		j++;
	}

	bloque = atoi(bloques[j]);

	while(pedido->size != 0){
		restoBloque = conf->tamanio_bloque - offsetReal;

		escribirEnArchivo(bloque, pedido->buffer, restoBloque);
		bytesEscritos += restoBloque;
		pedido->size -= restoBloque;

		offsetReal = 0;

		if(pedido->size > 0){//aun faltan cosas por escribir
			if(j == cantBloques){
				bloque = reservarNuevoBloque(pedido->path);
			}else{
				j++;
				bloque = atoi(bloques[j]);
			}
		}

	}

	free(pedido->path);
	free(pedido->buffer);
	free(pedido);

	enviar_paquete_vacio(ESCRITURA_OK, socketConexionKernel);

}

void escribirEnArchivo(int bloque, char* buffer, int size){
	FILE* archivo = fopen(generarPathBloque(bloque), "a");

	fwrite(buffer, size, 1, archivo);

	close(archivo);
}

void obtenerDatos(void* package){
	pedido_obtener_datos* pedido = deserializar_pedido_obtener_datos(package);

	if(validarArchivo(pedido->path)){
		enviar_paquete_vacio(ARCHIVO_INEXISTENTE, socketConexionKernel);
		return;
	}

	char* buffer = malloc(sizeof(pedido->size));

	char** bloques = obtenerNumeroBloques(pedido->path);

	int offsetReal = pedido->offset, j=0, bytesLeidos = 0, restoBloque;

	while(offsetReal > conf->tamanio_bloque){
		offsetReal -= conf->tamanio_bloque;
		j++;
	}

	int bloque = atoi(bloques[j]);

	while(bytesLeidos != pedido->size){
		restoBloque = conf->tamanio_bloque - offsetReal;

		leerArchivo(bloque, buffer+bytesLeidos, restoBloque);

		bytesLeidos += restoBloque;

		j++;
		bloque = atoi(bloques[j]);
		offsetReal = 0;
	}


	header_t header;
	header.length = pedido->size;
	header.type = LECTURA_OK;
	sendSocket(socketConexionKernel, &header, buffer);

	free(pedido->path);
	free(pedido);
}

void leerArchivo(int bloque, char* buffer, int size){
	FILE* archivo = fopen(generarPathBloque(bloque), "a");

	fread(buffer, size, 1, archivo);

	close(archivo);
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
            mkdir(tmp, 0777);
            *p = '/';
        }
    mkdir(tmp, 0777);
}

int buscarBloqueLibre(){

	int i;

	for(i=0; bitarray_test_bit(bitarray, i); i++){

	}

	return i;
}

void escribirValorBitarray(bool valor, int pos){

	if(valor)
		bitarray_set_bit(bitarray, pos);
	else
		bitarray_clean_bit(bitarray, pos);

	FILE* bitmap = fopen(pathMetadataBitarray, "w");
	fwrite(bitarray->bitarray, bitarray->size, 1, bitmap);
	fclose(bitmap);

}

char** obtenerNumeroBloques(char* path){
	t_config* c = config_create(path);
	char** bloques = config_get_array_value(c, "BLOQUES");
	config_destroy(c);

	return bloques;
}

int obtenerNumBloque(char* path, int offset){
	char** bloques = obtenerNumeroBloques(generarPathArchivo(path));

	int numBloque = (offset / conf->tamanio_bloque);
	return atoi(bloques[numBloque]);
}

int reservarNuevoBloque(char* pathArchivo){
	FILE* archivo = fopen(pathArchivo, "a");

	int bloqueLibre = buscarBloqueLibre();
	escribirValorBitarray(1, bloqueLibre);

	fseek(archivo, -1, SEEK_END);
	fprintf(archivo, ",%d]", bloqueLibre);

	fclose(archivo);

	return bloqueLibre;
}

int cantidadBloques(char** bloques){
	int j=0;

	while(bloques[j] != NULL)
		j++;

	return j;
}

pedido_obtener_datos* deserializar_pedido_obtener_datos(char* paquete){

	pedido_obtener_datos* pedido = malloc(sizeof(pedido_obtener_datos));
	memcpy(&(pedido->offset), paquete, sizeof(int));
	memcpy(&(pedido->size), paquete+sizeof(int), sizeof(int));
	int len_path;
	memcpy(&len_path, paquete+2*sizeof(int), sizeof(int));
	pedido->path = malloc(len_path+1);
	memcpy(pedido->path, paquete+3*sizeof(int), len_path);

	return pedido;
}

pedido_guardar_datos* deserializar_pedido_guardar_datos(char* paquete){
	int offset = 0;

	pedido_guardar_datos* pedido = malloc(sizeof(pedido_guardar_datos));
	memcpy(&(pedido->offset), paquete, sizeof(int));
	offset += sizeof(int);
	memcpy(&(pedido->size), paquete+offset, sizeof(int));
	offset += sizeof(int);
	int len_path;
	memcpy(&len_path, paquete+offset, sizeof(int));
	offset += sizeof(int);
	pedido->path = malloc(len_path+1);
	memcpy(pedido->path, paquete+offset, len_path);
	pedido->path[len_path] = '\0';
	offset += len_path;

	int len_buffer;
	memcpy(&len_buffer, paquete+offset, sizeof(int));
	offset += sizeof(int);
	pedido->buffer = malloc(len_buffer+1);
	memcpy(pedido->buffer, paquete+offset, len_buffer);
	pedido->buffer[len_buffer] = '\0';

	return pedido;
}

char* generarPathBloque(int num_bloque){
	char* path_bloque = string_new();
	strcat(path_bloque, conf->punto_montaje);
	strcat(path_bloque, "Bloques/");
	strcat(path_bloque, string_itoa(num_bloque));
	strcat(path_bloque, ".bin");

	return path_bloque;
}

char* generarPathArchivo(char* path){
	char* path_archivo = string_new();
	strcat(path_archivo, conf->punto_montaje);
	strcat(path_archivo, "Archivos/");
	strcat(path_archivo, path);

	return path_archivo;
}
