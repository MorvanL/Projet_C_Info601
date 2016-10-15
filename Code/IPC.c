#define _XOPEN_SOURCE

#include <stdlib.h>     /* Pour exit, EXIT_FAILURE, EXIT_SUCCESS */
#include <stdio.h>      /* Pour printf, perror */
#include <sys/shm.h>    /* Pour msgget, msgsnd, msgrcv */
#include <sys/sem.h>  /* Pour semget, semctl, semop */
#include <sys/msg.h>    /* Pour msgget, msgsnd, msgrcv */
#include <errno.h>      /* Pour errno */
#include <sys/stat.h>   /* Pour S_IRUSR, S_IWUSR */

#include "IPC.h"


/* SEGMENT DE MEMOIRE PARATGEE */

/** Creation d'un segment de memoire partage 
 * @param taille la taille du segment a creer
 * @return l'identifiant du segment cree
 **/
int creation_shm (size_t taille) {
  int shmid;
  if((shmid = shmget((key_t)CLE_SHM, taille, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL)) == -1) {
    if(errno == EEXIST)
      fprintf(stderr, "Le segment de memoire partagee (cle=%d) existe deja\n", CLE_SHM);
    else
      perror("Erreur lors de la creation du segment de memoire ");
    exit(EXIT_FAILURE);
  }
  return shmid;
}


/** Recuperation d'un segment de memoire partage 
 * @return l'identifiant du segment 
 **/
int recuperation_shm () { 
  int shmid;
  if((shmid = shmget((key_t)CLE_SHM, 0, 0)) == -1) {
    perror("Erreur lors de la recuperation du segment de memoire partagee ");
    exit(EXIT_FAILURE);
  }
  return shmid;
}

/** Attachement d'un segment de memoire partage 
 * @param shmid l'identifiant du segment
 * @return l'adresse du segment 
 **/
void* attachement_shm (int shmid) {
    void* adresse;
    if((adresse = shmat(shmid, NULL, 0)) == (void*)-1) {
    	perror("Erreur lors de l'attachement du segment de memoire partagee ");
    	exit(EXIT_FAILURE);
    }
    return adresse;
}

/** Detachement d'un segment de memoire partage  
 * @param adresse l'adresse du segment
 **/
void detachement_shm (void* adresse) {
  if(shmdt(adresse) == -1) {
    perror("Erreur lors du detachement du segment de memoire partagee ");
    exit(EXIT_FAILURE);
  }
}

/** Suppression d'un segment de memoire partage  
 * @param shmid l'identifiant du segment
 **/
void suppression_shm (int shmid) {
  if(shmctl(shmid, IPC_RMID, 0) == -1) {
    perror("Erreur lors de la suppression du segment de memoire partagee ");
    exit(EXIT_FAILURE);
  }
}


/* TABLEAU DE SEMAPHORE */

/** Creation d'un tableau de semaphore
 * @param nbSem le nombre de semaphore dans le tableau
 * @param CLE la cle du tableau
 * @return l'identifiant du tableau cree
 **/
int creation_sem (key_t CLE, int nbSem) {
  int semid;
  if((semid = semget(CLE, nbSem, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL)) == -1) {
    if(errno == EEXIST)
      fprintf(stderr, "Tableau de semaphores (cle=%d) existant\n", CLE);
    else
      perror("Erreur lors de la creation du tableau de semaphores ");
    exit(EXIT_FAILURE);
  }
  return semid;
}

/** Recuperation du tableau de semaphore 
 * @param CLE la cle du tableau
 * @return semid l'identifiant du tableau
 **/
int recuperation_sem (key_t CLE) { 
  int semid;
  if((semid = semget(CLE, 0, 0)) == -1) {
    perror("Erreur lors de la recuperation du tableau de semaphores ");
    exit(EXIT_FAILURE);
  }
  return semid;
}

/** initialisation d'un tableau de semaphore 
 * @param semid l'identifiant du tableau
 * @param val le tableau avec les valeurs d'initialisation
 **/
void initialisation_sem (int semid, unsigned short val []) {
  if(semctl(semid, 0, SETALL, val) == -1) {
    perror("Erreur lors de l'initialisation des semaphores ");
    exit(EXIT_FAILURE);
  }
}

/** Suppression d'un tableau de semaphore 
 * @param semid l'identifiant du tableau
 **/
void suppression_sem (int semid) {
  if(semctl(semid, 0, IPC_RMID) == -1) {
    perror("Erreur lors de la suppresion du tableau de semaphores ");
    exit(EXIT_FAILURE);
  }
}

/* FIL DE MESSAGES */

/** Creation d'une file de messages
 * @param CLE la cle de la file
 * @return l'identifiant de la file creee
 **/
int creation_msq (key_t CLE) {
  int msqid;
  if((msqid = msgget(CLE, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL)) == -1) {
    if(errno == EEXIST)
      fprintf(stderr, "Erreur : file (cle=%d) existante\n", CLE);
    else
      perror("Erreur lors de la creation de la file ");
    exit(EXIT_FAILURE);
  }
  return msqid;
}

/** Recuperation de la file de messages
 * @param CLE la cle de la file
 * @return l'identifiant de la file
 **/
int recuperation_msq (key_t CLE) { 
  int msqid;
  if((msqid = msgget(CLE, 0)) == -1) {
    perror("Erreur lors de la recuperation de la file ");
    exit(EXIT_FAILURE);
  }
  return msqid;
}


/** Suppression d'une file de messages
 * @param msqid l'identifiant de la file de message
 **/
void suppression_msq (int msqid) {
  if(msgctl(msqid, IPC_RMID, 0) == -1) {
    perror("Erreur lors de la suppression de la file ");
    exit(EXIT_FAILURE);
  }
}

/** reception d'un message dans une file de message
 * @param msqid l'identifiant de la file de message
 * @param requete pointeur pour stocker le message reçu
 * @param type le type du message a ecouter
 **/
void rcv_requete (int msqid, requete_t* requete, int type) { 
	if(msgrcv(msqid, requete, sizeof(requete_t) - sizeof(long), type, 0) == -1) {
	    perror("Erreur lors de la reception d'une requete ");
	    exit(EXIT_FAILURE);
	}
}

/** envoie d'un message dans une file de message
 * @param msqid l'identifiant de la file de message
 * @param requete pointeur vers le message a envoyer
 **/
void snd_requete (int msqid, requete_t* requete) { 
	if(msgsnd(msqid, requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
	    perror("Erreur lors de l'envoi d'une requete ");
	    exit(EXIT_FAILURE);
	}	
}




















