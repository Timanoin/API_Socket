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


// Retourne la longueur d'un nombre (nombre de caractères nécessaires à son écriture)
int longueur_nombre(int nombre);

// Construit un message en répétant lg-5 fois un motif
void construire_message (char *message, char motif, int lg, int numero);

// Affichage en mode puits
void afficher_message_puits (char *message, int lg, int numero);

// Affichage en mode source
void afficher_message_source (char *message, int lg, int numero);

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
	int source = -1 ; /* 0=puits, 1=source */
	int udp = 0; // 0=tcp, 1=udp
	int nb_messages = NB_MESSAGES_DEF;
	int lg_message = LG_MESSAGE_DEF;

	while ((c = getopt(argc, argv, "pul:n:s")) != -1) 
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
				nb_messages = atoi(optarg);
				break;

			case 'u':
				udp = 1;
				break;

			case 'l' : 
				lg_message = atoi(optarg);
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
		comm_source(argv[argc-2], htons(atoi(argv[argc-1])), udp, lg_message, nb_messages);	
	 }
	else
	{
		printf("on est dans le puits\n");
		comm_puits(htons(atoi(argv[argc-1])), udp, lg_message, nb_messages);	
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

// Affichage en mode source
void afficher_message_source (char *message, int lg, int numero) {
	int i;
	printf("SOURCE : Envoi n°%d (%d) [", numero+1, lg);
	for (i=0 ; i<lg ; i++) 
	{
		printf("%c", message[i]) ; 
	}
	printf("]\n");
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
	char message[TAILLE_MAX];
	memset(message, 0, lg);

	char motif;

	if (udp)
	{
		// UDP
		// On construit le message. On l'envoie en UDP avec sendto 
		// en précisant le socket d'arrivée. 
		// On répète l'opération pour le nombre de messages choisi.

		int i;
		for (i=0; i<nb; i++)
		{
			motif = (char)((int)'a' + i % 26);
			construire_message(message, motif,lg, i+1); 
			sendto(sock, message, lg, 0, (struct sockaddr *)&adr_dest, sizeof(adr_dest));
			afficher_message_source(message, lg, i);
		}
	}
	else
	{
		// TCP
		// On envoie une demande de connexion à la machine distante.
		// Lorsque la demande est acceptée, on envoie les messages.

		connect(sock, (struct sockaddr *)&adr_dest, sizeof(adr_dest)); 

		int i;
		for (i=0; i<nb; i++)
		{
			motif = (char)((int)'a' + i % 26);
			construire_message(message, motif,lg, i+1); 
			write(sock, message, lg);
			afficher_message_source(message, lg, i);
		}

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
	int sock, sock_bis;
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
		sock = socket(AF_INET, SOCK_STREAM, 0);; 
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

	struct sockaddr_in padr_em;
	int plg_adr_em = sizeof(padr_em);

	if (udp)
	{
		// UDP
		// On attend un message. Quand il est recu, il est affiché.
		// Puis on retourne en attente si il reste des messages à recevoir.

		int i;
		for (i=0; i<nb; i++)
		{
			recvfrom(sock, message, lg, 0, (struct sockaddr*)&padr_em, &plg_adr_em);
			afficher_message_puits(message, lg, i);
		}
	}
	else
	{
		// TCP
		// On utilise listen pour construire une file d'attente des messages recus en simultané.
		// Puis on attend une demande de connexion. Lorsqu'une demande est recue, elle est acceptée.
		// Alors on divise le programme : 
		// - un fils qui va gérer la réception des messages 
		//   et leur affichage, qui meurt une fois sa tache terminée.
		// - le père qui continue à recevoir des demandes de connexion.

		listen(sock, NB_MAX);
		while (1) 
		{
			sock_bis = accept(sock, (struct sockaddr*)&padr_em, &plg_adr_em);

			if (sock_bis==-1)
			{
				perror("accept"); 
			}	
			switch (fork()) 
			{
				case -1 : 
					printf("erreur fork \n"); 
					exit(1);
				case 0: 
					close(sock);
					int i;
					int lg_rec; 
					for (i=0; i<nb; i++)
					{
						if ((lg_rec=read(sock_bis, message, lg)) < 0)
						{
							//perror("echec du read\n");
							exit(1); 
						}
						afficher_message_puits(message, lg_rec, i);
					}
					exit(0); 
				default : 
				close(sock_bis);
			}
		}

	}
	close(sock);
}

