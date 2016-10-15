#ifndef _ADMIN_
#define _ADMIN_

#include "zone.h"
#include "IPC.h"
#include "interface.h"

#define MAX_VOITURES 5

/**
 * Creation de l'interface principale
 */
void creation_interface();

/**
 * Creer un segment de memoire partagee et y place la carte 
 * @param fich le fichier contenant la carte
 * @param carte la carte
 * @param nbColonnes le nombre de colonnes de zones dans la carte
 * @param nbLignes le nombre de lignes de zones dans la carte
 * @param debut_shm l'adresse du debut du segment de memoire partagee
 * @param shmid l'identifiant du segment de memoire partagee
 */
void charger_carte (char* fich, Zone** carte, int* nbColonnes, int* nbLignes, int** debut_shm, int* shmid);

/* Routine du thread en charge de decider du changement de couleur des feux */
void *routineFeu(void *arg);

/* Routine du thread en charge de recuperer les informations sur l'execution de la simulation */
void* routineLecteur(void* arg);

#endif
