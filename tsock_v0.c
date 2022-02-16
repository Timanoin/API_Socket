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

#define NOM_POSTE_DEST localhost
#define LG_MESSAGE_DEF 30
#define NB_MESSAGES_DEF 30
#define TAILLE_MAX 500
#define NB_MAX 5

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

// Affichage en mode puits
void afficher_message_puits (char *message, int lg, int numero) {
	int i;
	printf("PUITS : Reception n°%d (%d) [", numero+1, lg);
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
	message_id[0] = (char)option;
	message_id[sizeof(int)] = (char)nb;
	message_id[2*sizeof(int)] = (char)lg;
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

	// TCP
	// On envoie une demande de connexion à la machine distante.

	int connexion = connect(sock, (struct sockaddr *)&adr_dest, sizeof(adr_dest)); 
	if (connexion == -1)
	{
		printf("Erreur de connexion\n");
		exit(1);
	}

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
	NULL;
}

// Crée un socket qui agit en temps que serveur de bal
// port : le port du poste destinataire
void comm_bal(int port)
{
	int sock, sock_bis;
	struct sockaddr_in adr_local;

	// Construction du socket
	// TCP
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

	struct sockaddr_in padr_em;
	int plg_adr_em = sizeof(padr_em);

	// TCP
	// On utilise listen pour construire une file d'attente des messages recus en simultané.
	// Puis on attend une demande de connexion. Lorsqu'une demande est recue, elle est acceptée.
	// Alors on divise le programme : 
	// - un fils qui va gérer la réception des messages 
	//   et leur affichage, qui meurt une fois sa tache terminée.
	// - le père qui continue à recevoir des demandes de connexion.
	listen(sock, NB_MAX);
	sock_bis = accept(sock, (struct sockaddr*)&padr_em, &plg_adr_em);
	if (sock_bis==-1)
	{
		perror("Erreur : la connexion n'a pas ete acceptee"); 
	}	

	char message_id[12];
	memset(message_id, 0, 3*sizeof(int)); 

	read(sock_bis, message_id, 3*sizeof(int)); 

	int identite = (int)message_id[0]; 
	int nb = (int)message_id[4]; 
	int lg = (int)message_id[8]; 



	close(sock);
}



