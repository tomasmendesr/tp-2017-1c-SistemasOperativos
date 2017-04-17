/*
 * pcp.h
 *
 *  Created on: 11/4/2017
 *      Author: utnso
 */

#ifndef PCP_H_
#define PCP_H_

void pcp();

void procesarMensajeCPU(int socketCPU, int mensaje, char* package);

void realizarSignal(int socketCPU, char* key);
void realizarWait(int socketCPU, char* key);
void enviarValorSemaforo(int socketCPU, int tipoMensaje, int valorSemaforo);

#endif /* PCP_H_ */
