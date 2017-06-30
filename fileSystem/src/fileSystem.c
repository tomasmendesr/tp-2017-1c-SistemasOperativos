#include "funcionesFs.h"

int main(int argc, char** argv){

	logger = log_create("../logFS","FS", 1, LOG_LEVEL_TRACE);

	crearConfig(argc, argv);

	inicializarMetadata();

	esperarConexionKernel();

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

	pathMetadata = string_new();
	string_append(&pathMetadata, conf->punto_montaje);
	string_append(&pathMetadata, METADATA_PATH);
	printf("%s\n", pathMetadata);

	pathBloques = string_new();
	string_append(&pathBloques, conf->punto_montaje);
	string_append(&pathBloques, BLOQUES_PATH);

	pathArchivos = string_new();
	string_append(&pathArchivos, conf->punto_montaje);
	string_append(&pathArchivos, ARCHIVOS_PATH);

	mkdirRecursivo(conf->punto_montaje);
	mkdir(pathMetadata, 0777);
	mkdir(pathBloques, 0777);
	mkdir(pathArchivos, 0777);

	pathMetadataArchivo = string_new();
	string_append(&pathMetadataArchivo, pathMetadata);
	string_append(&pathMetadataArchivo, METADATA_ARCHIVO);

	FILE* metadata = fopen(pathMetadataArchivo, "w");
	fprintf(metadata, "TAMANIO_BLOQUES=%d\n", conf->tamanio_bloque);
	fprintf(metadata, "CANTIDAD_BLOQUES=%d\n", conf->cantidad_bloques);
	fprintf(metadata, "MAGIC_NUMBER=SADICA\n");
	fclose(metadata);

	int sizeBitArray = conf->cantidad_bloques / 8;//en bytes
	if((sizeBitArray % 8) != 0)
		sizeBitArray++;

	pathMetadataBitarray = string_new();
	string_append(&pathMetadataBitarray, pathMetadata);
	string_append(&pathMetadataBitarray, BITMAP_ARCHIVO);

	if(validarArchivo(pathMetadataBitarray)){
		FILE* bitmap = fopen(pathMetadataBitarray, "rb");

		struct stat stats;
		fstat(fileno(bitmap),&stats);

		char* data = malloc(stats.st_size);

		fread(data, stats.st_size, 1, bitmap);

		fclose(bitmap);

		bitarray = bitarray_create_with_mode(data,stats.st_size, LSB_FIRST);

	}else{

		bitarray = bitarray_create_with_mode(string_repeat('\0', sizeBitArray), sizeBitArray, LSB_FIRST);


		FILE* bitmap = fopen(pathMetadataBitarray, "w");
		fwrite(bitarray->bitarray, sizeBitArray, 1, bitmap);
		fclose(bitmap);

	}

	//para crear los n bloques.bin

	int j;
	FILE* bloque;

	for (j = 1 ; j<=conf->cantidad_bloques ; j++){

		char* pathBloque = string_new();
		string_append(&pathBloque, pathBloques);
		string_append(&pathBloque, "/");
		string_append(&pathBloque, string_itoa(j));
		string_append(&pathBloque, ".bin");

		if(!verificarExistenciaDeArchivo(pathBloque)){
			bloque = fopen(pathBloque, "w");
			fwrite(string_repeat('\0', conf->tamanio_bloque),conf->tamanio_bloque, 1, bloque);
			fclose(bloque);
		}

		free(pathBloque);
	}

	printf("inicialize la metadata\n");
}
