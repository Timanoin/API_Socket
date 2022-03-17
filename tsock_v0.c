/* librairie standard ... */
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
#define DEBUG 0




// Retourne la longueur d'un nombre (nombre de caractères nécessaires à son écriture)
int longueur_nombre(int nombre);

// Construit un message en répétant lg-5 fois un motif
void construire_message (char *message, char motif, int lg, int numero);

// Affichage en mode puits
void afficher_message_puits (char *message, int lg, int numero);

// Affichage en mode source
void afficher_message_emetteur (char *message, int lg, int num_recept, int numero);

// Construit un message d'identification
void construire_message_id(char* message_id, int option, int nb, int lg, int num_recept);

// Envoie un message d'erreur et ferme le programme
void message_erreur(void);

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
					message_erreur();
					exit(1);
				}
				option = EMISSION;
				num_recept = atoi(optarg);
				break;

			// mode réception
			case 'r':
				if (option == EMISSION || option == BAL) 
				{
					message_erreur();
					exit(1);
				}
				option = RECEPTION;
				num_recept = atoi(optarg);
				break;

			// mode serveur bal
			case 'b':
				if (option == RECEPTION || option == EMISSION) 
				{
					message_erreur();
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
				message_erreur();
				break;
		}
	}

	if (option == EMISSION)
	 {
		printf("* Mode : client EMISSION\n\n");
		comm_emetteur(num_recept, argv[argc-2], htons(atoi(argv[argc-1])), lg_message, nb_messages);	
	 }
	else if (option == RECEPTION)
	{
		printf("* Mode : client RECEPTION\n\n");
		comm_recepteur(num_recept, argv[argc-2], htons(atoi(argv[argc-1])));	
	}
	else if (option == BAL)
	{
		printf("* Mode : serveur BOITE AUX LETTRES\n\n");
		comm_bal(htons(atoi(argv[argc-1])));
	}
	else
	{
		message_erreur();
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

// Envoie un message d'erreur et ferme le programme
void message_erreur(void)
{
	printf("usage: cmd [-e|-r|-b][-n ##] [-l ##]\n");
}

// Affichage en mode recepteur
void afficher_message_recepteur (char *message, int lg, int numero) 
{
	printf("* RECEPTION : Récupération de la lettre par le recepteur %d (%d) \n[%s]\n", numero, lg, message);
}

// Affichage en mode emetteur
void afficher_message_emetteur (char *message, int lg, int num_recept, int numero) 
{
	printf("* SOURCE : Envoi de la lettre n°%d pour le recepteur n°%d \n[%s]\n", numero+1, num_recept,message);
}

// Affichage en mode BAL
void afficher_message_bal_stockage (char *message, int num_recept, int numero)
{
	printf("* PUITS : Stockage de la lettre n°%d pour le recepteur n°%d \n[%s]\n", numero, num_recept,message);
}


// Construit un message d'identification
void construire_message_id(char* message_id, int option, int nb, int lg, int num_recept)
{
	memset(message_id, 0, 4*sizeof(int));
	sprintf(message_id, "%4d%4d%4d%4d",option,nb,lg,num_recept);
	printf("* Envoi du message d'identification...\n");
	#if DEBUG	
	printf("d-Contenu du message d'identification envoye : %s\n", message_id);
	#endif	
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
		printf("* Erreur lors de la creation du socket.\n");
		exit(1);
	}

	// Création de l'adresse distante du serveur BAL
	memset((char*)&adr_dest, 0, sizeof(adr_dest));
	adr_dest.sin_family = AF_INET;
	adr_dest.sin_port = port;
	
	struct hostent* hp;
	if ((hp = gethostbyname(nom_machine)) == NULL)
	{
		printf("* Erreur lors de la recuperation de l'adresse IP.\n");
		exit(1);
	}
	memcpy((char*)&(adr_dest.sin_addr.s_addr), hp->h_addr, hp->h_length);

	// On envoie une demande de connexion TCP à la machine distante.
	int connexion = connect(sock, (struct sockaddr *)&adr_dest, sizeof(adr_dest)); 
	if (connexion == -1)
	{
		printf("* Erreur lors de la connexion.\n");
		exit(1);
	}

	// Construction et envoi du message d'identification
	char message_id[12];
	construire_message_id(message_id,EMISSION, nb, lg, num_recept);
	write(sock, message_id, 4*sizeof(int));

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
	printf("\n* Fin de l'emission des messages.\n* Fermeture de la connexion par le serveur.\n");
	// La fermeture de la connexion est gérée par la bal
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
		printf("* Erreur lors de la creation du socket.\n");
		exit(1);
	}

	// Création de l'adresse distante du serveur BAL
	memset((char*)&adr_dest, 0, sizeof(adr_dest));
	adr_dest.sin_family = AF_INET;
	adr_dest.sin_port = port;
	
	struct hostent* hp;
	if ((hp = gethostbyname(nom_machine)) == NULL)
	{
		printf("* Erreur lors de la recuperation de l'adresse IP.\n");
		exit(1);
	}
	memcpy((char*)&(adr_dest.sin_addr.s_addr), hp->h_addr, hp->h_length);

	// On envoie une demande de connexion TCP à la machine distante.
	int connexion = connect(sock, (struct sockaddr *)&adr_dest, sizeof(adr_dest)); 
	if (connexion == -1)
	{
		printf("* Erreur lors de la connexion.\n");
		exit(1);
	}

	// Construction et envoi du message d'identification
	char message_id[16];
	construire_message_id(message_id,RECEPTION, 0, 0, num_recept);
	write(sock, message_id, 4*sizeof(int));

	// Reception du message indiquant si la bal existe
	// et le nombre de lettres
	char message_info[8];
	memset(message_info, 0, 9); 
	read(sock, message_info, 8);
	#if DEBUG	
	printf("d-Reception message etat de la boite aux lettres : %s\n", message_info);
	#endif

	int existance_bal;
	int nb_lettres;
	sscanf(message_info, "%4d%4d", &existance_bal, &nb_lettres); 

	if (!existance_bal)
	{
		// La bal n'existe pas
		printf("* La boite aux lettres n'existe pas.\n");
	}
	else if (!nb_lettres)
	{
		// La bal existe mais pointe vers NULL
		printf("* Vous n'avez aucun message.\n");
	}
	else
	{
		// La bal existe et contient au moins une lettre
		// On recoit le nombre de lettres indiquées dans le message_info
		int i; 
		for (i=0; i<nb_lettres; i++)
		{
			char message_taille_lettre[4];
			memset(message_taille_lettre, 0, 5); 
			read(sock, message_taille_lettre, 4); 

			int taille;
			sscanf(message_taille_lettre, "%4d", &taille);

			char message[TAILLE_MAX];
			memset(message, 0, taille);
			read(sock, message, taille);
			afficher_message_recepteur(message, taille, num_recept); 
		}
	}
	printf("\n* Fin de la reception des messages.\n* Fermeture de la connexion par le serveur.\n");
	// La bal ferme la connexion
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
		printf("* Erreur lors de la création du socket\n");
		exit(1);
	}
	// Création de l'adresse locale
	memset((char*)&adr_local, 0, sizeof(adr_local));
	adr_local.sin_family = AF_INET;
	adr_local.sin_port = port;
	adr_local.sin_addr.s_addr = INADDR_ANY;

	// On lie l'adresse créée à la bal
	if(bind(sock, (struct sockaddr*)&adr_local, sizeof(adr_local)) == -1)
	{
		printf("* Echec lors de la liaison de l'adresse du socket a la boite aux lettres.\n");
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
	
	// Initialisation de la liste de bal vide
	t_liste_bal* liste = initialiser_liste_bal();
	printf("* Initialisation de la liste de boite aux lettres.\n\n"); 

	// Boucle infinie : le serveur BAL ne peut se fermer 
	// qu'avec un arrêt forcé du processus
	while(1)
	{
		printf("* En attente d'une connexion entrante...\n");

		#if DEBUG	
		afficher_structure_bal(liste);
		#endif
			
		// Acceptation de la demande de connexion
		sock_bis = accept(sock, (struct sockaddr*)&padr_em, &plg_adr_em);
		if (sock_bis == -1)
		{
			perror("* Erreur : la connexion n'a pas ete acceptee."); 
		}	
		printf("* Une connexion entrante a ete acceptee.\n");

		// Reception du message d'identification de la part du client
		char message_id[16];
		memset(message_id, 0, 17); 
		read(sock_bis, message_id, 16); 

		#if DEBUG
		printf("Message identification lu avec succes : %s\n", message_id);
		#endif

		// Récupération des informations du message d'identification
		int identite;
		int nb_messages_attendus;
		int lg_attendue;
		int num_recept;	
		sscanf(message_id, "%4d%4d%4d%4d",&identite, &nb_messages_attendus, &lg_attendue, &num_recept); 
		
		if (identite == EMISSION)
		{
			printf("* Le client est un EMETTEUR.\n");

			int i;
			int lg_effective;
			
			// Verification de l'existence de la bal
			if (!verifier_existance_bal(liste, num_recept))
			{
				ajouter_bal(liste, num_recept);
				printf("* La bal du recepteur souhaite n'existe pas.\n* Creation d'une nouvelle bal.\n");
			}	
			
			// Reception des lettres et stockage
			char message[TAILLE_MAX];
			memset(message, 0, lg_attendue);
			t_bal* bal = recuperer_bal(liste, num_recept);
			for (i=0;i<nb_messages_attendus;i++)
			{	
				// Lecture du message
				lg_effective = read(sock_bis, message, lg_attendue);
				if (lg_effective==-1)
				{
					perror("* Erreur lors de la lecture.\n"); 
				}
				afficher_message_bal_stockage(message, num_recept, i+1);

				// Reservation de mémoire pour la lettre
				char *contenu_lettre = (char*) malloc(lg_effective+1);
				strcpy(contenu_lettre, message);

				// Stockage de la lettre dans la bal
				ajouter_lettre(bal, contenu_lettre, lg_effective);
			}
		}
		else // Recepteur
		{
			printf("* Le client est un RECEPTEUR.\n");
			if (verifier_existance_bal(liste, num_recept))
			{
				t_bal* bal = recuperer_bal(liste, num_recept);
				// Envoi d'un message qui indique que la bal existe 
				// et le nombre de lettres
				char message_recept[9];
				memset(message_recept, 0, 9);
				sprintf(message_recept, "%4d%4d", 1, bal->nb_lettres);
				write(sock_bis, message_recept, 2*sizeof(int));

				#if DEBUG
				printf("d-Message contenant l'etat de la bal : %s\n", message_recept);
				#endif

				t_lettre* p = bal->premiere_lettre; 
				// Boucle d'envoi des lettres stockées dans la bal
				while (p != NULL)
				{
					// Construction et envoi d'un message 
					// qui indique la taille de la prochaine lettre
					char message_taille[4];
					memset(message_taille, 0, 5);
					sprintf(message_taille, "%d", p->lg);
					write(sock_bis, message_taille, sizeof(int));

					// Envoi de la lettre
					write(sock_bis, p->message, p->lg);

					// Passage a la lettre suivante
					p = p->lettre_suivante; 
				}
				printf("* Toutes les lettres ont ete envoyees au recepteur avec succes.\n");
			}
			else
			{
				printf("* Erreur : aucune bal n'appartient a ce recepteur.\n");
				// Envoi d'un message qui indique que la bal n'existe pas
				char message_recept[9];
				memset(message_recept, 0, 9);
				sprintf(message_recept, "%4d%4d", 0, 0);
				write(sock_bis, message_recept, 2*sizeof(int));
			}
		}
		// Ferme la connexion entre la bal et le client. N'arrete pas la bal
		printf("* Fermeture de la connexion avec le client.\n");
		close(sock_bis);
	}
	close(sock);
}

//fin :)