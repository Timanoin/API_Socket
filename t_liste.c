#include "t_liste.h"
#include <malloc.h> 

void ajouter_bal(t_liste_bal* liste_bal, int num_recepteur)
{
    t_bal* bal = (t_bal*)malloc(sizeof(t_bal)); 
    bal->bal_suivante = NULL;
    bal->nb_lettres = 0; 
    bal->num_recepteur = num_recepteur;
    bal->premiere_lettre = NULL; 

    if (liste_bal->premiere_bal == NULL)
    {
        liste_bal->premiere_bal = bal;
    }
    else
    {
        t_bal* p = liste_bal->premiere_bal; 
        while (p->bal_suivante != NULL)
        {
            p = p->bal_suivante; 
        }
        p->bal_suivante = bal;
    }
    
    liste_bal->nb_bal++; 
}

// Verifie que la bal num_recepteur existe bien dans la liste_bal
int verifier_existance_bal(t_liste_bal* liste_bal, int num_recepteur)
{
    int existe = 0;
    t_bal* p = liste_bal->premiere_bal; 
    while (p!= NULL && !existe)
    {
        existe = (p->num_recepteur == num_recepteur);
        p = p->bal_suivante; 
    }
    return existe;
}

t_bal* recuperer_bal(t_liste_bal* liste_bal, int num_recepteur)
{
    t_bal* p = liste_bal->premiere_bal; 
    while (p->num_recepteur != num_recepteur && p!=NULL)
    {
        p = p->bal_suivante; 
    }
    return p;
}

t_lettre* nouvelle_lettre(char* message, int lg)
{
    t_lettre* nv_lettre = (t_lettre*)malloc(sizeof(t_lettre)); 
    nv_lettre->message = message;
    nv_lettre->lg = lg;
    nv_lettre->lettre_suivante = NULL;
    return nv_lettre;
}

void ajouter_lettre(t_bal* bal, char* message, int lg)
{
    t_lettre* nv_lettre = nouvelle_lettre(message,lg);

    if (!bal->nb_lettres)
    {
        // S'il n'y a aucune lettre, elle devient la premiere
        bal->premiere_lettre = nv_lettre;    
    } 
    else
    {
        t_lettre* p = bal->premiere_lettre;
        while(p->lettre_suivante != NULL)
        {    
            p = p->lettre_suivante;
        } 
        // A ce moment, p pointe vers la dernière lettre de la liste
        p->lettre_suivante = nv_lettre;
    } 
    
    bal->nb_lettres++;  
}

t_liste_bal* initialiser_liste_bal(void)
{
    t_liste_bal* liste_bal = (t_liste_bal*)malloc(sizeof(t_liste_bal));
    liste_bal->premiere_bal = NULL;
    liste_bal->nb_bal = 0; 
    return liste_bal;
}


// Affiche le contenu de chaque lettre, de chaque bal de la liste.
void afficher_structure_bal(t_liste_bal* liste)
{
    int i,j;
    
    t_bal* bal = liste->premiere_bal;
    for (i=0; i<liste->nb_bal;i++)
    {
        printf("BAL %d:\n", bal->num_recepteur);
        t_lettre* lettre = bal->premiere_lettre;
        for (j=0; j<bal->nb_lettres;j++)
        {
            printf("%s\n", lettre->message);
            lettre=lettre->lettre_suivante;
        } 
        bal=bal->bal_suivante;
        printf("\n");
    } 
}


