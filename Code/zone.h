#ifndef _ZONE_
#define _ZONE_

#include "case.h"

#define TAILLE_ZONE 10

typedef struct{
	int id;	/* Identifiant de la zone */
	Case mat [TAILLE_ZONE][TAILLE_ZONE];
}Zone;

/**
 * Creation d'une zone vide.
 * @param id l'identifiant de la zone
 * @return la zone creer
 **/
Zone* creer_Zone(int id);

/**
 * Suppresion d'une zone.
 * @param zone la zone a supprimer
 **/
void zone_supprimer(Zone** zone);

/**
 * Sauvegarde d'une zone dans le fichier carte.
 * @param zone la zone a sauvegarder
 * @param fd le descripteur du fichier carte
 **/
void Zone_sauver(Zone* z, int fd);

/**
 * Chargement d'une zone depuis un fichier.
 * @param z le pointeur dans lequel on charge la zone
 * @param idZone l'id de la zone a charger
 * @param fd le descripteur du fichier carte
 * @return la zone chargee
 **/
void Zone_charger(Zone* z ,int idZone, int fd);

#endif
