#include <stdlib.h> 

#include "case.h"

/**
 * Creation d'une case vide
 * @return la case creer
 **/
Case creer_Case() {
	Case c;
	
	c.tRoutage = tableRoutage_creer();
	c.type = CAR(VIDE);
	
	return c;
}

/**
 * Modification du type d'une case
 * @param c la case a modifier
 * @param type le nouveau type de la case 
 **/
void set_caseType(Case* c, char type) {
	c->type = type;
}



