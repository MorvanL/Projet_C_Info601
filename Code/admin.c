#define _XOPEN_SOURCE 500 

#include <stdlib.h>     /* Pour exit, EXIT_FAILURE, EXIT_SUCCESS */
#include <stdio.h>      /* Pour printf, perror */
#include <sys/msg.h>    /* Pour msgget, msgsnd, msgrcv */
#include <sys/sem.h>  /* Pour semget, semctl, semop */
#include <errno.h>      /* Pour errno */
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>      /* Pour open */
#include <pthread.h>

#include "admin.h"

int semid;
int msqid;
fenetre_t win_info;

/**
 * Creation de l'interface principale
 */
void creation_interface() {

	/* Initialistaion de l'interface */
	ncurses_initialiser(); 
	ncurses_initcouleurs();
  	ncurses_initsouris();

	fenetre_affch(NULL, "Pressez F2 pour fermer l'administrateur\n");

	/* Fenetre affichant les informations sur la simulation */
	win_info = fenetre_creer(LINES-1, COLS, 1, 0, "Informations", 0);
	fenetre_maj(&win_info); 
}

/**
 * Creer un segment de memoire partagee et y place la carte 
 * @param fich le fichier contenant la carte
 * @param carte la carte
 * @param nbColonnes le nombre de colonnes de zones dans la carte
 * @param nbLignes le nombre de lignes de zones dans la carte
 * @param debut_shm l'adresse du debut du segment de memoire partagee
 * @param shmid l'identifiant du segment de memoire partagee
 */
void charger_carte (char* fich, Zone** carte, int* nbColonnes, int* nbLignes, int** debut_shm, int* shmid) {
	int fd;
	int i;
	
	/* Ouverture  du fichier carte */
	if((fd = open(fich, O_RDWR)) == -1) {
		perror("Erreur, recuperation_carte : erreur lors de la creation du fichier carte ");
		exit(EXIT_FAILURE);
	}

	/* Lecture du nombre de colonne et de ligne de zones */
    	if(read(fd, nbColonnes, sizeof(int)) == -1) {
		perror("Erreur, recuperation_carte : erreur lors de la lecture dans le fichier");
		exit(EXIT_FAILURE);
   	}
    	if(read(fd, nbLignes, sizeof(int)) == -1) {
		perror("Erreur, recuperation_carte : erreur lors de la lecture dans le fichier");
		exit(EXIT_FAILURE);
    	}

	/* Creation, attachement et initialisation du segment de memoire partagé */
	*shmid = creation_shm (sizeof(int)*2 + sizeof(Zone)*(*nbColonnes)*(*nbLignes));
	*debut_shm = (int*)attachement_shm (*shmid);
	*(*debut_shm) = *nbColonnes;
	*(*debut_shm+1) = *nbLignes;
	*carte = (Zone*)(*debut_shm+2);
	for(i=0; i < (*nbColonnes)*(*nbLignes); i++) {
		Zone_charger(&((*carte)[i]), i+1, fd);
	}
}

/* Routine du thread en charge de decider du changement de couleur des feux */
void *routineFeu(void *arg) {
	int cpt = 1;
	
	while(1) {
		sleep(5);
		/* indique quels sont les feux qui passent au vert */
		if(semctl(semid, 5, SETVAL, cpt) == -1) {
			perror("Erreur lors des operations sur les semaphores de feux cote admin");
			exit(EXIT_FAILURE);
		}
		/* Puis on donne le signal de changement de couleur */
		if(semctl(semid, 6, SETVAL, 0) == -1) {
			perror("Erreur lors des operations sur les semaphores de feux cote admin");
			exit(EXIT_FAILURE);
		}
		cpt++;
		if( cpt > 4 )
			cpt = 1;
	}

	pthread_exit(NULL);
}

/* Routine du thread en charge de recuperer les informations sur l'execution de la simulation */
void* routineLecteur(void* arg) {
	requete_t requete;

	while(1) {
		/* Attente de reception d'un message */
		rcv_requete(msqid, &requete, 0);

		/* traitement du message reçu => affichage de l'information */
		fenetre_affch(&win_info, requete.message);
		fenetre_maj(&win_info); 
	}

	pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
	int shmid;
	int* debut_shm;
	unsigned short val_sem[7] = {1, 1, 1, 1, MAX_VOITURES, 0, 1}; /* les 4 premiers semaphores correspondent aux zones d'affichage, le 5eme correspond au nombre de voitures que l'on peut creer, les 2 derniers permettent la gestion des feux */
	int nbColonnes, nbLignes;
	Zone* carte;
	int ch;
	pthread_t thread_feu, thread_lecteur;
	int statut;

	/* Verification du nombre d'arguments */
   	if(argc != 2) {    
 		fprintf(stderr, "* Usage: %s carte.bin\n", argv[0]);
		fprintf(stderr, "* carte.bin : nom du fichier contenant la carte.\n");
		exit(EXIT_FAILURE);
    	}
	
	/* Creation du segment de memoire partagee et chargement de la carte dans celui-ci */
	charger_carte(argv[1], &carte, &nbColonnes, &nbLignes, &debut_shm, &shmid);
	/* Creation et initialisation du tableau de semaphore */
	semid = creation_sem((key_t)CLE_SEM, 7);
	initialisation_sem(semid, val_sem);
	/* Creation de la file de message */
	msqid = creation_msq ((key_t)CLE_MSG);

	/* Creation du thread effectuant la gestion des feux */
	statut = pthread_create(&thread_feu, NULL, routineFeu, NULL);
	if(statut != 0){
		fprintf(stderr, "Probleme creation thread feu\n");	
		exit(EXIT_FAILURE);	
	}

	/* Creation du thread lecteur de messages */
	statut = pthread_create(&thread_lecteur, NULL, routineLecteur, NULL);
	if(statut != 0){
		fprintf(stderr, "Probleme creation thread feu\n");	
		exit(EXIT_FAILURE);	
	}

	/* Creation de l'interface */
	creation_interface(win_info);

	while((ch = getch()) != KEY_F(2));

	/* Fin de ncurses */
	fenetre_detruire(&win_info);
	ncurses_stopper();
	
	/* Destruction du thread de gestion des feux */
	pthread_cancel(thread_feu);
	statut = pthread_join(thread_feu, NULL);
	if(statut != 0) {
		fprintf(stderr, "Probleme destruction du thread feu\n");	
		exit(EXIT_FAILURE);	
	}

	/* Destruction du thread lecteur de messages */
	pthread_cancel(thread_lecteur);
	statut = pthread_join(thread_lecteur, NULL);
	if(statut != 0) {
		fprintf(stderr, "Probleme destruction du thread lecteur\n");	
		exit(EXIT_FAILURE);	
	}

	/* detachement et suppression du segment de memoire partage */
	detachement_shm((void*)debut_shm);
	suppression_shm(shmid);
	/* Suppression du tableau de semaphore */
	suppression_sem(semid);
	/* Suppression de la file de messages */
	suppression_msq(msqid);

 	return EXIT_SUCCESS;
}
  
