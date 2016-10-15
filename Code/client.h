#ifndef _CLIENT_
#define _CLIENT_

#include "interface.h"
#include "zone.h"
#include "IPC.h"

#define MAX_VOITURES 5

typedef struct case_tag {
	int element;
	pthread_t *voiture;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} case_t;

typedef struct coordGrille_tag {
	int x;
	int y;
} coordGrille_t;

typedef struct args_thread_tag {
	coordGrille_t coordV;
	int numThread;
	int vitesse;
} args_thread;

/** 
  * Routine pour liberer un mutex que l'on peu ajouter a la pile de pthread_cleanup 
  * @param coord les coordonnee de la case dont on souhaite liberer le mutex
 **/
void unlock (void* coord);

/** 
  * Methode appelee par la methode deplacement pour effectuer les deplacements lorsque l'on reste dans la meme zone d'affichage
  * @param args les arguments du thread de la voiture
  * @param x la coordonnee x de la case a partir de laquelle on appelle la methode (case de la voiture, feu, stop)
  * @param y la coordonnee x de la case a partir de laquelle on appelle la methode (case de la voiture, feu, stop)
  * @param direction la direction dans laquelle veut bouger la voiture
  * @return 1 pour dire que la voiture doit etre detruite
 **/
int dep_out_client(args_thread* args, int x, int y, char direction);

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
int dep_in_client (args_thread* args, int x, int y, int xSuiv, int ySuiv, char direction);

/** 
  * Methode qui place sur la carte une voiture envoyee par un autre client
  * @param args les arguments du thread de la voiture
  * @param x la coordonnee x de la case a partir de laquelle on appelle la methode (case de la voiture, feu, stop)
  * @param y la coordonnee x de la case a partir de laquelle on appelle la methode (case de la voiture, feu, stop)
  * @param premier_appel vaut 0 si c'est le premier_appel a la fonction, vaut 1 si c'est le deuxieme appel suite a un feu ou un stop
  * @param direction la direction dans laquelle souhaite se deplacer la voiture : connu que si premier_appel vaut 0
  * @param premier_appel vaut 1 si c'est le premier appel a la fonction, vaut 0 si ce n'est pas le premier appel (apres un feu ou un stop)
 **/
void dep_spe (args_thread* args, int x, int y, char direction, int premier_appel);

/** 
  * Methode de la routine des voitures qui effectue les deplacements 
  * @param args les arguments du thread de la voiture
  * @param x la coordonnee x de la case a partir de laquelle on appelle la methode (case de la voiture, feu, stop)
  * @param y la coordonnee x de la case a partir de laquelle on appelle la methode (case de la voiture, feu, stop)
  * @param direction la direction dans laquelle veut bouger la voiture
  * @return l'etat au deplacement
 **/
int deplacement (args_thread* args, int x, int y, char direction);

void *routineVoiture(void *arg);

/* Routine du thread en charge d'effectuer le changement de couleur des feux sur ce client */
void *routineFeu(void *arg);

/* Routine du thread en charge de recuperer les voitures transmise par un autre client */
void *routine_Lecteur(void *arg);

/**
 * Recupere la zone d'affichage dans le segment de memoire partagee
 * @param carte la carte
 * @param numClient le numero du client d'affichage
 * @param nbColonnesAff le nombre de colonnes de zones affichees par le client
 * @param nbLignesAff le nombre de lignes de zones affichees par le client
 * @param debut_shm l'adresse du debut du segment de memoire partagee
 */
void recuperer_carte(Zone**** carte, int numClient, int* nbColonnesAff, int* nbLignesAff, int** debut_shm);

/**
 * Creation de l'interface principale
 * @param win_carte la fenetre contenant la carte
 * @param carte la carte a afficher
 * @param nbLignesAff le nombre de lignes de zones affichees par le client
 * @param nbColonnesAff le nombre de colonnes de zones affichees par le client
 */
void creation_interface(fenetre_t *win_carte, Zone*** carte, int nbLignesAff, int nbColonnesAff);

/**
 * Affichage d'une carte dans une fenetre ncurses.
 * @param fenetre la fenetre dans laquelle afficher la carte
 * @param carte la carte a afficher
 * @param nbLignesAff le nombre de lignes de zones affichees par le client
 * @param nbColonnesAff le nombre de colonnes de zones affichees par le client
 **/
void carte_afficher_fenetre(fenetre_t *fenetre, Zone*** carte, int nbLignesAff, int nbColonnesAff);

/**
 * Affichage d'une case de la carte dans une fenetre ncurses.
 * @param fenetre la fenetre
 * @param case le contenu de la case
 */
void carte_afficher_fenetre_case(fenetre_t *fenetre, Case c);

#endif
