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


	string_append(&metadata_path, METADATA_ARCHIVO);
	FILE* metadata = fopen(metadata_path, "w");
	fprintf(metadata, "TAMANIO_BLOQUES=%d\n", conf->tamanio_bloque);
	fprintf(metadata, "CANTIDAD_BLOQUES=%d\n", conf->cantidad_bloques);
	fprintf(metadata, "MAGIC_NUMBER=SADICA\n");
	fclose(metadata);

	free(metadata_path);
	free(bloques_path);
	free(archivos_path);

	printf("inicialize la metadata\n");
}

