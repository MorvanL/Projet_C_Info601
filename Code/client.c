#define _XOPEN_SOURCE 500 

#include <stdlib.h>     /* Pour exit, EXIT_FAILURE, EXIT_SUCCESS */
#include <stdio.h>      /* Pour printf, perror */
#include <sys/msg.h>    /* Pour msgget, msgsnd, msgrcv */
#include <sys/sem.h>  /* Pour semget, semctl, semop */
#include <errno.h>      /* Pour errno */
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#include "client.h"

int thread_vivant [MAX_VOITURES]; /* tableau de valeurs pour indiquer si le thread est mort ou vivant : 0 etant mort, 1 etant vivant */
pthread_mutex_t mutexAff = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexCreation = PTHREAD_MUTEX_INITIALIZER; /* Mutex protegeant la creation des voitures */
case_t** grille;

pthread_t threadsVoiture [MAX_VOITURES];
args_thread argsThreads [MAX_VOITURES];

fenetre_t win_carte;
Zone*** carte = NULL;
int nbColonnesAff, nbLignesAff;
int numClient = 0;
int semid;
int msqid;

/** 
  * Routine pour liberer un mutex que l'on peut ajouter a la pile de pthread_cleanup 
  * @param coord les coordonnee de la case dont on souhaite liberer le mutex
 **/
void unlock (void* coord) {
	coordGrille_t* c = (coordGrille_t*)coord;
	pthread_mutex_unlock(&((grille[c->x][c->y]).mutex));
}

/** 
  * Routine permettant au thread de signaler qu'il est mort 
  * @param num le numero du thread
 **/
void indique_mort (void* numThread) {
	int* num = (int*)numThread;
	thread_vivant[*num] = 0; 
}

/** 
  * Methode appelee par la methode deplacement pour effectuer les deplacements lorsque l'on reste dans la meme zone d'affichage
  * @param args les arguments du thread de la voiture
  * @param x la coordonnee x de la case a partir de laquelle on appelle la methode (case de la voiture, feu, stop)
  * @param y la coordonnee x de la case a partir de laquelle on appelle la methode (case de la voiture, feu, stop)
  * @param direction la direction dans laquelle veut bouger la voiture
  * @return 1 pour dire que la voiture doit etre detruite
 **/
int dep_out_client(args_thread* args, int x, int y, char direction) {
	int mort = 0;
	int feu = 0;
	char courante;
	int ligneZone, colZone, xZone, yZone;
	int client_receveur = 0;
	int attribuee;
	requete_t requete;
	coordGrille_t coord_courante;

	/* S'il y a un feu devant la voiture */
	zone_getcoordonnees(x, y, &ligneZone, &colZone, &xZone, &yZone);
	courante = carte[ligneZone][colZone]->mat[xZone][yZone].type;
	if((courante == CAR_FEU_ROUGE) || (courante == CAR_FEU_VERT)) {
		feu = 1;
		coord_courante.x = x;
		coord_courante.y = y;
		/* On verifie si le feu est vert */
		pthread_mutex_lock(&((grille[x][y]).mutex));
		while(carte[ligneZone][colZone]->mat[xZone][yZone].type == CAR_FEU_ROUGE) {
			/* Si le feu est rouge on se met en attente jusqu'a ce que le feu soit vert */
			pthread_cleanup_push(unlock, (void*)(&coord_courante));
			pthread_cond_wait(&((grille[x][y]).cond), &((grille[x][y]).mutex));
			pthread_cleanup_pop(NULL);
		}	
	}
	/* On détermine le client qui va recevoir la voiture ainsi que les coordonnees de la voiture pour ce client*/
	switch(direction) {
		case CAR_HAUT:
				client_receveur = numClient - 2;
				requete.x = x;
				requete.y = -2;
			break;
		case CAR_BAS:
				client_receveur = numClient + 2;
				requete.x = x;
				requete.y = -1;
			break;
		case CAR_DROITE:
				client_receveur = numClient + 1;
				requete.x = -1;
				requete.y = y;
			break;
		case CAR_GAUCHE:
				client_receveur = numClient - 1;
				requete.x = -2;
				requete.y = y;
			break;
	}
	
	pthread_mutex_lock(&((grille[args->coordV.x][args->coordV.y]).mutex));
	/* On verifie qu'il n'y a pas eu une demande de suppression */
	if(feu == 1){
		pthread_cleanup_push(unlock, (void*)(&(args->coordV)));
		pthread_cleanup_push(unlock, (void*)(&coord_courante));
		pthread_testcancel();
		pthread_cleanup_pop(NULL);
		pthread_cleanup_pop(NULL);
	}
	else{
		pthread_cleanup_push(unlock, (void*)(&(args->coordV)));
		pthread_testcancel();
		pthread_cleanup_pop(NULL);
	}
	/* On verifie qu'il existe un client d'affichage */
	if((client_receveur >= 1) && (client_receveur <= 4)) {
		if((attribuee = semctl(semid, client_receveur-1, GETVAL)) == -1) {
			perror("Erreur lors de la recuperation de la valeur du semaphore Client");
			exit(EXIT_FAILURE);
		}
		if(attribuee == 0){ /* S'il existe */
			/* On transfert de la voiture au bon client */
			requete.type =  client_receveur + 1;
			requete.vitesse =  args->vitesse;
			snd_requete(msqid, &requete);
			/* On indique le transfert au coordinateur */
			requete.type = 1;
			sprintf(requete.message, "Voiture transferee du client %d au client %d\n", numClient, client_receveur);
			snd_requete(msqid, &requete);
			mort = 2;
		}
	}
	if(feu == 1) {
		pthread_mutex_unlock(&((grille[x][y]).mutex));
	}
	if(mort != 2){
		mort = 1;
	}	
	
	return mort;
}

/** 
  * Methode appelee par la methode deplacement pour effectuer les deplacements lorsque l'on reste dans la meme zone d'affichage
  * @param args les arguments du thread de la voiture
  * @param x la coordonnee x de la case a partir de laquelle on appelle la methode (case de la voiture, feu, stop)
  * @param y la coordonnee x de la case a partir de laquelle on appelle la methode (case de la voiture, feu, stop)
  * @param xSuiv la coordonnee x de la case sur laquelle la voiture veut se deplacer
  * @param ysuiv la coordonnee x de la case sur laquelle la voiture veut se deplacer
  * @param direction la direction dans laquelle veut bouger la voiture
  * @return si la voiture doit etre detruite ou non
 **/
int dep_in_client (args_thread* args, int x, int y, int xSuiv, int ySuiv, char direction) {
	int mort;
	int feu = 0;
	int ligneZone, colZone, xZone, yZone;
	char actuelle; /* Case sur laquelle est la voiture */
	char courante; /* Case ayant les coordonnees x et y (case de la voiture, feu, stop) */
	char suivante; /* Case sur laquelle on souhaite se deplacer */
	coordGrille_t coord_courante; /* Utiliser pour les appels a la methode unlock appelee par pthread_cleanup */
	coordGrille_t coord_suivante; /* Utiliser pour les appels a la methode unlock appelee par pthread_cleanup */
	
	zone_getcoordonnees(xSuiv, ySuiv, &ligneZone, &colZone, &xZone, &yZone);
	suivante = carte[ligneZone][colZone]->mat[xZone][yZone].type;
	/* On rappelle avec xSuiv et ySuiv s'il y a un feu ou un stop devant la voiture pour aller a la case apres le feu ou le stop*/
	if((suivante == CAR_FEU_ROUGE) || (suivante == CAR_FEU_VERT)) {
		mort = deplacement(args, xSuiv, ySuiv, direction);
	}
	else if(suivante == CAR_STOP) {
		sleep(1);
		mort = deplacement(args, xSuiv, ySuiv, direction);
	}
	else if(suivante == CAR_VIDE) {
		mort = 1;
	}
	else {
		coord_suivante.x = xSuiv;
		coord_suivante.y = ySuiv;
		/* S'il y a un feu devant la voiture */
		zone_getcoordonnees(x, y, &ligneZone, &colZone, &xZone, &yZone);
		courante = carte[ligneZone][colZone]->mat[xZone][yZone].type;
		if((courante == CAR_FEU_ROUGE) || (courante == CAR_FEU_VERT)) {
			feu = 1;
			coord_courante.x = x;
			coord_courante.y = y;
			/* On verifie si la case suivant le feu est accessible et si le feu est vert */
			pthread_mutex_lock(&((grille[xSuiv][ySuiv]).mutex));
			pthread_mutex_lock(&((grille[x][y]).mutex));
			while((grille[xSuiv][ySuiv].element == VOITURE) || (carte[ligneZone][colZone]->mat[xZone][yZone].type == CAR_FEU_ROUGE)) { 
				if(grille[xSuiv][ySuiv].element == VOITURE) {
					/* Si une voiture nous bloque on se met en attente jusqu'a ce que la case soit libre */
					pthread_mutex_unlock(&((grille[x][y]).mutex));
					pthread_cleanup_push(unlock, (void*)(&coord_suivante));
					pthread_cond_wait(&((grille[xSuiv][ySuiv]).cond), &((grille[xSuiv][ySuiv]).mutex));
					pthread_cleanup_pop(NULL);
					pthread_mutex_lock(&((grille[x][y]).mutex));
				}
				else{
					/* Si le feu est rouge on se met en attente jusqu'a ce que le feu soit vert */
					pthread_mutex_unlock(&((grille[xSuiv][ySuiv]).mutex));
					pthread_cleanup_push(unlock, (void*)(&coord_courante));
					pthread_cond_wait(&((grille[x][y]).cond), &((grille[x][y]).mutex));
					pthread_cleanup_pop(NULL);
					pthread_mutex_lock(&((grille[xSuiv][ySuiv]).mutex));
				}
			}	
		}
		else {
			pthread_mutex_lock(&((grille[xSuiv][ySuiv]).mutex));
			while(grille[xSuiv][ySuiv].element == VOITURE) {
				/* Si une voiture nous bloque on se met en attente jusqu'a ce que la case soit libre */
				pthread_cleanup_push(unlock, (void*)(&coord_suivante));
				pthread_cond_wait(&((grille[xSuiv][ySuiv]).cond), &((grille[xSuiv][ySuiv]).mutex));
				pthread_cleanup_pop(NULL);
			}
		}
		/* On effectue le deplacement et on met a jour l'affichage */
		pthread_mutex_lock(&((grille[args->coordV.x][args->coordV.y]).mutex));
		/* On verifie qu'il n'y a pas eu une demande de suppression */
		if(feu == 1){
			pthread_cleanup_push(unlock, (void*)(&coord_suivante));
			pthread_cleanup_push(unlock, (void*)(&(args->coordV)));
			pthread_cleanup_push(unlock, (void*)(&coord_courante));
			pthread_testcancel();
			pthread_cleanup_pop(NULL);
			pthread_cleanup_pop(NULL);
			pthread_cleanup_pop(NULL);
		}
		else{
			pthread_cleanup_push(unlock, (void*)(&coord_suivante));
			pthread_cleanup_push(unlock, (void*)(&(args->coordV)));
			pthread_testcancel();
			pthread_cleanup_pop(NULL);
			pthread_cleanup_pop(NULL);
		}
		/* deplacement */
		grille[xSuiv][ySuiv].element = VOITURE;
		grille[args->coordV.x][args->coordV.y].element = VIDE;
		grille[xSuiv][ySuiv].voiture = grille[args->coordV.x][args->coordV.y].voiture;
		grille[args->coordV.x][args->coordV.y].voiture = NULL;
		/* affichage */
		pthread_mutex_lock(&mutexAff);
		fenetre_setpos(&win_carte, xSuiv, ySuiv);
		fenetre_affcar(&win_carte, CAR(VOITURE), COL(VOITURE));
		fenetre_setpos(&win_carte, args->coordV.x, args->coordV.y);
		zone_getcoordonnees(args->coordV.x, args->coordV.y, &ligneZone, &colZone, &xZone, &yZone);
		actuelle = carte[ligneZone][colZone]->mat[xZone][yZone].type;
		if(actuelle == CAR_CROISEMENT)
			fenetre_affcar(&win_carte, CAR_CROISEMENT, COL_CROISEMENT);
		else 
			fenetre_affcar(&win_carte, direction, COL(TYPE(direction)));
		fenetre_maj(&win_carte);
		/* liberation des mutex */
		pthread_mutex_unlock(&mutexAff);
		if(feu == 1) {
			pthread_mutex_unlock(&((grille[x][y]).mutex));
		}
		pthread_cond_signal(&((grille[args->coordV.x][args->coordV.y]).cond)); /* On signal le deplacement si une voiture etait en attente sur la case pour un deplacement */
		pthread_mutex_unlock(&((grille[args->coordV.x][args->coordV.y]).mutex));
		pthread_mutex_unlock(&((grille[xSuiv][ySuiv]).mutex));
		/* fin deplacement */
		args->coordV.x = xSuiv;
		args->coordV.y = ySuiv;
		mort = 0;
	}

	return mort;
}

/** 
  * Methode qui place sur la carte une voiture envoyee par un autre client
  * @param args les arguments du thread de la voiture
  * @param x la coordonnee x de la case a partir de laquelle on appelle la methode (case de la voiture, feu, stop)
  * @param y la coordonnee x de la case a partir de laquelle on appelle la methode (case de la voiture, feu, stop)
  * @param premier_appel vaut 0 si c'est le premier_appel a la fonction, vaut 1 si c'est le deuxieme appel suite a un feu ou un stop
  * @param direction la direction dans laquelle souhaite se deplacer la voiture : connu que si premier_appel vaut 0
  * @param premier_appel vaut 1 si c'est le premier appel a la fonction, vaut 0 si ce n'est pas le premier appel (apres un feu ou un stop)
 **/
void dep_spe (args_thread* args, int x, int y, char direction, int premier_appel) {
	int feu = 0;
	int ligneZone, colZone, xZone, yZone, xSuiv = 0, ySuiv = 0;
	char courante; /* Case ayant les coordonnees x et y (case de la voiture, feu, stop) */
	char suivante; /* Case sur laquelle on souhaite se deplacer */
	coordGrille_t coord_courante; /* Utiliser pour les appels a la methode unlock appelee par pthread_cleanup */
	coordGrille_t coord_suivante; /* Utiliser pour les appels a la methode unlock appelee par pthread_cleanup */

	if(premier_appel) {
		if(x == -2) {
			direction = CAR_GAUCHE;
			xSuiv = nbColonnesAff*TAILLE_ZONE - 1;
			ySuiv = y;
		}
		if(x == -1) {
			direction = CAR_DROITE;
			xSuiv = 0;
			ySuiv = y;
		}	
		if(y == -2) {
			direction = CAR_HAUT;
			xSuiv = x;
			ySuiv = nbLignesAff*TAILLE_ZONE - 1;
		}
		if(y == -1) {
			direction = CAR_BAS;
			xSuiv = x;
			ySuiv = 0;
		}
	}
	else {
		switch(direction) {
			case CAR_HAUT:
				xSuiv = x;
				ySuiv = y - 1;	
				break;
			case CAR_BAS:	
				xSuiv = x;
				ySuiv = y + 1;
				break;
			case CAR_DROITE:	
				xSuiv = x + 1;
				ySuiv = y;
				break;
			case CAR_GAUCHE:
				xSuiv = x - 1;
				ySuiv = y;
				break;
		}
	}

	zone_getcoordonnees(xSuiv, ySuiv, &ligneZone, &colZone, &xZone, &yZone);
	suivante = carte[ligneZone][colZone]->mat[xZone][yZone].type;
	/* On rappelle avec xSuiv et ySuiv s'il y a un feu ou un stop devant la voiture pour aller a la case apres le feu ou le stop*/
	if((suivante == CAR_FEU_ROUGE) || (suivante == CAR_FEU_VERT)) {
		dep_spe(args, xSuiv, ySuiv, direction, 0);
	}
	else if(suivante == CAR_STOP) {
		sleep(1);
		dep_spe(args, xSuiv, ySuiv, direction, 0);
	}
	else {
		coord_suivante.x = xSuiv;
		coord_suivante.y = ySuiv;
		/* S'il y a un feu devant la voiture */
		zone_getcoordonnees(x, y, &ligneZone, &colZone, &xZone, &yZone);
		courante = carte[ligneZone][colZone]->mat[xZone][yZone].type;
		if((courante == CAR_FEU_ROUGE) || (courante == CAR_FEU_VERT)) {
			feu = 1;
			coord_courante.x = x;
			coord_courante.y = y;
			/* On verifie si la case suivant le feu est accessible et si le feu est vert */
			pthread_mutex_lock(&((grille[xSuiv][ySuiv]).mutex));
			pthread_mutex_lock(&((grille[x][y]).mutex));
			while((grille[xSuiv][ySuiv].element == VOITURE) || (carte[ligneZone][colZone]->mat[xZone][yZone].type == CAR_FEU_ROUGE)) {
				if(grille[xSuiv][ySuiv].element == VOITURE) {
					/* Si une voiture nous bloque on se met en attente jusqu'a ce que la case soit libre */
					pthread_mutex_unlock(&((grille[x][y]).mutex));
					pthread_cleanup_push(unlock, (void*)(&coord_suivante));
					pthread_cond_wait(&((grille[xSuiv][ySuiv]).cond), &((grille[xSuiv][ySuiv]).mutex));
					pthread_cleanup_pop(NULL);
					pthread_mutex_lock(&((grille[x][y]).mutex));
				}
				else{
					/* Si le feu est rouge on se met en attente jusqu'a ce que le feu soit vert */
					pthread_mutex_unlock(&((grille[xSuiv][ySuiv]).mutex));
					pthread_cleanup_push(unlock, (void*)(&coord_courante));
					pthread_cond_wait(&((grille[x][y]).cond), &((grille[x][y]).mutex));
					pthread_cleanup_pop(NULL);
					pthread_mutex_lock(&((grille[xSuiv][ySuiv]).mutex));
				}
			}	
		}
		else {
			pthread_mutex_lock(&((grille[xSuiv][ySuiv]).mutex));
			while(grille[xSuiv][ySuiv].element == VOITURE) {
				/* Si une voiture nous bloque on se met en attente jusqu'a ce que la case soit libre */
				pthread_cleanup_push(unlock, (void*)(&coord_suivante));
				pthread_cond_wait(&((grille[xSuiv][ySuiv]).cond), &((grille[xSuiv][ySuiv]).mutex));
				pthread_cleanup_pop(NULL);
			}
		}
		/* On effectue le deplacement et on met a jour l'affichage */
		/* deplacement */
		grille[xSuiv][ySuiv].element = VOITURE;
		grille[xSuiv][ySuiv].voiture = &(threadsVoiture[args->numThread]);
		/* affichage */
		pthread_mutex_lock(&mutexAff);
		fenetre_setpos(&win_carte, xSuiv, ySuiv);
		fenetre_affcar(&win_carte, CAR(VOITURE), COL(VOITURE));
		fenetre_maj(&win_carte);
		/* liberation des mutex */
		pthread_mutex_unlock(&mutexAff);
		if(feu == 1) {
			pthread_mutex_unlock(&((grille[x][y]).mutex));
		}
		pthread_mutex_unlock(&((grille[xSuiv][ySuiv]).mutex));
		/* fin deplacement */
		args->coordV.x = xSuiv;
		args->coordV.y = ySuiv;
	}
}

/** 
  * Methode de la routine des voitures qui effectue les deplacements 
  * @param args les arguments du thread de la voiture
  * @param x la coordonnee x de la case a partir de laquelle on appelle la methode (case de la voiture, feu, stop)
  * @param y la coordonnee x de la case a partir de laquelle on appelle la methode (case de la voiture, feu, stop)
  * @param direction la direction dans laquelle veut bouger la voiture
  * @return l'etat au deplacement
 **/
int deplacement (args_thread* args, int x, int y, char direction) {
	int mort = 0;
	int xSuiv, ySuiv;

	switch(direction) {
		case CAR_HAUT:
			/* Si on est au bord de la carte */
			if(y == 0) {
				mort = dep_out_client(args, x, y, direction);
			}
			else{	/* Si on n'est pas au bord de la carte */
				/* On regarde si le deplacement sur la case suivante est possible */
				xSuiv = x;
				ySuiv = y - 1;
				mort = dep_in_client(args, x, y, xSuiv, ySuiv, direction);
			}	
			break;
		case CAR_BAS:
			if(y == nbLignesAff*TAILLE_ZONE-1) {
				mort = dep_out_client(args, x, y, direction);
			}
			else{	
				xSuiv = x;
				ySuiv = y + 1;
				mort = dep_in_client(args, x, y, xSuiv, ySuiv, direction);
			}
			break;
		case CAR_DROITE:
			if(x == nbColonnesAff*TAILLE_ZONE-1) {
				mort = dep_out_client(args, x, y, direction);
			}
			else{	
				xSuiv = x + 1;
				ySuiv = y;
				mort = dep_in_client(args, x, y, xSuiv, ySuiv, direction);
			}
			break;
		case CAR_GAUCHE:
			if(x == 0) {
				mort = dep_out_client(args, x, y, direction);
			}
			else{	
				xSuiv = x - 1;
				ySuiv = y;
				mort = dep_in_client(args, x, y, xSuiv, ySuiv, direction);
			}
			break;
	}

	return mort;
}

void *routineVoiture(void *args) {
	requete_t requete;
	int mort = 0;
	args_thread* arg = (args_thread*)args;
	coordGrille_t* coordVoiture = &(arg->coordV);
	int ligneZone, colZone, xZone, yZone;
	char direction = CAR_HAUT, ancienne_direction;
	int entree_crois = 1, sortie_crois, nbTestOk, i;
	struct sembuf opVoiture;
	int statut;
	unsigned long temps;

	pthread_cleanup_push(indique_mort, (void*)(&(arg->numThread))); 
	/* Si c'est une voiture envoyee d'un autre client alors on la met a sa place sur la carte*/
	if((coordVoiture->x < 0) || (coordVoiture->y < 0)){
		dep_spe(arg, coordVoiture->x, coordVoiture->y, ' ', 1);
	}
	/* Definition du temps entre 2 deplacements selon la vitesse de la voiture */
	temps = arg->vitesse *50000;
	usleep(temps);
	while (1) {
		/* On recupère la direction a suivre et on effectue le deplacement */
		zone_getcoordonnees(coordVoiture->x, coordVoiture->y, &ligneZone, &colZone, &xZone, &yZone);
		ancienne_direction = direction;
		/* Si on se trouve sur un croisement on regarde la table de routage */
		if((direction = carte[ligneZone][colZone]->mat[xZone][yZone].type) == CAR_CROISEMENT) {
			switch(ancienne_direction) {
				case CAR_HAUT:
						entree_crois = 2;
					break;
				case CAR_BAS:
						entree_crois = 1;
					break;
				case CAR_DROITE:
						entree_crois = 8;
					break;
				case CAR_GAUCHE:
						entree_crois = 4;
					break;
			}
			sortie_crois = 0;
			nbTestOk = (rand()%6 + 1); /* On choisi un nombre aleatoire entre 1 et 6 pour ne pas avantager une sortie plus qu'une autre */
			i = 0;
			while(i < nbTestOk){
				sortie_crois = sortie_crois * 2;
				if(sortie_crois == 0)
					sortie_crois = 1;
				else if(sortie_crois == 16)
					sortie_crois = 0;
				if(tableRoutage_direction(&(carte[ligneZone][colZone]->mat[xZone][yZone].tRoutage), entree_crois, sortie_crois) == DIRECTION_OK) {
					i++;
				}
			}
			switch(sortie_crois) {
				case 1:
						direction = CAR_HAUT;
					break;
				case 2:
						direction = CAR_BAS;
					break;
				case 4:
						direction = CAR_DROITE;
					break;
				case 8:
						direction = CAR_GAUCHE;
					break;
			}
				
		}
		else {
			direction = carte[ligneZone][colZone]->mat[xZone][yZone].type;
		}
		mort = deplacement(arg, coordVoiture->x, coordVoiture->y, direction);
		if((mort == 1) || (mort == 2)) { /* Mort de la voiture par manque de client d'affichage */
			/* on detache le thread */
			statut = pthread_detach(pthread_self());
			if(statut != 0){
				fprintf(stderr, "Probleme detach thread voiture\n");	
				exit(EXIT_FAILURE);	
			}
			/* On indique la voiture morte */
			/* On augmente le semaphore gerant le nombre de voiture s'il n'y a pas transfert de la voiture */
			if(mort == 1){
				opVoiture.sem_num = 4;
				opVoiture.sem_op = 1;
				opVoiture.sem_flg = 0;
				if(semop(semid, &opVoiture, 1) == -1) {
				    perror("Erreur lors de l'operation sur le semaphore ");
				    exit(EXIT_FAILURE);
				}
			}
			grille[coordVoiture->x][coordVoiture->y].element = VIDE;
			grille[coordVoiture->x][coordVoiture->y].voiture = NULL;
			/* On met a jour l'affichage */
			pthread_mutex_lock(&mutexAff);
			fenetre_setpos(&win_carte, coordVoiture->x, coordVoiture->y);
			fenetre_affcar(&win_carte, direction, COL(TYPE(direction)));
			fenetre_maj(&win_carte);
			pthread_mutex_unlock(&mutexAff);
			/* On signal la place libre a une possible voiture en attente */
			pthread_cond_signal(&((grille[coordVoiture->x][coordVoiture->y]).cond));
			pthread_mutex_unlock(&((grille[coordVoiture->x][coordVoiture->y]).mutex));
			/* On indique la mort du thread au coordinateur s'il ne s'agit pas d'un transfert */
			if(mort == 1){
				requete.type = 1;
				sprintf(requete.message, "Suppression d'une voiture sur le client numero %d \n", numClient);
				snd_requete(msqid, &requete);
			}

			pthread_exit(NULL);
		}
		usleep(temps);
	}	
	pthread_cleanup_pop(NULL);			
	pthread_exit(NULL);
}

/* Routine du thread en charge d'effectuer le changement de couleur des feux sur ce client */
void *routineFeu(void *arg) {
	int i, j;
	int ligneZone, colZone, xZone, yZone;
	int nbFeux = 0, numFeux = 0, feu_vert;
	coordGrille_t* coords; /* Tableau contenant les coordonnees des cases feu de la zone */
	Case** cases_feu; /* Tableau de pointeurs sur les cases feu de la zone */ 
	case_t** cases_grille_feu; /* Tableau de pointeurs sur les cases de la grille qui correspondent à une case feu de la zone */ 
	struct sembuf opFeux;

	/* Parcours de la carte pour connaitre le nombre de feux */
	for(i=0; i < nbColonnesAff*TAILLE_ZONE; i++) {
		for(j=0; j < nbLignesAff*TAILLE_ZONE; j++) {
			zone_getcoordonnees(i, j, &ligneZone, &colZone, &xZone, &yZone);
		  	if((carte[ligneZone][colZone]->mat[xZone][yZone].type == CAR_FEU_ROUGE) || (carte[ligneZone][colZone]->mat[xZone][yZone].type == CAR_FEU_VERT)) {
				nbFeux++;
			}	
		}
	}
	
	/* On alloue les tableaux selon le nombre de feux */
	coords = (coordGrille_t*)malloc(sizeof(coordGrille_t)*nbFeux);
	cases_feu = (Case**)malloc(sizeof(Case*)*nbFeux);
	cases_grille_feu = (case_t**)malloc(sizeof(case_t*)*nbFeux);

	/* Parcours de la carte pour recuperer les positions des feux */
	for(i=0; i < nbColonnesAff*TAILLE_ZONE; i++) {
		for(j=0; j < nbLignesAff*TAILLE_ZONE; j++) {
			zone_getcoordonnees(i, j, &ligneZone, &colZone, &xZone, &yZone);
		  	if((carte[ligneZone][colZone]->mat[xZone][yZone].type == CAR_FEU_ROUGE) || (carte[ligneZone][colZone]->mat[xZone][yZone].type == CAR_FEU_VERT)) {
				cases_feu[numFeux] = &(carte[ligneZone][colZone]->mat[xZone][yZone]);
				cases_grille_feu[numFeux] = &(grille[i][j]);
				coords[numFeux].x = i;
				coords[numFeux].y = j;
				numFeux++;
			}	
		}
	}

	while(1) {
		/* Attend que le semaphore indiquant un changement de couleur de feux passe a 0 */
		/* On met le thread annulable pendant l'attente */
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
		opFeux.sem_num = 6;
		opFeux.sem_op = 0;
		opFeux.sem_flg = 0;
		if(semop(semid, &opFeux, 1) == -1) {
		    perror("Erreur lors des operations sur les semaphores de feux cote client ");
		    exit(EXIT_FAILURE);
		}
		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

		/* On recupere la valeur pour laquelle les feux passent au vert */
		if((feu_vert = semctl(semid, 5, GETVAL)) == -1) {
			perror("Erreur lors des operations sur les semaphores de feux cote admin");
			exit(EXIT_FAILURE);
		}
		
		/* On effectue le changement de couleur */
		for(i=0; i < numFeux; i++) {
			pthread_mutex_lock(&(cases_grille_feu[i]->mutex));
		}
		pthread_mutex_lock(&mutexAff);
		for(i=0; i < numFeux; i++) {
			if(cases_feu[i]->tRoutage.table[1] == feu_vert)  {
				cases_feu[i]->type = CAR_FEU_VERT;
				fenetre_setpos(&win_carte, coords[i].x, coords[i].y);
				fenetre_affcar(&win_carte, CAR(FEU_VERT), COL(FEU_VERT));
				/* On signal que le feu passe au vert */
				pthread_cond_signal(&(cases_grille_feu[i]->cond));
			}
			else if(cases_feu[i]->type == CAR_FEU_VERT) {
				cases_feu[i]->type = CAR_FEU_ROUGE;
				fenetre_setpos(&win_carte, coords[i].x, coords[i].y);
				fenetre_affcar(&win_carte, CAR(FEU_ROUGE), COL(FEU_ROUGE));
			}
		}
		fenetre_maj(&win_carte);
		pthread_mutex_unlock(&mutexAff);
		for(i=0; i < numFeux; i++) {
			pthread_mutex_unlock(&(cases_grille_feu[i]->mutex));
		}
		


		/* On augmente la valeur du semaphore de signalement pour ne plus qu'elle soit egale a 0 */
		opFeux.sem_num = 6;
		opFeux.sem_op = 1;
		opFeux.sem_flg = 0;
		if(semop(semid, &opFeux, 1) == -1) {
		    perror("Erreur lors des operations sur les semaphores de feux cote client ");
		    exit(EXIT_FAILURE);
		}
	}	

	pthread_exit(NULL);
}


/* Routine du thread en charge de recuperer les voitures transmise par un autre client */
void *routine_Lecteur(void *arg) {
	requete_t requete;
	int numVoiture = 0;
	int statut;

	while(1) {
		/* Attente de reception d'un message */
		rcv_requete(msqid, &requete, numClient + 1);
			
		/* traitement du message reçu => creation du thread voiture*/
		pthread_mutex_lock(&mutexCreation);
		while(thread_vivant[numVoiture] == 1){
			numVoiture++;
		}
		pthread_mutex_unlock(&mutexCreation);
		thread_vivant[numVoiture] = 1;
		argsThreads[numVoiture].coordV.x = requete.x;
		argsThreads[numVoiture].coordV.y = requete.y;
		argsThreads[numVoiture].numThread = numVoiture;
		argsThreads[numVoiture].vitesse = requete.vitesse;
		statut = pthread_create(&threadsVoiture[numVoiture], NULL, routineVoiture, (void *)(&(argsThreads[numVoiture])));
		if(statut != 0){
			fprintf(stderr, "Probleme creation thread voiture lors du transfert\n");	
			exit(EXIT_FAILURE);	
		}
		numVoiture = 0;
	}

	pthread_exit(NULL);
}

/**
 * Recupere la zone d'affichage dans le segment de memoire partagee
 * @param carte la carte
 * @param numClient le numero du client d'affichage
 * @param nbColonnesAff le nombre de colonnes de zones affichees par le client
 * @param nbLignesAff le nombre de lignes de zones affichees par le client
 * @param debut_shm l'adresse du debut du segment de memoire partagee
 */
void recuperer_carte(Zone**** carte, int numClient, int* nbColonnesAff, int* nbLignesAff, int** debut_shm) {
	int i, j;	
	struct sembuf op;
	int attribuee;
	int shmid;
	int nbColonnesTot, nbLignesTot;
	requete_t requete;

	/* Verification que la zone n'est pas deja attribuee */
	if((attribuee = semctl(semid, numClient-1, GETVAL)) == -1) {
		perror("Erreur lors de la recuperation de la valeur du semaphore nbClient");
		exit(EXIT_FAILURE);
	}
	/* Si la zone est disponible on l'attribue sinon fin du processus */
	if(attribuee == 1) {
		op.sem_num = numClient-1;
		op.sem_op = -1;
		op.sem_flg = 0;
		if(semop(semid, &op, 1) == -1) {
		    perror("Erreur lors de l'operation sur le semaphore ");
		    exit(EXIT_FAILURE);
		}
		/* On indique la de creation d'un client au coordinateur */
		requete.type = 1;
		sprintf(requete.message, "Creation d'un client pour la zone numero %d\n", numClient);
		snd_requete(msqid, &requete);
	}
	else {
		/* On indique la tentative de creation d'un client au coordinateur */
		requete.type = 1;
		sprintf(requete.message, "Creation d'un client rejetee : Client deja existant pour la zone numero %d\n", numClient);
		snd_requete(msqid, &requete);
		printf("Erreur la zone est deja attribuee a un client d'affichage\n");
		exit(EXIT_FAILURE);
	}

	/* Recuperation et attachement du segment de memoire partage */
	shmid = recuperation_shm ();
	*debut_shm = (int*)attachement_shm (shmid);
	nbColonnesTot = *(*debut_shm);
	nbLignesTot = *(*debut_shm+1);
	
	/* Recuperation de la zone d'affichage*/
	/* Calcul du nombre de zones a affichees */
	if(numClient == 1) {
		*nbColonnesAff = (nbColonnesTot+1)/2;
		*nbLignesAff = (nbLignesTot+1)/2;
	}
	else if(numClient == 2) {
		*nbColonnesAff = nbColonnesTot/2;
		*nbLignesAff = (nbLignesTot+1)/2;
	}
	else if(numClient == 3) {
		*nbColonnesAff = (nbColonnesTot+1)/2;
		*nbLignesAff = nbLignesTot/2;
	}
	else if(numClient == 4) {
		*nbColonnesAff = nbColonnesTot/2;
		*nbLignesAff = nbLignesTot/2;
	}
	/* Allocation des pointeurs pour la carte a afficher */
	*carte = (Zone***)malloc(sizeof(Zone**)*(*nbLignesAff));
	for(i=0; i < *nbLignesAff; i++) {
		(*carte)[i] = (Zone**)malloc(sizeof(Zone*)*(*nbColonnesAff));
	}
	/* Recuperation des zones correspondantes */
	if((numClient == 1) || (numClient == 2)) {
		for(i=0; i < *nbLignesAff; i++) {
			for(j=0; j < *nbColonnesAff; j++) {
				(*carte)[i][j] = (Zone*)(((*debut_shm)+2)+(((((numClient-1)*((nbColonnesTot+1)/2))+(i*nbColonnesTot)+j)*sizeof(Zone)))/4);
			}
		}
	}
	else {
		for(i=0; i < *nbLignesAff; i++) {
			for(j=0; j < *nbColonnesAff; j++) {
				(*carte)[i][j] = (Zone*)(((*debut_shm)+2)+(((((numClient-3)*((nbColonnesTot+1)/2))+(i*nbColonnesTot)+(((nbLignesTot/2)+1)*(nbColonnesTot))+j)*sizeof(Zone))/4));
			}
		}
	}
}

/**
 * Creation de l'interface principale
 * @param win_carte la fenetre contenant la carte
 * @param carte la carte a afficher
 * @param nbLignesAff le nombre de lignes de zones affichees par le client
 * @param nbColonnesAff le nombre de c de zones affichees par le client
 */
void creation_interface(fenetre_t *win_carte, Zone*** carte, int nbLignesAff, int nbColonnesAff) {
	/* Initialistaion de l'interface */
	ncurses_initialiser(); 
	ncurses_initcouleurs();
  	ncurses_initsouris(); 

	fenetre_affch(NULL, "Pressez F2 pour fermer ce client d'affichage\n");
	
	/* Fenetre de la carte */
	*win_carte = fenetre_creer(nbLignesAff*TAILLE_ZONE+2, nbColonnesAff*TAILLE_ZONE+2, 1, 0, "Carte", 1);
	carte_afficher_fenetre(win_carte, carte, nbLignesAff, nbColonnesAff);
}

/**
 * Affichage d'une carte dans une fenetre ncurses.
 * @param fenetre la fenetre dans laquelle afficher la carte
 * @param carte la carte a afficher
 * @param nbLignesAff le nombre de lignes de zones affichees par le client
 * @param nbColonnesAff le nombre de colonnes de zones affichees par le client
 **/
void carte_afficher_fenetre(fenetre_t *fenetre, Zone*** carte, int nbLignesAff, int nbColonnesAff) {
	int i, j, k, l;
	
	for(k = 0; k < nbLignesAff; k++) {
	     for(l=0; l < nbColonnesAff; l++) {
		  for(i = 0; i < TAILLE_ZONE; i++) {
		    /* On se place au bon endroit pour commencer a ecrire la ligne de cases selon la zone */
		    fenetre_setpos(fenetre, l*TAILLE_ZONE, k*TAILLE_ZONE+i);
		    for(j = 0; j < TAILLE_ZONE; j++) {
		      carte_afficher_fenetre_case(fenetre, carte[k][l]->mat[j][i]);
		    }
		  }
	     }
	}
  	fenetre_maj(fenetre);
}

/**
 * Affichage d'une case de la carte dans une fenetre ncurses.
 * @param fenetre la fenetre
 * @param case le contenu de la case
 */
void carte_afficher_fenetre_case(fenetre_t *fenetre, Case c) {
  int type = TYPE(c.type);
  fenetre_affcar(fenetre, c.type, COL(type));
}

int main(int argc, char* argv[]) {
	int i, j, c;
	struct sembuf opZone;
	struct sembuf opVoiture;
	int* debut_shm;
	int ch;
	int sourisx, sourisy, x, y, ligneZone, colZone, xZone, yZone;	
	pthread_t thread_feu;
	pthread_t lecteur_messages;
	int statut;
	int numVoiture = 0;
	requete_t requete;

	srand(time(NULL));
	
	/* Demande à l'utilisateur la zone a affichee */
	printf("Veuillez entrer la zone a afficher (1,2,3 ou 4) : \n");
	while((numClient < 1) || (numClient > 4)) {
		if(scanf("%d", &numClient) != 1) {
			printf("Vous devez entrer une valeur entre 1 et 4 : \n");
		}
		while ((c = getchar ()) != '\n' && c != EOF);
	}
	
	/* Recuperation du tableau de semaphore */
	semid = recuperation_sem ((key_t)CLE_SEM);

	/* Recuperation de la file de message */
	msqid = recuperation_msq ((key_t)CLE_MSG);

	/* Recuperation de la carte dans le segment de memoire partagee */
	recuperer_carte(&carte, numClient, &nbColonnesAff, &nbLignesAff, &debut_shm);
	
	/* Preparation des cases pour les threads (allocation de la grille) */
	grille = (case_t**)malloc(sizeof(case_t*)*(nbColonnesAff*TAILLE_ZONE));
	for(i=0; i < nbColonnesAff*TAILLE_ZONE; i++) {
		grille[i] = (case_t*)malloc(sizeof(case_t)*(nbLignesAff*TAILLE_ZONE));
		for(j=0; j < nbLignesAff*TAILLE_ZONE; j++) {
			statut = pthread_mutex_init(&(grille[i][j].mutex), NULL);
			statut = pthread_cond_init(&(grille[i][j].cond), NULL);
			if(statut != 0){
				fprintf(stderr, "Probleme initialisation mutex sur case\n");	
				exit(EXIT_FAILURE);	
			}
			grille[i][j].element = VIDE;
		}
	}


	/* Initialisation du tableau indiquant si les threads sont mort ou vivant */
	for(i=0; i < MAX_VOITURES; i++) {
		thread_vivant[i] = 0;
	}

	/* Creation de l'interface */
	creation_interface(&win_carte, carte, nbLignesAff, nbColonnesAff);

	/* Creation du thread pour les feux */
	statut = pthread_create(&thread_feu, NULL, routineFeu, NULL);
	if(statut != 0){
		fprintf(stderr, "Probleme creation thread feu\n");	
		exit(EXIT_FAILURE);	
	}

	/* Creation du thread lecteur de message */
	statut = pthread_create(&lecteur_messages, NULL, routine_Lecteur, NULL);
	if(statut != 0){
		fprintf(stderr, "Probleme creation thread lecteur\n");	
		exit(EXIT_FAILURE);	
	}
	
	/* BOUCLE PRINCIPALE */
	while( (ch = getch()) != KEY_F(2) ) {
	   switch(ch) {
	   case KEY_MOUSE:
	      if(souris_getpos(&sourisx, &sourisy) == OK) {
	      	/* Clic dans la fenetre de la carte */
		if(fenetre_getcoordonnees(&win_carte, sourisx, sourisy, &x, &y) == 1) {
		  zone_getcoordonnees(x, y, &ligneZone, &colZone, &xZone, &yZone);
		  if( (carte[ligneZone][colZone]->mat[xZone][yZone].type == CAR(HAUT)) || 
		      (carte[ligneZone][colZone]->mat[xZone][yZone].type == CAR(BAS))  ||
		      (carte[ligneZone][colZone]->mat[xZone][yZone].type == CAR(GAUCHE))  ||
		      (carte[ligneZone][colZone]->mat[xZone][yZone].type == CAR(DROITE)) ||
		      (carte[ligneZone][colZone]->mat[xZone][yZone].type == CAR(CROISEMENT)) ) {
			pthread_mutex_lock(&(grille[x][y].mutex));
			if(grille[x][y].element != VOITURE) { 
				/* Creation du thread de la voiture */
				/* On indique la de creation de la voiture au coordinateur */
				requete.type = 1;
				sprintf(requete.message, "Creation d'une voiture dans la zone numero %d en %d %d\n", numClient, x, y);
				snd_requete(msqid, &requete);
				/* On baisse le nombre possible de voitures encore creable */
				opVoiture.sem_num = 4;
				opVoiture.sem_op = -1;
				opVoiture.sem_flg = IPC_NOWAIT;
				if(semop(semid, &opVoiture, 1) == -1) {
				    if(errno == EAGAIN) { /* Si le nombre maximum de voitures est atteint alors on envoie un message au coordinateur */
					requete.type = 1;
					sprintf(requete.message, "Creation de voiture rejetee : nombre maximum de voiture atteint\n");
					snd_requete(msqid, &requete);
				    }
				    else{
					    perror("Erreur lors de l'operation sur le semaphore ");
					    exit(EXIT_FAILURE);
				    }
				}
				else {
					pthread_mutex_lock(&mutexCreation);
					while(thread_vivant[numVoiture] == 1){
						numVoiture++;
					}
					pthread_mutex_unlock(&mutexCreation);
					thread_vivant[numVoiture] = 1;
					argsThreads[numVoiture].coordV.x = x;
					argsThreads[numVoiture].coordV.y = y;
					argsThreads[numVoiture].numThread = numVoiture;
					argsThreads[numVoiture].vitesse = (rand()%10 + 1);
					statut = pthread_create(&threadsVoiture[numVoiture], NULL, routineVoiture, (void *)(&(argsThreads[numVoiture])));
					if(statut != 0){
						fprintf(stderr, "Probleme creation thread voiture\n");	
						exit(EXIT_FAILURE);	
					}
					grille[x][y].element = VOITURE;
					grille[x][y].voiture = &threadsVoiture[numVoiture]; 
					/* Mise a jour de l'affichage */
					pthread_mutex_lock(&mutexAff);
					fenetre_setpos(&win_carte, x, y);
				    	fenetre_affcar(&win_carte, CAR(VOITURE), COL(VOITURE));
					fenetre_maj(&win_carte);
					pthread_mutex_unlock(&mutexAff);
					numVoiture = 0;
				}
			}
			else { /* S'il y a une voiture on la supprime */
				/* Demande d'annulation du thread et attente de la recuperation */
				pthread_cancel(*(grille[x][y].voiture));
				/* On libere le mutex pour eviter un interblocage avec le thread de la voiture si ce dernier essaie de verouiller le mutex avant un deplacement */
				pthread_mutex_unlock(&(grille[x][y].mutex));
				statut = pthread_join(*(grille[x][y].voiture), NULL);
				if(statut != 0) {
					fprintf(stderr, "Probleme destruction du thread voiture\n");	
					exit(EXIT_FAILURE);	
				}
				/* On indique la mort du thread au coordinateur */
				/* En envoyant un message */
				requete.type = 1;
				sprintf(requete.message, "Suppression d'une voiture sur le client numero %d \n", numClient);
				snd_requete(msqid, &requete);
				/* Et en augmentant le nombre de voitures possible de creer */
				opVoiture.sem_num = 4;
				opVoiture.sem_op = 1;
				opVoiture.sem_flg = 0;
				if(semop(semid, &opVoiture, 1) == -1) {
				    perror("Erreur lors de l'operation sur le semaphore ");
				    exit(EXIT_FAILURE);
				}
				pthread_mutex_lock(&(grille[x][y].mutex));
				grille[x][y].element = VIDE;
				grille[x][y].voiture = NULL;
				/* On met a jour l'affichage */
				pthread_mutex_lock(&mutexAff);
				fenetre_setpos(&win_carte, x, y);
				fenetre_affcar(&win_carte, carte[ligneZone][colZone]->mat[xZone][yZone].type, COL(TYPE(carte[ligneZone][colZone]->mat[xZone][yZone].type)));
				fenetre_maj(&win_carte);
				pthread_mutex_unlock(&mutexAff);
				/* On signal la place libre a une possible voiture en attente */
				pthread_cond_signal(&(grille[x][y].cond));
			}
			pthread_mutex_unlock(&(grille[x][y].mutex));
		  }
	        }
	      }
   	      break;
  	   }
	}	

	/* On indique la destruction d'un client au coordinateur */
	requete.type = 1;
	sprintf(requete.message, "Destruction d'un client pour la zone numero %d\n", numClient);
	snd_requete(msqid, &requete);

	/* Destruction du thread de feux */
	pthread_cancel(thread_feu);
	statut = pthread_join(thread_feu, NULL);
	if(statut != 0) {
		fprintf(stderr, "Probleme destruction du thread feu\n");	
		exit(EXIT_FAILURE);	
	}
	
	/* Destruction du thread lecteur */
	pthread_cancel(lecteur_messages);
	statut = pthread_join(lecteur_messages, NULL);
	if(statut != 0) {
		fprintf(stderr, "Probleme destruction du thread feu\n");	
		exit(EXIT_FAILURE);	
	}

	/* Destruction des threads voitures */
	i = 0;
	while(i < MAX_VOITURES){
		if(thread_vivant[i] == 1) {
			pthread_cancel(threadsVoiture[i]);
			statut = pthread_join(threadsVoiture[i], NULL);
			if(statut != 0) {
				fprintf(stderr, "Probleme destruction du thread voiture\n");	
				exit(EXIT_FAILURE);	
			}
			/* On indique la voiture morte */
			opVoiture.sem_num = 4;
			opVoiture.sem_op = 1;
			opVoiture.sem_flg = 0;
			if(semop(semid, &opVoiture, 1) == -1) {
			    perror("Erreur lors de l'operation sur le semaphore ");
			    exit(EXIT_FAILURE);
			}
		}
		i++;
	}

	/* Liberation de la grille et destruction des mutex */
	for(i=0; i < nbColonnesAff*TAILLE_ZONE; i++) {
		for(j=0; j < nbLignesAff*TAILLE_ZONE; j++) {
			statut = pthread_cond_destroy(&(grille[i][j].cond));
			if(statut != 0){
				fprintf(stderr, "Probleme destruction mutex sur case\n");	
				exit(EXIT_FAILURE);	
			}
			statut = pthread_mutex_destroy(&(grille[i][j].mutex));
			if(statut != 0){
				fprintf(stderr, "Probleme destruction mutex sur case\n");	
				exit(EXIT_FAILURE);	
			}
		}
		free(grille[i]);
	}
	free(grille);
	
	/* Liberation de la carte */
	for(i=0; i < nbLignesAff; i++) {
		free(carte[i]);
	}
	free(carte);

	/* On remet la zone d'affichage disponible */
	opZone.sem_num = numClient-1;
	opZone.sem_op = 1;
	opZone.sem_flg = 0;
	if(semop(semid, &opZone, 1) == -1) {
	    perror("Erreur lors de l'operation sur le semaphore ");
	    exit(EXIT_FAILURE);
	}

	/* Detachement du segment de memoire partage */
	detachement_shm ((void*)debut_shm);

	/* Fin de ncurses */
	fenetre_detruire(&win_carte);
	ncurses_stopper();
	
	return EXIT_SUCCESS;
}
	
