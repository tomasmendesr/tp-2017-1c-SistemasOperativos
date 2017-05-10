#include "funcionesFs.h"

int main(int argc, char** argv){

	logger = log_create("logFS","FS", 1, LOG_LEVEL_TRACE);

	crearConfig(argc, argv);

	inicializarMetadata();

	//esperarConexionKernel();

	destruirConfiguracionFS(conf);
	return EXIT_SUCCESS;
}

void esperarConexionKernel(){
	socketEscucha = createServer(IP, conf->puertoEscucha, BACKLOG);
	if(socketEscucha != -1){
		log_info(logger,"Esperando conexion del kernel...");
	}else{
		log_error(logger,"Error al levantar el servidor");
	}

	socketConexionKernel = acceptSocket(socketEscucha);

	void* paquete;
	int tipo_mensaje;

	if(recibir_paquete(socketConexionKernel, &paquete, &tipo_mensaje)){
		log_info(logger, "Conexion con kernel establecida");
	}

	procesarMensajesKernel();
}

void inicializarMetadata(){

	char* metadata_path = string_new();
	string_append(&metadata_path, conf->punto_montaje);
	string_append(&metadata_path, METADATA_PATH);
	printf("%s\n", metadata_path);

	char* bloques_path = string_new();
	string_append(&bloques_path, conf->punto_montaje);
	string_append(&bloques_path, BLOQUES_PATH);

	char* archivos_path = string_new();
	string_append(&archivos_path, conf->punto_montaje);
	string_append(&archivos_path, ARCHIVOS_PATH);

	mkdir(conf->punto_montaje, 0777);
	mkdir(metadata_path, 0777);
	mkdir(bloques_path, 0777);
	int resultado = mkdir(archivos_path, 0777);
	if(resultado == -1){
		log_error(logger, "No pudo crearse la metadata");
		exit(1);
	}

	char* bitmap_path = string_new();
	string_append(&bitmap_path, metadata_path);

	string_append(&metadata_path, METADATA_ARCHIVO);
	FILE* metadata = fopen(metadata_path, "a");
	fprintf(metadata, "TAMANIO_BLOQUES=%d\n", conf->tamanio_bloque);
	fprintf(metadata, "CANTIDAD_BLOQUES=%d\n", conf->cantidad_bloques);
	fprintf(metadata, "MAGIC_NUMBER=SADICA\n");
	fclose(metadata);


	int sizeBitArray = conf->cantidad_bloques / 8;//en bytes
	if((sizeBitArray % 8) != 0)
		sizeBitArray++;

	bitarray = bitarray_create_with_mode(string_repeat('0', sizeBitArray), sizeBitArray, MSB_FIRST);

	int index;
	for(index = 0; index < conf->cantidad_bloques; index++)
		bitarray_clean_bit(bitarray, index);

	char* data = malloc(sizeBitArray);
	for(index =0; index <sizeBitArray; index++);
		data[index] = '\0';

	string_append(&bitmap_path, BITMAP_ARCHIVO);
	printf("%s\n", bitmap_path);
	FILE* bitmap = fopen(bitmap_path, "a");
	fwrite(data, sizeBitArray, 1, bitmap);
	fclose(bitmap);

	//para crear los n bloques.bin

	int j;
	FILE* bloque;
	strcat(bloques_path, "/");
	char* path = bloques_path;
	int null = strlen(bloques_path);

	for (j = 1 ; j<=conf->cantidad_bloques ; j++){

		strcat(path, string_itoa(j));
		strcat(path, ".bin");
		printf("%s\n", path);
		bloque = fopen( path, "a");
		fclose(bloque);
		path[null] = '\0';
	}

	free(metadata_path);
	free(bloques_path);
	free(archivos_path);
	free(bitmap_path);

	printf("inicialize la metadata\n");
}
