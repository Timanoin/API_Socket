#include "t_liste.h"
#include <malloc.h> 

void ajouter_bal(t_liste_bal* liste_bal, int num_recept)
{
    t_bal* bal = (t_bal*)malloc(sizeof(t_bal)); 
    bal->bal_suivante = NULL;
    bal->nb_lettres = 0; 
    bal->num_recept = num_recept;
    bal->premiere_lettre = NULL; 

    t_bal* p = liste_bal->premiere_bal; 
    while (p->bal_suivante != NULL)
    {
        p = p->bal_suivante; 
    }
    p->bal_suivante = bal; 
}   

void ajouter_lettre(t_bal* bal, char* message, int lg )
{
    lettre* nv_lettre = (lettre*)malloc(sizeof(lettre)); 
    nv_lettre->message = message;
    nv_lettre->lg = lg;
    nv_lettre->lettre_suivante = NULL; 

    lettre* p = bal->premiere_lettre; 
    while (p->lettre_suivante != NULL)
    {
        p = p->lettre_suivante; 
    }
    p->lettre_suivante = nv_lettre;   
}

t_liste_bal* initialiser_liste_bal(void)
{
    t_liste_bal* liste_bal = (t_liste_bal*)malloc(sizeof(t_liste_bal));
    liste_bal->premiere_bal = NULL;
    liste_bal->nb_bal = 0; 
}