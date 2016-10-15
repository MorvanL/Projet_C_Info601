#include <stdlib.h>     /* Pour exit, EXIT_FAILURE */
#include <stdarg.h>     /* Pour va_list, va_start, va_end */

#include "interface.h"
#include "zone.h"

/**
 * Initialisation de ncurses.
 */
void ncurses_initialiser() {
  initscr();	        /* Demarre le mode ncurses */
  cbreak();	        /* Pour les saisies clavier */
  noecho();             /* Desactive l'affichage des caracteres saisis */
  keypad(stdscr, TRUE);	/* Active les touches specifiques */
  curs_set(FALSE);      /* Desactive le curseur */
  refresh();            /* Met a jour l'affichage */
}

/**
 * Initialisation des couleurs et de la palette.
 */
void ncurses_initcouleurs() {
  /* Activation de la couleur (si possible !) */
  if(has_colors() == FALSE) {
    fprintf(stderr, "\033[1m[ERREUR]\033[0m Votre terminal ne supporte par la couleur\n");
    exit(EXIT_FAILURE);
  }  
  start_color();

  /* Definition de la palette */
  init_pair(1, COLOR_YELLOW, COLOR_MAGENTA);
  init_pair(2, COLOR_CYAN, COLOR_MAGENTA);
  init_pair(3, COLOR_RED, COLOR_MAGENTA);
  init_pair(7, COLOR_GREEN, COLOR_MAGENTA);
  init_pair(4, COLOR_RED, COLOR_MAGENTA);
  init_pair(5, COLOR_WHITE, COLOR_CYAN);
  init_pair(6, COLOR_WHITE, COLOR_BLACK);
}

/**
 * Initialisation de la souris.
 */
void ncurses_initsouris() {
  if(!mousemask(BUTTON1_PRESSED, NULL)) {
    fprintf(stderr, "\033[1m[ERREUR]\033[0m Erreur lors de l'initialisation de la souris.\n");
    exit(EXIT_FAILURE);
  }
}

/**
 * Verification des dimensions du terminal.
 * @param largeur la largeur attendue
 * @param hauteur la hauteur attendue
 * @return 1 si les dimensions sont bonnes, - 1 sinon
 */
int ncurses_verifiedim(int largeur, int hauteur) {
  int resultat = 1;

  if((COLS < largeur) || (LINES < hauteur))
    resultat = -1;

  return resultat;
}

/**
 * Fin de ncurses.
 */
void ncurses_stopper() {
  endwin();
}

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
fenetre_t fenetre_creer(int hauteur, int largeur, int posy, int posx, char *titre, int scroll) {
  fenetre_t fenetre;

  /* Memorisation des informations generales */
  fenetre.posx = posx;
  fenetre.posy = posy;
  fenetre.hauteur = hauteur;
  fenetre.largeur = largeur;

  /* Creation du bord avec le titre */
  fenetre.bord = newwin(hauteur, largeur, posy, posx);
  box(fenetre.bord, 0, 0);
  wprintw(fenetre.bord, titre);
  wrefresh(fenetre.bord);

  /* Creation du contenu */
  fenetre.interieur = newwin(hauteur - 2, largeur - 2, posy + 1, posx + 1);
  if(scroll != 1)
    scrollok(fenetre.interieur, TRUE);
  wrefresh(fenetre.interieur);
  wclear(fenetre.interieur);

  return fenetre;
}

/**
 * Detruit la fenetre.
 * @param fenetre la fenetre a detruire
 */
void fenetre_detruire(fenetre_t *fenetre) {
  /* On efface d'abord le contour de la fenetre */
  wborder(fenetre->interieur, ' ', ' ', ' ',' ',' ',' ',' ',' ');
  wrefresh(fenetre->interieur);

  /* On detruit les deux fenetres */
  delwin(fenetre->interieur);
  delwin(fenetre->bord);
}

/**
 * Met a jour l'affichage de la fenetre.
 * @param fenetre la fenetre
 */
void fenetre_maj(fenetre_t *fenetre) {
  wrefresh(fenetre->interieur);
}

/**
 * Verifie si des coordonnees globales se trouvent dans une fenetre.
 * @param fenetre la fenetre
 * @param x coordonnee en x
 * @param y coordonnee en y
 * @return 1 si oui, -1 sinon
 */
int fenetre_estdans(fenetre_t *fenetre, int x, int y) {
  int resultat = -1;

  if(((x >= fenetre->posx + 1) && (x < fenetre->largeur + fenetre->posx - 1)) &&
     ((y >= fenetre->posy + 1) && (y < fenetre->hauteur + fenetre->posy - 1)))
    resultat = 1;

  return resultat;
}

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
int fenetre_getcoordonnees(fenetre_t *fenetre, int posx, int posy, int *x, int *y) {
  int resultat = fenetre_estdans(fenetre, posx, posy);

  if(resultat == 1) {
    *x = posx - fenetre->posx - 1;
    *y = posy - fenetre->posy - 1;
  }
  return resultat;
}	

/**
 * Recupere depuis des coordonnees de la fenetre_carte, les coordonnees au sein d'une 
 * zone ainsi que la zone correspondante.
 * @param posx coordonnees x au sein de la fenetre
 * @param poxy coordonnees y au sein de la fenetre
 * @param numLigneZone la ligne numero de la ligne de zone de la zone dans le client d'affichage
 * @param numColZone la ligne numero de la colonne de zone de la zone dans le client d'affichage
 * @param xZone coordonnee x au sein de la zone
 * @param yZone coordonnee y au sein de la zone
 */
void zone_getcoordonnees(int posx, int posy, int* numLigneZone, int* numColZone, int* xZone, int* yZone) { 
	*xZone = posx % TAILLE_ZONE;
   	*yZone = posy % TAILLE_ZONE;
	*numLigneZone = posy / TAILLE_ZONE;
	*numColZone = posx / TAILLE_ZONE;
}	

/**
 * Affiche une chaine de caracteres dans la fenetre a la position actuelle.
 * @param fenetre la fenetre
 * @param str la chaine
 */
void fenetre_affch(fenetre_t *fenetre, char *chaine, ...) {
  va_list pa;

  va_start(pa, chaine);
  if(fenetre == NULL)
    vwprintw(stdscr, chaine, pa);
  else
    vwprintw(fenetre->interieur, chaine, pa);
  va_end(pa);
}

/**
 * Affiche une chaine de caracteres dans la fenetre a la position actuelle.
 * @param fenetre la fenetre
 * @param posx la position en x
 * @param posy la position en y
 * @param str la chaine
 */
void fenetre_affch_pos(fenetre_t *fenetre, int posx, int posy, char *chaine, ...) {
  va_list pa;

  wmove(fenetre->interieur, posy, posx);
  va_start(pa, chaine);
  if(fenetre == NULL)
    vwprintw(stdscr, chaine, pa);
  else
    vwprintw(fenetre->interieur, chaine, pa);
  va_end(pa);
}

/**
 * Affiche un caractere dans la fenetre a la position actuelle et dans la
 * couleur specifiee.
 * @param fenetre la fenetre
 * @param car le caractere
 * @param num le numero de la couleur
 */
void fenetre_affcar(fenetre_t *fenetre, char car, int num) {
  waddch(fenetre->interieur, car | COLOR_PAIR(num));
}

/**
 * Deplace le curseur a la position specifiee.
 * @param fenetre la fenetre
 * @param 
 */
void fenetre_setpos(fenetre_t *fenetre, int x, int y) {
  wmove(fenetre->interieur, y, x);
}

/**
 * Recupere la position x et y de la souris.
 * @param[out] x la position en x
 * @param[out] y la position en y
 * @return OK si reussite
 */
int souris_getpos(int *x, int *y) {
  MEVENT event;
  int resultat = getmouse(&event);

  if(resultat == OK) {
    *x = event.x;
    *y = event.y;
  }
  return resultat;
}
