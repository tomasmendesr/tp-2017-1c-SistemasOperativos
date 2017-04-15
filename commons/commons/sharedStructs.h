/*
 * sharedStructs.h
 *
 *  Created on: 14/4/2017
 *      Author: utnso
 */

#ifndef COMMONS_SHAREDSTRUCTS_H_
#define COMMONS_SHAREDSTRUCTS_H_

typedef struct{
	int pid;
	int pag;
	int offset;
	int size;
}t_operacion_bytes;

typedef struct{
	int pid;
	int cantPag;
}t_operacion_pag;


#endif /* COMMONS_SHAREDSTRUCTS_H_ */
