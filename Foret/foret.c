#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "foret.h"

//constante&variable reseau
#define BUFFER 1024
int sock;
struct sockaddr_in recv_addr;
struct sockaddr_in exp_addr;
int n, exp_len;
char* buf;
int nbClients = 10;
int indexClient = 0;
Client* clients;

//variable lecture de fichier
int tailleNom = 24;
int nbIpemon = 0;

//variable jeu
int idn = 0;
Ipemon* ipedia;

int memeIP(struct sockaddr_in sin1, struct sockaddr_in sin2) {
	int i;
	i = ((sin1.sin_family == sin2.sin_family
			&& sin1.sin_addr.s_addr == sin2.sin_addr.s_addr));
	return i;
}

void ouvertureLiaison(int port) {
	sock = socket(PF_INET, SOCK_DGRAM, 0); //creation d'une liaison
	if (sock == -1) {
		perror("socket()");
		exit(1);
	}
	bzero((char *) &recv_addr, sizeof recv_addr); //mise a zero de l'adresse de reception
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	recv_addr.sin_port = htons(port);
	if (bind(sock, (struct sockaddr *) &recv_addr, sizeof recv_addr) == -1) { //assigne une adresse a sock
		perror("bind()");
		exit(1);
	}
}

char* reception() {
	exp_len = sizeof exp_addr;
	char* buf = (char*) malloc(BUFFER);
	if ((n = recvfrom(sock, buf, BUFFER, 0, (struct sockaddr *) &exp_addr,
			(socklen_t *) &exp_len)) < 0) {
		perror("recvfrom()");
		exit(1);
	}
	buf[n] = '\0';
	return buf;
}

void envoi(char* buf) {
	if ((n = sendto(sock, buf, BUFFER, 0, (struct sockaddr *) &exp_addr,
			(socklen_t) exp_len)) < 0) {
		perror("sendto()");
		exit(1);
	}
}

//generation de toutes les espéces de Ipemons
Ipemon* generationListe() {
	FILE* ficIpedia;
	ficIpedia = fopen("Ipedia.ip", "rb");
	if (ficIpedia == NULL) {
		perror("fopen()");
		exit(2);
	}
	int i;
	fscanf(ficIpedia, "%d\n", &nbIpemon);
	Ipemon* ipedia = (Ipemon*) malloc(nbIpemon * sizeof(Ipemon));
	for (i = 0; i < nbIpemon; i++) {
		ipedia[i].nom = (char*) malloc(tailleNom);
		fscanf(ficIpedia,
				"%s\n%d:%d:%d:%d:%d:(%d:%d:%d:%d:%d:%d):(%d:%d:%d:%d:%d:%d)\n",
				ipedia[i].nom, &ipedia[i].espece, &ipedia[i].niveauVie,
				&ipedia[i].niveauVieMax, &ipedia[i].experience,
				&ipedia[i].rapidite, &ipedia[i].attaque.air,
				&ipedia[i].attaque.eau, &ipedia[i].attaque.electricite,
				&ipedia[i].attaque.feu, &ipedia[i].attaque.pierre,
				&ipedia[i].attaque.plante, &ipedia[i].defense.air,
				&ipedia[i].defense.eau, &ipedia[i].defense.electricite,
				&ipedia[i].defense.feu, &ipedia[i].defense.pierre,
				&ipedia[i].defense.plante);
	}
	fclose(ficIpedia);
	return ipedia;
}

void debug(Ipemon ip) {
	printf("%s:%d:%d:%d:%d:%d:(%d:%d:%d:%d:%d:%d):(%d:%d:%d:%d:%d:%d)\n",
			ip.nom, ip.espece, ip.niveauVie, ip.niveauVieMax, ip.experience,
			ip.rapidite, ip.attaque.air, ip.attaque.eau, ip.attaque.electricite,
			ip.attaque.feu, ip.attaque.pierre, ip.attaque.plante,
			ip.defense.air, ip.defense.eau, ip.defense.electricite,
			ip.defense.feu, ip.defense.pierre, ip.defense.plante);
	fflush(stdout);
}

void affichageIpemon(Ipemon* ipedia) {
	int i;
	for (i = 0; i < nbIpemon; i++) {
		debug(ipedia[i]);
	}
}

Ipemon stringToIpemon(char* str) {
	Ipemon ip;
	int id;
	sscanf(str, "%d:%d:%d:%d:%d:%d:(%d:%d:%d:%d:%d:%d):(%d:%d:%d:%d:%d:%d)",
			&id, &ip.espece, &ip.niveauVie, &ip.niveauVieMax, &ip.experience,
			&ip.rapidite, &ip.attaque.air, &ip.attaque.eau,
			&ip.attaque.electricite, &ip.attaque.feu, &ip.attaque.pierre,
			&ip.attaque.plante, &ip.defense.air, &ip.defense.eau,
			&ip.defense.electricite, &ip.defense.feu, &ip.defense.pierre,
			&ip.defense.plante);
	ip.nom = ipedia[ip.espece - 1].nom;
	return ip;
}

int diminutionVie(Ipemon ipA, Ipemon ipD) {
	int d;
	int dif = (ipA.experience + ipD.experience) / 2;
	if (!dif)
		dif++; //pour eviter la division par 0
	d = ((ipA.attaque.air * ipA.experience - ipD.defense.air * ipD.experience)
			+ (ipA.attaque.eau * ipA.experience
					- ipD.defense.eau * ipD.experience)
			+ (ipA.attaque.electricite * ipA.experience
					- ipD.defense.electricite * ipD.experience)
			+ (ipA.attaque.feu * ipA.experience
					- ipD.defense.feu * ipD.experience)
			+ (ipA.attaque.pierre * ipA.experience
					- ipD.defense.pierre * ipD.experience)
			+ (ipA.attaque.plante * ipA.experience
					- ipD.defense.plante * ipD.experience)) / (5 * dif);
	d = (int) fabs((double) d);
	return d;
}

int getClient(struct sockaddr_in addr) {
	int i = 0;
	while (i < nbClients) {
		if (memeIP((clients[i].addr), addr)) {
			return i;
		}
		i++;
	}
	return -1;
}

int getClient2(struct sockaddr_in addr, struct sockaddr_in* addrs) {
	int i = 0;
	while (i < nbClients) {
		if (memeIP((addrs[i]), addr)) {
			return i;
		}
		i++;
	}
	return -1;
}

void traitement(char* buf) {
	//M1
	int rand;
	int indx;
	if (strncmp(buf, "NOUVEAU", 7) == 0) {
		idn++;
		rand = random() % nbIpemon;
		bzero(buf, BUFFER);
		sprintf(buf,
				"%d:%d:%d:%d:%d:%d:(%d:%d:%d:%d:%d:%d):(%d:%d:%d:%d:%d:%d)",
				idn, ipedia[rand].espece, ipedia[rand].niveauVie,
				ipedia[rand].niveauVieMax, ipedia[rand].experience,
				ipedia[rand].rapidite, ipedia[rand].attaque.air,
				ipedia[rand].attaque.eau, ipedia[rand].attaque.electricite,
				ipedia[rand].attaque.feu, ipedia[rand].attaque.pierre,
				ipedia[rand].attaque.plante, ipedia[rand].defense.air,
				ipedia[rand].defense.eau, ipedia[rand].defense.electricite,
				ipedia[rand].defense.feu, ipedia[rand].defense.pierre,
				ipedia[rand].defense.plante);
		envoi(buf);
		printf("nouveau pokemon offert\n");
	} //M2
	else if (strncmp(buf, "DUEL_INIT", 9) == 0) {
		char* pseudo = (char*) malloc(tailleNom);
		int val;
		int i = 0, flag = 0, y = 0;
		while (buf[i] != '\0' && flag > -1) {
			if (buf[i] == ' ') {
				flag = !flag;
				if (flag)
					i++;
				else {
					pseudo[y] = '\0';
					flag = -1;
				};
			};
			if (flag > 0) {
				pseudo[y] = buf[i];
				y++;
			}
			i++;
		}
		val = atoi(buf + i);
		//debug
		printf("%s veut combattre et a envoyé la valeur %d\n", pseudo, val);
		Ipemon ipmnF;
		int rand;
		rand = random() % nbIpemon;
		ipmnF = ipedia[rand];
		clients[indexClient].pseudo = pseudo;
		clients[indexClient].addr = exp_addr;
		clients[indexClient].ipemon = ipmnF;
		indexClient++;
		bzero(buf, BUFFER);
		sprintf(buf, "DUEL_OK foret %d", ipmnF.rapidite);
		envoi(buf);
		if (ipmnF.rapidite > val) {
			printf("ipemon foret plus rapide, %s attaque en premier\n",
					ipmnF.nom);
			sprintf(buf,
					"%d:%d:%d:%d:%d:%d:(%d:%d:%d:%d:%d:%d):(%d:%d:%d:%d:%d:%d)",
					idn, ipmnF.espece, ipmnF.niveauVie, ipmnF.niveauVieMax,
					ipmnF.experience, ipmnF.rapidite, ipmnF.attaque.air,
					ipmnF.attaque.eau, ipmnF.attaque.electricite,
					ipmnF.attaque.feu, ipmnF.attaque.pierre,
					ipmnF.attaque.plante, ipmnF.defense.air, ipmnF.defense.eau,
					ipmnF.defense.electricite, ipmnF.defense.feu,
					ipmnF.defense.pierre, ipmnF.defense.plante);
			envoi(buf);
		}
	} //M3
	else if (strstr(buf, "OK") != NULL) {

	} //M4
	else if (strstr(buf, "KO") != NULL) {
		indx = getClient(exp_addr);
		if (indx > -1) {
			printf("%s a perdu\n", clients[indx].pseudo);
			free(clients[indx].pseudo);
			indexClient--;
			clients[indx] = clients[indexClient];
		} else
			puts("somebody losts but we don't know who!!!!!?????");
	} //M5
	else {
		Ipemon ipmnA, ipmnF;
		indx = getClient(exp_addr);
		if (indx > -1) {
			int degat = 0;
			ipmnA = stringToIpemon(buf);
			degat = diminutionVie(ipmnA, clients[indx].ipemon);
			clients[indx].ipemon.niveauVie -= degat;
			ipmnF = clients[indx].ipemon;
			printf("%s a diminué de %d la vie de %s\n", clients[indx].pseudo,
					degat, clients[indx].ipemon.nom);
			if ((ipmnF.niveauVie) > 0) {
				bzero(buf, BUFFER);
				strcpy(buf, "0:A:OK");
				envoi(buf);
				bzero(buf, BUFFER);
				sprintf(buf,
						"%d:%d:%d:%d:%d:%d:(%d:%d:%d:%d:%d:%d):(%d:%d:%d:%d:%d:%d)",
						idn, ipmnF.espece, ipmnF.niveauVie, ipmnF.niveauVieMax,
						ipmnF.experience, ipmnF.rapidite, ipmnF.attaque.air,
						ipmnF.attaque.eau, ipmnF.attaque.electricite,
						ipmnF.attaque.feu, ipmnF.attaque.pierre,
						ipmnF.attaque.plante, ipmnF.defense.air,
						ipmnF.defense.eau, ipmnF.defense.electricite,
						ipmnF.defense.feu, ipmnF.defense.pierre,
						ipmnF.defense.plante);
				envoi(buf);
				bzero(buf, BUFFER);
			} else {
				idn++;
				bzero(buf, BUFFER);
				strcpy(buf, "0:A:KO");
				envoi(buf);
				bzero(buf, BUFFER);
				ipmnF.niveauVie = ipmnF.niveauVieMax;
				sprintf(buf,
						"%d:%d:%d:%d:%d:%d:(%d:%d:%d:%d:%d:%d):(%d:%d:%d:%d:%d:%d)",
						idn, ipmnF.espece, ipmnF.niveauVie, ipmnF.niveauVieMax,
						ipmnF.experience, ipmnF.rapidite, ipmnF.attaque.air,
						ipmnF.attaque.eau, ipmnF.attaque.electricite,
						ipmnF.attaque.feu, ipmnF.attaque.pierre,
						ipmnF.attaque.plante, ipmnF.defense.air,
						ipmnF.defense.eau, ipmnF.defense.electricite,
						ipmnF.defense.feu, ipmnF.defense.pierre,
						ipmnF.defense.plante);
				envoi(buf);
				printf("%s a remporté %s\n", clients[indx].pseudo,
						clients[indx].ipemon.nom);
				free(clients[indx].pseudo);
				indexClient--;
				clients[indx] = clients[indexClient];
			}
		} else {
			puts("pseudo inconue a tenté d'attaquer");
		}
	}
}

void setTimeOut(int t){
	struct timeval tv;
			tv.tv_sec = 3600;
			tv.tv_usec = 0;
			if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))
					< 0) {
				perror("Error");
			}
}

int main(int argc, char** argv) {
	int c;
	c = fork();
	switch (c) {
	case -1:
		perror("fork()");
		exit(2);
		break;
	case 0:
		ouvertureLiaison(6000);
		int max = 0;
		int ind;
		struct sockaddr_in* clients_Addr = (struct sockaddr_in*) malloc(nbClients * sizeof(struct sockaddr_in));
		int* tabVal = (int*) malloc(nbClients * sizeof(int));
		while (1) {
			buf = reception();
			printf("%s\n",buf);
			if (strstr(buf,"init")) {
				puts("new");
				clients_Addr[max] = exp_addr;
				bzero(buf, BUFFER);
				strcpy(buf,"0");
				tabVal[max]=0;
				envoi(buf);
				max++;
			}else{
				ind=getClient2(exp_addr, clients_Addr);
				if(ind>-1){
					int a=atoi(buf);
					if((tabVal[ind]+1)== a){
						bzero(buf, BUFFER);
						a++;
						sprintf(buf,"%d",a);
						envoi(buf);
						tabVal[ind]=a;
					}else{
						puts("des données on était perdu");
						bzero(buf, BUFFER);
						strcpy(buf,"WARNING");
						envoi(buf);
					}
				}else{
					puts("clients inconu et non initialiser");
				}
			}
		}
		close(sock);
		return 0;
		break;
	default:
		clients = (Client*) malloc(nbClients * sizeof(Client));
		ipedia = generationListe();
		ouvertureLiaison(5000);
		while (1) {
			buf = reception();
			traitement(buf);
		}
		close(sock);
		break;
	}
	return 0;
}
