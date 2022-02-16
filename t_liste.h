#ifndef __T_LISTE__
#define __T_LISTE__

typedef struct lettre lettre;
struct lettre
{
    char* message;
    int lg; 
    struct lettre* lettre_suivante;
};
 

typedef struct t_bal t_bal;
struct t_bal
{
    lettre* premiere_lettre;
    int num_recept;

    int nb_lettres;
    struct t_bal* bal_suivante;
};

typedef struct 
{
    t_bal* premiere_bal;
    int nb_bal;
}t_liste_bal;

void ajouter_bal(t_liste_bal* liste_bal, int num_recept);

void ajouter_lettre(t_bal* bal, char* message, int lg); 

t_liste_bal* initialiser_liste_bal(void);




#endif 
