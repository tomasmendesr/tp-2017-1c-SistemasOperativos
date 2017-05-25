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
	return verificarExistenciaDeArchivo(path);
}

void crearArchivo(void* package){
	//no implementado aun
}

void borrarArchivo(void* package){
	char* path_archivo = (char*)package;

	if(validarArchivo(path_archivo)){
		enviar_paquete_vacio(ARCHIVO_INEXISTENTE, socketConexionKernel);
	}else{
		unlink(generarPathArchivo(path_archivo));

		//tengo que liberar los bloques

	}
}

void guardarDatos(void* package){
	pedido_guardar_datos* pedido = deserializar_pedido_guardar_datos(package);


	free(pedido->path);
	free(pedido->buffer);
	free(pedido);
}

void obtenerDatos(void* package){
	pedido_obtener_datos* pedido = deserializar_pedido_obtener_datos(package);

	if(validarArchivo(pedido->path)){
		enviar_paquete_vacio(ARCHIVO_INEXISTENTE, socketConexionKernel);
		return;
	}

	int p = obtenerNumBloque(pedido);

	char* path_bloque = generarPathBloque(p);

	FILE* archivo = fopen(path_bloque, "r");
	char* buffer = malloc(sizeof(pedido->size));

	int offsetReal = pedido->offset;
	while(offsetReal > conf->tamanio_bloque) offsetReal -= conf->tamanio_bloque;

	fseek(archivo, offsetReal, SEEK_SET);
	fread(buffer, pedido->size, 1, archivo);

	header_t header;
	header.length = pedido->size;
	header.type = LECTURA_OK;
	sendSocket(socketConexionKernel, &header, buffer);

	fclose(archivo);
	free(path_bloque);
	free(pedido->path);
	free(pedido);
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
	char byte;
	int cantLeida = 0;

	FILE* bitarray = fopen(pathMetadataBitarray, "rb");

	fread(&byte, 1, 1, bitarray);

	while(!res){
		if(!(byte & 0x01)){
			res = true;
		}else{
			byte = byte >> 1;
			cantLeida++;
			j++;
		}
		if(cantLeida == 8){
			fread(&byte, 1, 1, bitarray);
			cantLeida = 0;
		}

	}

	fclose(bitarray);

	return j;
}

void escribirValorBitarray(int valor, int pos){
	char c;

	int posByte = pos / 8;
	int posBit = (pos % 8) * 8;

	FILE* bitarray = fopen(pathMetadataBitarray, "rb");

	fseek(bitarray, posByte, SEEK_SET);
	if((c = getc(bitarray)) != EOF){
		c ^= 1 << posBit;
		fseek(bitarray, -1L, SEEK_CUR);
		putc(c, bitarray);
		fflush(bitarray);
	}

	fclose(bitarray);

}

char** obtenerNumeroBloques(char* path){
	t_config* c = config_create(path);
	char** bloques = config_get_array_value(c, "BLOQUES");
	config_destroy(c);

	return bloques;
}

int obtenerNumBloque(pedido_obtener_datos* pedido){
	char** bloques = obtenerNumeroBloques(generarPathArchivo(pedido->path));

	int numBloque = (pedido->offset / conf->tamanio_bloque);
	return atoi(bloques[numBloque]);
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
