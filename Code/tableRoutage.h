#ifndef _TABLEROUTAGE_
#define _TABLEROUTAGE_

/* Constantes pour chaque direction */
#define DIRECTION_HAUT   1
#define DIRECTION_BAS    2
#define DIRECTION_DROITE 4
#define DIRECTION_GAUCHE 8

/* Macro permettant de determiner le decalage suivant la direction */
#define DECALAGE(dir) (					\
		       (dir) == DIRECTION_HAUT ? 4 :	\
		       (dir) == DIRECTION_BAS ? 0 :	\
		       (dir) == DIRECTION_DROITE ? 4 :	\
		       (dir) == DIRECTION_GAUCHE ? 0 :	\
		       -1)
/* Macro permettant de determiner la ligne correspond a une direction */
#define INDICE(dir) (					\
		       (dir) == DIRECTION_HAUT ? 0 :	\
		       (dir) == DIRECTION_BAS ? 0 :	\
		       (dir) == DIRECTION_DROITE ? 1 :	\
		       (dir) == DIRECTION_GAUCHE ? 1 :	\
		       -1)
/* Macro permettant de determiner le caractere correspond a une direction */
#define CARAC(dir) (						\
		       (dir) == DIRECTION_HAUT ? 'H' :		\
		       (dir) == DIRECTION_BAS ? 'B' :		\
		       (dir) == DIRECTION_DROITE ? 'D' :	\
		       (dir) == DIRECTION_GAUCHE ? 'G' :	\
		       '\0')

/* Direction possible ou non */
#define DIRECTION_OK 1
#define DIRECTION_KO 0

/* Structure de la table de routage */
typedef struct {
  char table[2];
} tableRoutage_t;

/**
 * Creation d'une table de routage vide.
 * @return un table de routage vide
 */
tableRoutage_t tableRoutage_creer();

/**
 * Modifie la valeur d'une entree.
 * @param t la table de routage a modifier
 * @param entree la direction d'entree
 * @param sortie la direction de sortie
 * @param valeur la nouvelle valeur
 */
void tableRoutage_modifier(tableRoutage_t *t, int entree, int sortie, int valeur);

/**
 * Indique si une direction est possible.
 * @param t la table de routage
 * @param entree la direction d'entree
 * @param sortie la direction de sortie
 * @return soit DIRECTION_OK, soit DIRECTION_KO
 */
int tableRoutage_direction(tableRoutage_t *t, int entree, int sortie);

#endif
