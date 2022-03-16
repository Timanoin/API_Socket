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
void construire_message_id(char* message_id, int option, int nb, int lg, int num_recept)
{
	memset(message_id, 0, 4*sizeof(int));
	sprintf(message_id, "%4d%4d%4d%4d",option,nb,lg,num_recept);
	printf("Contenu du message d'identification : %s\n", message_id);
}

// Construit un premier message adressé au recepteur lui indiquant 
// si la bal existe et le nombre de messages
void construire_message_recepteur(char* message_recept, int existance_bal, int nb_lettres)
{
	memset(message_recept, 0, 2*sizeof(int));
	sprintf(&message_recept[0],"%d",existance_bal);
	sprintf(&message_recept[sizeof(int)], "%d",nb_lettres);
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
	printf("Fin de l'emission.\n");
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
	char message_id[16];
	construire_message_id(message_id,RECEPTION, 0, 0, num_recept);
	write(sock, message_id, 4*sizeof(int));

	// Reception du message indiquant si la bal existe
	//et le nombre de lettres
	char message_info[8];
	memset(message_info, 0, 2*sizeof(int)); 
	read(sock, message_info, 2*sizeof(int)); 

	if (atoi(&message_info[0]) == 0)
	{
		// La bal n'existe pas
		printf("La boite aux lettres n'existe pas.\n");
	}
	else if (atoi(&message_info[4]) == 0)
	{
		// La bal existe mais pointe vers NULL
		printf("Vous n'avez aucun message. :(\n");
	}
	else
	{
		// La bal existe et contient au moins une lettre
		// On recoit le nombre de lettres indiquées dans le message_info
		int i; 
		for (i=0; i<atoi(&message_info[4]); i++)
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
	printf("Fin de la reception.\n");
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
		printf("Erreur lors de la création du socket\n");
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
	
	// Initialisation de la liste de bal vide
	t_liste_bal* liste = initialiser_liste_bal();
	printf("Liste de bal initialisee\n"); 

	// Boucle infinie : le serveur BAL ne peut se fermer 
	// qu'avec un arrêt forcé du processus
	while(1)
	{
		printf("En attente de connexion...\n");
		// Acceptation de la demande de connexion
		sock_bis = accept(sock, (struct sockaddr*)&padr_em, &plg_adr_em);
		if (sock_bis == -1)
		{
			perror("Erreur : la connexion n'a pas ete acceptee"); 
		}	
		printf("Connexion acceptée.\n");

		// Reception du message d'identification de la part du client
		char message_id[16];
		memset(message_id, 0, 17); 
		read(sock_bis, message_id, 16); 
		printf("Message identification lu avec succes : %s\n", message_id);

		// Récupération des informations du message d'identification
		int identite;
		int nb_messages_attendus;
		int lg_attendue;
		int num_recept;	
		sscanf(message_id, "%4d%4d%4d%4d",&identite, &nb_messages_attendus, &lg_attendue, &num_recept); 


		char message[TAILLE_MAX];
		memset(message, 0, lg_attendue);
		
		if (identite == EMISSION)
		{
			printf("Le client est un EMETTEUR.\n");

			int i;
			int lg_effective;
			
			// Verification de l'existence de la bal
			if (!verifier_existance_bal(liste, num_recept))
			{
				ajouter_bal(liste, num_recept);
				printf("La bal n'existe pas. Creation d'une nouvelle bal.\n");
			}

			t_bal* bal = recuperer_bal(liste, num_recept);

			for (i=0;i<nb_messages_attendus;i++)
			{
				lg_effective = read(sock_bis, message, lg_attendue);
				if (lg_effective==-1)
				{
					perror("Erreur read\n"); 
				}
				// Extraction des informations du message et stockage de la lettre
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
			if (verifier_existance_bal(liste, num_recept))
			{
				t_bal* bal = recuperer_bal(liste, num_recept);
				// Envoi d'un message qui indique que la bal existe 
				// et le nombre de lettres
				char message_recept[TAILLE_MESSAGE_RECEPT];
				construire_message_recepteur(message_recept, 1, bal->nb_lettres);
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
						char message_taille[4];
						memset(message_taille, 0, sizeof(int));
						sprintf(&message_taille[0], "%d", p->lg);
						write(sock_bis, message_taille, sizeof(int));

						//Envoi de la lettre
						write(sock_bis, p->message, p->lg);
						p = p->lettre_suivante; 
					}
				}
			}
			else
			{
				printf("Erreur : aucune bal a ce nom\n");
			}
			// Ferme la connexion entre la bal et le client. N'arrete pas la bal
			close(sock_bis);
		}
	}
	close(sock);
}

//fin :)