#ifndef SRC_CONSOLA_H_
#define SRC_CONSOLA_H_

#include <pthread.h>

int crearLog();
int iniciarConsola();
char* leerArchivo(FILE *archivo);
int iniciarProcesoLeyendoArchivoPropio();

#endif /* SRC_CONSOLA_H_ */
