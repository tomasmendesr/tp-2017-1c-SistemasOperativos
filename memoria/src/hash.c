/*
 * hash.c
 *
 *  Created on: 9/5/2017
 *      Author: utnso
 */

#include "hash.h"

void hashInit(int size){
	cant_frame = size;
	createSeed();
}

void createSeed(){
	srand(time(NULL));
	seed1 = rand()%500 + 1000;
	seed2 = rand()%15000 + 5000;

	//Si es multiplo de 10, hago un rerun
	if(seed1%10 == 0 || seed2%10 == 0)
		createSeed();
}

int getPos(int pid, int pag){

	int val = pid*seed1 + pag;

	return (val * seed2) % cant_frame;
}

int getSeed1(){
	return seed1;
}

int getSeed2(){
	return seed2;
}
