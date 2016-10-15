#ifndef _IPC_
#define _IPC_

#define CLE_SHM 2000
#define CLE_MSG 1098
#define CLE_SEM 1099

/* Structure utilisee pour les requetes dans la file de messages*/
typedef struct {
	long type;
	int x; /* la valeur -1 signifie que la voiture apparaitra a la coordonnee 0, -2 signifie a la coordonnee maximum */
	int y; /* la valeur -1 signifie que la voiture apparaitra a la coordonnee 0, -2 signifie a la coordonnee maximum */
	int vitesse; 
	char message [80]; 
} requete_t;

/** Creation d'un segment de memoire partage 
 * @param taille la taille du segment a creer
 * @return l'identifiant du segment cree
 **/
int creation_shm (size_t taille);

/** Recuperation d'un segment de memoire partage 
 * @return l'identifiant du segment 
 **/
int recuperation_shm ();

/** Attachement d'un segment de memoire partage 
 * @param shmid l'identifiant du segment
 * @return l'adresse du segment 
 **/
void* attachement_shm (int shmid);

/** Detachement d'un segment de memoire partage  
 * @param adresse l'adresse du segment
 **/
void detachement_shm (void* adresse);

/** Suppression d'un segment de memoire partage  
 * @param shmid l'identifiant du segment
 **/
void suppression_shm (int shmid);

/* TABLEAU DE SEMAPHORE */

/** Creation d'un tableau de semaphore
 * @param nbSem le nombre de semaphore dans le tableau
 * @param CLE la du tableau
 * @return l'identifiant du tableau cree
 **/
int creation_sem (key_t CLE, int nbSem);

/** Recuperation du tableau de semaphore 
 * @param CLE la cle du tableau
 * @return semid l'identifiant du tableau
 **/
int recuperation_sem (key_t CLE);

/** initialisation d'un tableau de semaphore 
 * @param semid l'identifiant du tableau
 * @param val le tableau avec les valeurs d'initialisation
 **/
void initialisation_sem (int semid, unsigned short val []);

/** Suppression d'un tableau de semaphore 
 * @param semid l'identifiant du tableau
 **/
void suppression_sem (int semid);

/* FIL DE MESSAGES */

/** Creation d'une file de messages
 * @param CLE la cle de la file
 * @return l'identifiant de la file creee
 **/
int creation_msq (key_t CLE);

/** Recuperation de la file de messages
 * @param CLE la cle de la file
 * @return l'identifiant de la file
 **/
int recuperation_msq (key_t CLE);

/** Suppression d'une file de messages
 * @param msqid l'identifiant de la file de message
 **/
void suppression_msq (int msqid);

/** reception d'un message dans une file de message
 * @param msqid l'identifiant de la file de message
 * @param requete pointeur pour stocker le message reçu
 * @param type le type du message a ecouter
 **/
void rcv_requete (int msqid, requete_t* requete, int type);

/** envoie d'un message dans une file de message
 * @param msqid l'identifiant de la file de message
 * @param requete pointeur vers le message a envoyer
 **/
void snd_requete (int msqid, requete_t* requete);

#endif
