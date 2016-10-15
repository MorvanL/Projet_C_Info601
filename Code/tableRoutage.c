#include "tableRoutage.h"

#include <stdio.h>    /* Pour printf */
#include <stdlib.h>   /* Pour malloc */

/**
 * Creation d'une table de routage vide.
 * @return un table de routage vide
 */
tableRoutage_t tableRoutage_creer() {
  tableRoutage_t resultat;

    resultat.table[0] = '\0';
    resultat.table[1] = '\0';

  return resultat;
}

/**
 * Modifie la valeur d'une entree.
 * @param t la table de routage a modifier
 * @param entree la direction d'entree
 * @param sortie la direction de sortie
 * @param valeur la nouvelle valeur
 */
void tableRoutage_modifier(tableRoutage_t *t, int entree, int sortie, int valeur) {
  if(valeur == DIRECTION_OK)
    t->table[INDICE(entree)] = t->table[INDICE(entree)] | (sortie<<DECALAGE(entree));
  else
    t->table[INDICE(entree)] = t->table[INDICE(entree)] - (sortie<<DECALAGE(entree));
}

/**
 * Indique si une direction est possible.
 * @param t la table de routage
 * @param entree la direction d'entree
 * @param sortie la direction de sortie
 * @return soit DIRECTION_OK, soit DIRECTION_KO
 */
int tableRoutage_direction(tableRoutage_t *t, int entree, int sortie) {
  int resultat;

  if((t->table[INDICE(entree)] & (sortie<<DECALAGE(entree))) == 0)
    resultat = DIRECTION_KO;
  else
    resultat = DIRECTION_OK;

  return resultat;
}
