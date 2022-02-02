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

#define NOM_POSTE_DEST localhost
#define LG_MESSAGE_DEF 30
#define NB_MESSAGES_DEF 30
#define TAILLE_MAX 500

// Construit un message en répétant lg-5 fois un motif
void construire_message (char *message, char motif, int lg, int numero);

void afficher_message (char *message, int lg);

// Crée un socket qui agit en temps que source
// nom_dest : le nom du poste destinataire
// port : le port du poste destinataire
// udp : 0=tcp, 1=udp
// lg : longueur du message
// nb : nombre de messages
void comm_source(char* nom_dest, int port, int udp, int lg, int nb);

// Crée un socket qui agit en temps que puits
// port : le port de reception
// udp : 0=tcp, 1=udp
// lg : longueur du message
// nb : nombre de messages
void comm_puits(int port, int udp, int lg, int nb);


void main (int argc, char **argv)
{
	int c;
	extern char *optarg;
	extern int optind;
	int nb_message = -1; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int source = -1 ; /* 0=puits, 1=source */
	int udp = 0; // 0=tcp, 1=udp
	int nb_messages = NB_MESSAGES_DEF;
	int lg_message = LG_MESSAGE_DEF;

	while ((c = getopt(argc, argv, "pun:s")) != -1) 
	{
		switch (c) 
		{
			case 'p':
				if (source == 1) 
				{
					printf("usage: cmd [-p|-s][-n ##]\n");
					exit(1);
				}
				source = 0;
				break;

			case 's':
				if (source == 0) 
				{
					printf("usage: cmd [-p|-s][-n ##]\n");
					exit(1) ;
				}
				source = 1;
				break;

			case 'n':
				nb_message = atoi(optarg);
				break;

			case 'u':
				udp = 1;
				break;

			default:
				printf("usage: cmd [-p|-s][-n ##]\n");
				break;
		}
	}

	// Oubli de préciser puits ou source
	if (source == -1) 
	{
		printf("usage: cmd [-p|-s][-n ##]\n");
		exit(1) ;
	}

	if (source == 1)
	 {
		printf("on est dans le source\n");
		comm_source(argv[argc-2], atoi(argv[argc-1]), udp, lg_message, nb_messages);	
	 }
	else
	{
		printf("on est dans le puits\n");
		comm_puits(atoi(argv[argc-1]), udp, lg_message, nb_messages);	
	}

	if (nb_message != -1) 
	{
		if (source == 1)
			printf("nb de tampons à envoyer : %d\n", nb_message);
		else
			printf("nb de tampons à recevoir : %d\n", nb_message);
	} 
	else 
	{
		if (source == 1) 
		{
			nb_message = 10 ;
			printf("nb de tampons à envoyer = 10 par défaut\n");
		} 
		else printf("nb de tampons à envoyer = infini\n");
	}
}

// Construit un message en répétant lg-5 fois un motif
void construire_message (char *message, char motif, int lg, int numero) 
{
	// Construction de la suite de motifs
	int i;
	for (i=0 ; i<lg; i++) message[i] = motif; 
}

void afficher_message (char *message, int lg) {
	int i;
	printf("message construit : ");
	for (i=0 ; i<lg ; i++) 
	{
		printf("%c", message[i]) ; 
		printf("\n");
	}
}

// Crée un socket qui agit en temps que source
// nom_dest : le nom du poste destinataire
// port : le port du poste destinataire
// udp : 0=tcp, 1=udp
// lg : longueur du message
// nb : nombre de messages
void comm_source(char* nom_dest, int port, int udp, int lg, int nb)
{
	int sock;
	struct sockaddr_in adr_dest;

	// Construction du socket
	if (udp)
	{	
		// UDP		
		sock = socket(AF_INET, SOCK_DGRAM, 0);
	}
	else
	{
		// TCP
		sock = socket(AF_INET, SOCK_STREAM, 0);
	}

	if (sock == -1)
	{
		printf("Erreur lors de la création du socket\n");
		exit(1);
	}
	printf("Socket construit");

	// Création de l'adresse distante
	memset((char*)&adr_dest, 0, sizeof(adr_dest));
	adr_dest.sin_family = AF_INET;
	adr_dest.sin_port = port;
	struct hostent* hp;
	if ((hp = gethostbyname(nom_dest)) == NULL)
	{
		printf("Erreur gethostbyname \n");
		exit(1);
	}
	memcpy((char*)&(adr_dest.sin_addr.s_addr), hp->h_addr, hp->h_length);

	// Envoi des messages
	char* message;
	memset(message, 0, lg);

	char motif;

	if (udp)
	{
		// UDP
		int i;
		for (i=0; i<nb; i++)
		{
			motif = (char)((int)'a' + i % 26);
			construire_message(message, motif,lg, i+1); 
			sendto(sock, message, lg, 0, (struct sockaddr *)&adr_dest, sizeof(adr_dest));
		}
	}
	else
	{
		// TCP
	}

	close(sock);
}



// Crée un socket qui agit en temps que puits
// port : le port de reception
// udp : 0=tcp, 1=udp
// lg : longueur du message
// nb : nombre de messages
void comm_puits(int port, int udp, int lg, int nb)
{
	int sock;
	struct sockaddr_in adr_local;

	// Construction du socket
	if (udp)
	{	
		// UDP		
		sock = socket(AF_INET, SOCK_DGRAM, 0);
	}
	else
	{
		// TCP
		sock = socket(AF_INET, SOCK_STREAM, 0);
	}

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

	// Reception et affichage des messages
	char message[TAILLE_MAX];
	memset(message, 0, lg*sizeof(char));

	struct sockaddr padr_em;

	int plg_adr_em;

	printf("Erreur :)\n");
	if (udp)
	{
		// UDP
		int i;
		for (i=0; i<nb; i++)
		{
			recvfrom(sock, message, lg, 0, &padr_em, &plg_adr_em);
			afficher_message(message, lg);
		}
	}
	else
	{
		// TCP
	}
	close(sock);

}

