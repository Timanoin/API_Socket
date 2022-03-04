#ifndef __T_LISTE__
#define __T_LISTE__

typedef struct t_lettre t_lettre;
struct t_lettre
{
    char* message;
    int lg; 
    struct t_lettre* lettre_suivante;
};
 

typedef struct t_bal t_bal;
struct t_bal
{
    t_lettre* premiere_lettre;
    int num_recepteur;

    int nb_lettres;
    struct t_bal* bal_suivante;
};

typedef struct 
{
    t_bal* premiere_bal;
    int nb_bal;
}t_liste_bal;

void ajouter_bal(t_liste_bal* liste_bal, int num_recepteur);

// Verifie que la bal num_recepteur existe bien dans la liste_bal
int verifier_existance_bal(t_liste_bal* liste_bal, int num_recepteur);

t_bal* recuperer_bal(t_liste_bal* liste_bal, int num_recepteur);

t_lettre* nouvelle_lettre(char* message, int lg);

void ajouter_lettre(t_bal* bal, char* message, int lg); 

t_liste_bal* initialiser_liste_bal(void);

#endif 
