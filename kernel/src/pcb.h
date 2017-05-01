/*
 * pcb.h
 *
 *  Created on: 12/4/2017
 *      Author: utnso
 */

#ifndef PCB_H_
#define PCB_H_

#include <commons/structUtiles.h>
#include "funcionesKernel.h"
#include <parser/metadata_program.h>
#include <sys/stat.h>
#include <parser/parser.h>

//Prototipos de creacion y manejo de pcb
pcb_t* crearPCB(char* codigo, int id, int fd);
t_list* llenarLista(t_intructions * indiceCodigo, t_size cantInstruc);

#endif /* PCB_H_ */
