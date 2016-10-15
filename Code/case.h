#ifndef _CASE_
#define _CASE_

#include "tableRoutage.h"

#define VIDE    	0
#define HAUT     	1
#define BAS  		2
#define GAUCHE  	3
#define DROITE  	4
#define CROISEMENT  	5
#define FEU_ROUGE  	6
#define FEU_VERT  	7
#define STOP 		8
#define ROUTAGE		9
#define VOITURE 	10

#define CAR_VIDE 	' '
#define CAR_HAUT  	'^'
#define CAR_BAS 	'v'
#define CAR_GAUCHE 	'<'
#define CAR_DROITE 	'>'
#define CAR_CROISEMENT 	'#'
#define CAR_FEU_ROUGE	'f'
#define CAR_FEU_VERT    'F'
#define CAR_STOP 	's'
#define CAR_ROUTAGE     'r'
#define CAR_VOITURE	'V'

#define COL_VIDE 	5
#define COL_HAUT  	1
#define COL_BAS 	1
#define COL_GAUCHE 	1
#define COL_DROITE 	1
#define COL_CROISEMENT 	2
#define COL_FEU_ROUGE	3
#define COL_FEU_VERT	7
#define COL_STOP 	4
#define COL_VOITURE 	6

#define TYPE(valcase) (							\
		       (valcase) == CAR_HAUT ? HAUT :			\
		       (valcase) == CAR_BAS ? BAS :			\
		       (valcase) == CAR_GAUCHE ? GAUCHE :		\
		       (valcase) == CAR_DROITE ? DROITE :		\
		       (valcase) == CAR_CROISEMENT ? CROISEMENT :	\
		       (valcase) == CAR_FEU_ROUGE ? FEU_ROUGE :		\
		       (valcase) == CAR_FEU_VERT ? FEU_VERT :		\
		       (valcase) == CAR_STOP ? STOP :			\
		       (valcase) == CAR_VOITURE ? VOITURE :		\
		       VIDE)

#define CAR(tcase) (						\
		    (tcase)== HAUT ? CAR_HAUT :			\
		    (tcase)== BAS ? CAR_BAS :			\
		    (tcase)== GAUCHE ? CAR_GAUCHE :		\
		    (tcase)== DROITE ? CAR_DROITE :		\
		    (tcase)== CROISEMENT ? CAR_CROISEMENT :	\
		    (tcase)== FEU_ROUGE ? CAR_FEU_ROUGE :	\
		    (tcase)== FEU_VERT ? CAR_FEU_VERT :	        \
		    (tcase)== STOP ? CAR_STOP :			\
		    (tcase)== ROUTAGE ? CAR_ROUTAGE :		\
		    (tcase)== VOITURE ? CAR_VOITURE :		\
		    CAR_VIDE)

#define COL(tcase) (						\
		    (tcase)== HAUT ? COL_HAUT :			\
		    (tcase)== BAS ? COL_BAS :			\
		    (tcase)== GAUCHE ? COL_GAUCHE :		\
		    (tcase)== DROITE ? COL_DROITE :		\
		    (tcase)== CROISEMENT ? COL_CROISEMENT :	\
		    (tcase)== FEU_ROUGE ? COL_FEU_ROUGE :	\
     	            (tcase)== FEU_VERT ? COL_FEU_VERT :	\
		    (tcase)== STOP ? COL_STOP :			\
		    (tcase)== VOITURE ? COL_VOITURE :		\
		    COL_VIDE)

typedef struct {
	char type;  
	tableRoutage_t tRoutage;  /* table de routage pour croisement, allumage feu */	
}Case;

/**
 * Creation d'une case vide
 * @return la case creer
 **/
Case creer_Case();

/**
 * Modification du type d'une case.
 * @param c la case a modifier
 * @param type le nouveau type de la case 
 **/
void set_caseType(Case* c, char type);

#endif
