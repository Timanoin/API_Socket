/* librairie standard ...kkkkk */
#include <stdlib.h>
/* pour getopt */
#include <unistd.h>
/* déclaration des types de base */
#include <sys/types.h>
/* constantes relatives aux domaines, types et protocoles */
#include <sys/socket.h>
/* constantes et structures propres au domaine UNIX */
#include <sys/un.h>
/* constantes et structures propres au domaine INTERNET */
#include <netinet/in.h>
/* structures retournées par les fonctions de gestion de la base de
données du réseau */
#include <netdb.h>
/* pour les entrées/sorties */
#include <stdio.h>
/* pour la gestion des erreurs */
#include <errno.h>

#include "t_liste.h"

#define NOM_POSTE_DEST localhost
#define LG_MESSAGE_DEF 30
#define NB_MESSAGES_DEF 30
#define TAILLE_MAX 500
#define NB_MAX 5

#define TAILLE_MESSAGE_RECEPT 8

#define EMISSION 0
#define RECEPTION 1
#define BAL 2


// Retourne la longueur d'un nombre (nombre de caractères nécessaires à son écriture)
int longueur_nombre(int nombre);

// Construit un message en répétant lg-5 fois un motif
void construire_message (char *message, char motif, int lg, int numero);

// Affichage en mode puits
void afficher_message_puits (char *message, int lg, int numero);

// Affichage en mode source
void afficher_message_emetteur (char *message, int lg, int num_recept, int numero);

// Construit un message d'identification
void construire_message_id(char* message_id, int option, int nb, int lg);

// Crée un socket qui agit en temps qu'emetteur
// num_recept : numéro du récepteur
// nom_machine : le nom du poste hébergeant le serveur
// port : le port du poste destinataire
// lg : longueur du message
// nb : nombre de messages
void comm_emetteur(int num_recept, char* nom_machine, int port, int lg, int nb);

// Crée un socket qui agit en temps que récepteur
// num_recept : numéro du récepteur
// nom_machine : le nom du poste hébergeant le serveur
// port : le port du poste destinataire
void comm_recepteur(int num_recept, char* nom_machine, int port);

// Crée un socket qui agit en temps que serveur de bal
// port : le port du poste destinataire
void comm_bal(int port);

void main (int argc, char **argv)
{
	int c;
	extern char *optarg;
	extern int optind;
	int option = -1 ; /* 0=emetteur, 1=recepteur, 2=bal */
	int num_recept;
	int nb_messages = NB_MESSAGES_DEF;
	int lg_message = LG_MESSAGE_DEF;

	while ((c = getopt(argc, argv, "e:r:bl:n:")) != -1) 
	{
		switch (c) 
		{
			// mode émission
			case 'e':
				if (option == RECEPTION || option == BAL) 
				{
					printf("usage: cmd [-p|-s][-n ##]\n");
					exit(1);
				}
				option = EMISSION;
				num_recept = atoi(optarg);
				break;

			// mode réception
			case 'r':
				if (option == EMISSION || option == BAL) 
				{
					printf("usage: cmd [-p|-s][-n ##]\n");
					exit(1);
				}
				option = RECEPTION;
				num_recept = atoi(optarg);
				break;

			// mode serveur bal
			case 'b':
				if (option == RECEPTION || option == EMISSION) 
				{
					printf("usage: cmd [-p|-s][-n ##]\n");
					exit(1);
				}
				option = BAL;
				break;

			

			case 'n':
				nb_messages = atoi(optarg);
				break;

			case 'l' : 
				lg_message = atoi(optarg);
				break;

			default:
				printf("usage: cmd [-p|-s][-n ##]\n");
				break;
		}
	}

	if (option == EMISSION)
	 {
		printf("On est en mode emission\n");
		comm_emetteur(num_recept, argv[argc-2], htons(atoi(argv[argc-1])), lg_message, nb_messages);	
	 }
	else if (option == RECEPTION)
	{
		printf("On est en mode reception\n");
		comm_recepteur(num_recept, argv[argc-2], htons(atoi(argv[argc-1])));	
	}
	else if (option == BAL)
	{
		printf("On est en mode serveur boite aux lettres\n");
		comm_bal(htons(atoi(argv[argc-1])));
	}
	else
	{
		printf("usage: cmd [-p|-s][-n ##]\n");
		exit(1) ;
	}

	exit(0);
}

// FONCTIONS

// Retourne la longueur d'un nombre (nombre de caractères nécessaires à son écriture)
int longueur_nombre(int nombre)
{
	int lg = 0;
	while (nombre != 0)
	{
		nombre /= 10;
		lg++;
	}
	return lg;
}

// Construit un message en répétant lg-5 fois un motif
void construire_message (char *message, char motif, int lg, int numero) 
{
	// Construction de la suite de motifs
	int i;
	char str[5];
	sprintf(str, "%d", numero);
	for (i=0; i < lg; i++) 
	{
		if (i < 5-longueur_nombre(numero))
		{
			message[i] = '-';
		}
		else if (i < 5)
		{		
			message[i] = str[i-5+longueur_nombre(numero)];
		}
		else
		{
			message[i] = motif;
		}
	}
}

// Affichage en mode recepteur
void afficher_message_recepteur (char *message, int lg, int numero) 
{
	int i;
	printf("RECEPTION : Récupération de la lettre par le recepteur %d (%d) [", numero, lg);
	for (i=0 ; i<lg ; i++) 
	{
		printf("%c", message[i]) ; 
	}
	printf("]\n");
}

// Affichage en mode emetteur
void afficher_message_emetteur (char *message, int lg, int num_recept, int numero) {
	int i;
	printf("SOURCE : Envoi de la lettre n°%d pour le recepteur n°%d[", numero+1, num_recept);
	for (i=0 ; i<lg ; i++) 
	{
		printf("%c", message[i]) ; 
	}
	printf("]\n");
}


// Construit un message d'identification
void construire_message_id(char* message_id, int option, int nb, int lg)
{
	memset(message_id, 0, 3*sizeof(int));
	message_id[0] = itoa(option);
	message_id[sizeof(int)] = itoa(nb);
	message_id[2*sizeof(int)] = itoa(lg);
}

// Construit un premier message adressé au recepteur lui indiquant 
// si la bal existe et le nombre de messages
void construire_message_recepteur(char* message_recept, int existance_bal, int nb_lettres)
{
	memset(message_recept, 0, 2*sizeof(int));
	message_recept[0] = itoa(existance_bal);
	message_recept[sizeof(int)] = itoa(nb_lettres);
}


// Crée un socket qui agit en temps qu'emetteur
// num_recept : numéro du récepteur
// nom_machine : le nom du poste hébergeant le serveur
// port : le port du poste destinataire
// lg : longueur du message
// nb : nombre de messages
void comm_emetteur(int num_recept, char* nom_machine, int port, int lg, int nb)
{
	int sock;
	struct sockaddr_in adr_dest;

	// Construction du socket TCP
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		printf("Erreur lors de la création du socket\n");
		exit(1);
	}

	// Création de l'adresse distante du serveur BAL
	memset((char*)&adr_dest, 0, sizeof(adr_dest));
	adr_dest.sin_family = AF_INET;
	adr_dest.sin_port = port;
	
	struct hostent* hp;
	if ((hp = gethostbyname(nom_machine)) == NULL)
	{
		printf("Erreur gethostbyname \n");
		exit(1);
	}
	memcpy((char*)&(adr_dest.sin_addr.s_addr), hp->h_addr, hp->h_length);

	// On envoie une demande de connexion TCP à la machine distante.
	int connexion = connect(sock, (struct sockaddr *)&adr_dest, sizeof(adr_dest)); 
	if (connexion == -1)
	{
		printf("Erreur de connexion\n");
		exit(1);
	}

	// Construction et envoi du message d'identification
	char message_id[12];
	construire_message_id(message_id,EMISSION, nb, lg);
	write(sock, message_id, 3*sizeof(int));

	// Envoi des messages
	char message[TAILLE_MAX];
	memset(message, 0, lg);
	char motif;

	int i;
	for (i=0; i<nb; i++)
	{
		motif = (char)((int)'a' + i % 26);
		construire_message(message, motif,lg, num_recept); 
		write(sock, message, lg);
		afficher_message_emetteur(message, lg, num_recept, i);
	}
}

// Crée un socket qui agit en temps que récepteur
// num_recept : numéro du récepteur
// nom_machine : le nom du poste hébergeant le serveur
// port : le port du poste destinataire
void comm_recepteur(int num_recept, char* nom_machine, int port)
{
	int sock;
	struct sockaddr_in adr_dest;

	// Construction du socket TCP
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		printf("Erreur lors de la création du socket\n");
		exit(1);
	}

	// Création de l'adresse distante du serveur BAL
	memset((char*)&adr_dest, 0, sizeof(adr_dest));
	adr_dest.sin_family = AF_INET;
	adr_dest.sin_port = port;
	
	struct hostent* hp;
	if ((hp = gethostbyname(nom_machine)) == NULL)
	{
		printf("Erreur gethostbyname \n");
		exit(1);
	}
	memcpy((char*)&(adr_dest.sin_addr.s_addr), hp->h_addr, hp->h_length);

	// On envoie une demande de connexion TCP à la machine distante.
	int connexion = connect(sock, (struct sockaddr *)&adr_dest, sizeof(adr_dest)); 
	if (connexion == -1)
	{
		printf("Erreur de connexion\n");
		exit(1);
	}

	// Construction et envoi du message d'identification
	char message_id[12];
	construire_message_id(message_id,RECEPTION, 0, 0);
	write(sock, message_id, 3*sizeof(int));

	// Reception du message indiquant si la bal existe
	//et le nombre de lettres
	char message_info[8];
	memset(message_info, 0, 2*sizeof(int)); 
	read(sock, message_info, 2*sizeof(int)); 

	if (atoi(message_info[0]) == 0)
	{
		printf("La boite aux lettres n'existe pas.\n");
	}
	else if (atoi(message_info[4]) == 0)
	{
		printf("Vous n'avez aucun message. :(\n");
	}
	else
	{
		int i; 
		for (i=0; i<atoi(message_info[4]); i++)
		{
			char message_recu[4];
			memset(message_recu, 0, sizeof(int)); 
			read(sock, message_recu, sizeof(int)); 

			char message[TAILLE_MAX];
			memset(message, 0, atoi(message_recu));
			read(sock, message, atoi(message_recu));
			afficher_message_recepteur(message, atoi(message_recu), num_recept); 
		}
	}
	
}

// Crée un socket qui agit en temps que serveur de bal
// port : le port du poste destinataire
void comm_bal(int port)
{
	int sock, sock_bis;
	struct sockaddr_in adr_local;

	// Construction du socket TCP
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		printf("Erreur lors de la création du socket\n");
		exit(1);
	}

	// Création de l'adresse locale
	memset((char*)&adr_local, 0, sizeof(adr_local));
	adr_local.sin_family = AF_INET;
	adr_local.sin_port = port;
	adr_local.sin_addr.s_addr = INADDR_ANY;

	if(bind(sock, (struct sockaddr*)&adr_local, sizeof(adr_local)) == -1)
	{
		printf("Echec du bind\n");
		exit(1);
	}

	// TCP
	// On utilise listen pour construire une file d'attente des messages recus en simultané.
	// Puis on attend une demande de connexion. Lorsqu'une demande est recue, elle est acceptée.
	// Alors on divise le programme : 
	// - un fils qui va gérer la réception des messages 
	//   et leur affichage, qui meurt une fois sa tache terminée.
	// - le père qui continue à recevoir des demandes de connexion.
	struct sockaddr_in padr_em;
	int plg_adr_em = sizeof(padr_em);

	listen(sock, NB_MAX);
	sock_bis = accept(sock, (struct sockaddr*)&padr_em, &plg_adr_em);
	if (sock_bis == -1)
	{
		perror("Erreur : la connexion n'a pas ete acceptee"); 
	}	

	t_liste_bal* liste = initialiser_liste_bal(); 

	// Reception du message d'identification
	char message_id[12];
	memset(message_id, 0, 3*sizeof(int)); 
	read(sock_bis, message_id, 3*sizeof(int)); 
	
	// Récupération des informations du message d'identification
	int identite = (int)message_id[0]; 
	int nb_messages_attendus = (int)message_id[4]; 
	int lg_attendue = (int)message_id[8]; 

	char message[TAILLE_MAX];
	memset(message, 0, lg_attendue);

	if (identite == EMISSION)
	{
		int i;
		int lg_effective;
		//Verification de l'existence de la bal
		if (!verifier_existance_bal(liste, identite))
		{
			ajouter_bal(liste, identite);
		}

		t_bal* bal = recuperer_bal(liste, identite);

		for (i=0;i<nb_messages_attendus;i++)
		{
			lg_effective = read(sock_bis, message, lg_attendue);
			if (lg_effective==-1)
			{
				perror("Erreur read\n"); 
			}
			// Extraction des informations du message
			if (bal->nb_lettres==0)
			{
				bal->premiere_lettre = nouvelle_lettre(message, lg_effective);
				bal->nb_lettres = 1;
			}
			else
			{
				ajouter_lettre(bal, message, lg_effective);
			}
		}
	}
	else // Recepteur
	{
		if (verifier_existance_bal(liste, identite))
		{
			t_bal* bal = recuperer_bal(liste, identite);

			// Envoi d'un message qui indique que la bal existe 
			// et le nombre de lettres
			char message_recept[TAILLE_MESSAGE_RECEPT];
			construire_message_recept(message_recept, 1, bal->nb_lettres);
			write(sock_bis, message_recept, 2*sizeof(int));

			int i;
			for (i=0;i<bal->nb_lettres;i++)
			{
				t_lettre* p = bal->premiere_lettre; 
				// Boucle d'envoi des lettres stockées dans la bal
				while (p != NULL)
				{
					// Construction et envoi d'un message 
					// qui indique la taille de la prochaine lettre
					char* message_taille;
					memset(message_taille, 0, sizeof(int));
					message_taille = itoa(p->lg);
					write(sock_bis, message_taille, sizeof(int));

					//Envoi de la lettre
					write(sock_bis, p->message, p->lg);
					p = p->lettre_suivante; 
				}
			}
		}
		close(sock_bis);
	}

	close(sock);
}


