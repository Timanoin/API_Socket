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
#define NB_MESSAGES_DEF 10

// Construit un message en répétant lg-5 fois un motif
void construire_message (char *message, char motif, int lg, int numero);

void afficher_message (char *message, int lg);

// Crée un socket qui agit en temps que source
// nom_dest : le nom du poste destinataire
// port : le port du poste destinataire
// udp : 0=tcp, 1=udp
// lg : longueur du message
// nb : nombre de messages
void source(char* nom_dest, int port, int udp, int lg, int nb);

// Crée un socket qui agit en temps que puits
// port : le port de reception
// udp : 0=tcp, 1=udp
// lg : longueur du message
// nb : nombre de messages
void puits(char* nom_dest, int port, int udp, int lg, int nb);


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

	while ((c = getopt(argc, argv, "pn:s")) != -1) 
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
		source(argv[argc-2], argv[argc-1], udp);	
	 }
	else
	{
		printf("on est dans le puits\n");
		puits(argv[argc-1], udp);	
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
	

	int i;
	for (i=0 ; i<lg; i++) message[i] = motif; 
}

void afficher_message (char *message, int lg) {
	int i;
	printf("message construit : ");
	for (i=5 ; i<lg ; i++) 
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
void source(char* nom_dest, int port, int udp, int lg, int nb)
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
		printf("Erreur lors de la création du socket");
		exit(1);
	}

	// Création de l'adresse distante
	memset((char*)&adr_local, 0, sizeof(adr_local))
	adr_dest.sin_family = AF_INET
	adr_dest.sin_port = argv[argc-1]
	adr_dest.sin_addr.s_addr = gethostbyname(argv[argc-2])->h_addr_list[0]
	if (adr_dest.sin_addr.s_addr == NULL)
	{
		printf("Erreur lors de la détermination de l'IP de destination");
		exit(1);
	}

	// Envoi des messages
	char* message;
	memset(message, 0, lg)
	if (udp)
	{
		// UDP
		sendto(sock, message, lg, 0, (struct sockaddr *)&adr_local, sizeof(adr_local));
	}
	else
	{
		// TCP
	}



// Crée un socket qui agit en temps que puits
// port : le port de reception
// udp : 0=tcp, 1=udp
// lg : longueur du message
// nb : nombre de messages
void puits(char* nom_dest, int port, int udp, int lg, int nb);

