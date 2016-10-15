#include <stdlib.h> 
#include <stdio.h>
#include <unistd.h>     /* Pour read, write */
#include <fcntl.h>

#include "zone.h"

/**
 * Creation d'une zone vide
 * @param id l'identifiant de la zone
 * @return la zone creer
 **/
Zone* creer_Zone( int id) {
	int i, j;
	Zone* z = (Zone*)malloc(sizeof(Zone));
	
	z->id = id;
	for(i=0; i<TAILLE_ZONE; i++){
		for(j=0; j<TAILLE_ZONE; j++){
			z->mat[i][j] = creer_Case();
		}
	}	
					
	return z;
}

/**
 * Suppresion d'une zone.
 * @param zone la zone a supprimer
 **/
void zone_supprimer(Zone** zone) {
	free(*zone);
}

/**
 * Sauvegarde d'une zone dans le fichier carte et libere la memoire associee
 * @param zone la zone a sauvegarder
 * @param fd le descripteur du fichier carte
 **/
void Zone_sauver(Zone* z, int fd) {
	
	/* Deplacement dans le fichier a l'endroit où ecrire la zone */
	if(lseek(fd, sizeof(Zone)*(z->id-1) + sizeof(int)*2, SEEK_SET) == (off_t)-1){
		perror("Erreur, Zone_sauver : erreur lors du déplacement dans le fichier ");
		exit(EXIT_FAILURE);
	}

	/* Ecriture de la zone sur le fichier */
	if(write(fd, z, sizeof(Zone)) == -1) {
    		perror("Erreur, Zone_sauver : erreur lors de l'ecriture dans le fichier ");
    		exit(EXIT_FAILURE);
  	}
  	
  	/* Liberation de la memoire */
  	zone_supprimer(&z);
}

/**
 * Chargement d'une zone depuis un fichier.
 * @param z le pointeur dans lequel on charge la zone
 * @param idZone l'id de la zone a charger
 * @param fd le descripteur du fichier carte
 * @return la zone chargee
 **/
void Zone_charger(Zone* z ,int idZone, int fd) {
	/* Deplacement dans le fichier a l'endroit où lire la zone */
	if(lseek(fd, sizeof(Zone)*(idZone-1) + sizeof(int)*2, SEEK_SET) == (off_t)-1){
		perror("Erreur, Zone_charger : erreur lors du déplacement dans le fichier ");
		exit(EXIT_FAILURE);
	}

	/* Lecture de la zone */
	if(read(fd, z, sizeof(Zone)) == -1) {
		perror("Erreur, Zone_charger : erreur lors de la lecture dans le fichier");
		exit(EXIT_FAILURE);
	}  	
}

































