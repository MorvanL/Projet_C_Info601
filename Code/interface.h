#ifndef _INTERFACE_
#define _INTERFACE_

#include <ncurses.h>

/**
 * Structure representant une fenetre. Elle est constituee en fait de deux
 * fenetres ncurses. L'une correspond au bord et l'autre correspond a
 * l'interieur.
 */
typedef struct {
  int posx, posy;
  int largeur, hauteur;
  WINDOW *bord;
  WINDOW *interieur;
} fenetre_t;

/**
 * Initialisation de ncurses.
 */
void ncurses_initialiser();

/**
 * Initialisation des couleurs et de la palette.
 */
void ncurses_initcouleurs();

/**
 * Initialisation de la souris.
 */
void ncurses_initsouris();

/**
 * Verification des dimensions du terminal.
 * @param largeur la largeur attendue
 * @param hauteur la hauteur attendue
 * @return 1 si les dimensions sont bonnes, - 1 sinon
 */
int ncurses_verifiedim(int largeur, int hauteur);

/**
 * Fin de ncurses.
 */
void ncurses_stopper();

/**
 * Cree une nouvelle fenetre dans la fenetre principale.
 * @param hauteur la hauteur de la fenetre
 * @param largeur la largeur de la fenetre
 * @param posy la position y du coin haut bas
 * @param posx la position x du coin haut gauche
 * @param titre le titre de la fenetre
 * @param scroll autorise le scroll (si 1) ou non
 * @return la fenetre creee
 */
fenetre_t fenetre_creer(int hauteur, int largeur, int posy, int posx, char *titre, int scroll);

/**
 * Detruit la fenetre.
 * @param fenetre la fenetre a detruire
 */
void fenetre_detruire(fenetre_t *fenetre);

/**
 * Met a jour l'affichage de la fenetre.
 * @param fenetre la fenetre
 */
void fenetre_maj(fenetre_t *fenetre);

/**
 * Verifie si des coordonnees globales se trouvent dans une fenetre.
 * @param fenetre la fenetre
 * @param x coordonnee en x
 * @param y coordonnee en y
 * @return 1 si oui, -1 sinon
 */
int fenetre_estdans(fenetre_t *fenetre, int x, int y);

/**
 * Recupere depuis des coordonnees globales, les coordonnees au sein d'une 
 * fenetre.
 * @param fenetre la fenetre
 * @param posx coordonnees globale x
 * @param poxy coordonnees globale y
 * @param x coordonnee calculee en x
 * @param y coordonnee calculee en y
 * @return 1 si les coordonnees sont dans la fenetre, -1 sinon
 */
int fenetre_getcoordonnees(fenetre_t *fenetre, int posx, int posy, int *x, int *y);

/*
 * Recupere depuis des coordonnees de la fenetre_carte, les coordonnees au sein d'une 
 * zone ainsi que la zone correspondante.
 * @param posx coordonnees x au sein de la fenetre
 * @param poxy coordonnees y au sein de la fenetre
 * @param numLigneZone la ligne numero de la ligne de zone de la zone dans le client d'affichage
 * @param numColZone la ligne numero de la colonne de zone de la zone dans le client d'affichage
 * @param xZone coordonnee x au sein de la zone
 * @param yZone coordonnee y au sein de la zone
 */
void zone_getcoordonnees(int posx, int posy, int* numLigneZone, int* numColZone, int* xZone, int* yZone);

/**
 * Affiche une chaine de caracteres dans la fenetre a la position actuelle.
 * @param fenetre la fenetre
 * @param str la chaine
 */
void fenetre_affch(fenetre_t *fenetre, char *chaine, ...);

/**
 * Affiche une chaine de caracteres dans la fenetre a la position actuelle.
 * @param fenetre la fenetre
 * @param posx la position en x
 * @param posy la position en y
 * @param str la chaine
 */
void fenetre_affch_pos(fenetre_t *fenetre, int posx, int posy, char *chaine, ...);

/**
 * Affiche un caractere dans la fenetre a la position actuelle et dans la
 * couleur specifiee.
 * @param fenetre la fenetre
 * @param car le caractere
 * @param num le numero de la couleur
 */
void fenetre_affcar(fenetre_t *fenetre, char car, int num);

/**
 * Deplace le curseur a la position specifiee.
 * @param fenetre la fenetre
 * @param 
 */
void fenetre_setpos(fenetre_t *fenetre, int x, int y);

/**
 * Recupere la position x et y de la souris.
 * @param[out] x la position en x
 * @param[out] y la position en y
 * @return OK si reussite
 */
int souris_getpos(int *x, int *y);

#endif
