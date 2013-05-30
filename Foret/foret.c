#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

//constante&variable reseau
#define BUFFER 1024
int sock;
struct sockaddr_in recv_addr;
struct sockaddr_in exp_addr;
int n, exp_len;
char* buf;

//variable lecture de fichier
int tailleNom = 24;
int nbIpemon = 0;

//variable jeu
typedef struct {
	int feu;
	int eau;
	int electricite;
	int plante;
	int air;
	int pierre;
} AttDef;

typedef struct {
	char* nom;
	int espece;
	int niveauVie;
	int niveauVieMax;
	int experience;
	int rapidite;
	AttDef attaque;
	AttDef defense;
} Ipemon;

int idn = 0;
Ipemon* ipedia;

void ouvertureLiaison() {
	sock = socket(PF_INET, SOCK_DGRAM, 0); //creation d'une liaison
	if (sock == -1) {
		perror("socket()");
		exit(1);
	}
	bzero((char *) &recv_addr, sizeof recv_addr); //mise a zero de l'adresse de reception
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	recv_addr.sin_port = htons(5000);
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

void debug(Ipemon ip){
	printf("%s:%d:%d:%d:%d:%d:(%d:%d:%d:%d:%d:%d):(%d:%d:%d:%d:%d:%d)\n",
					ip.nom, ip.espece, ip.niveauVie,
					ip.niveauVieMax, ip.experience,
					ip.rapidite, ip.attaque.air,
					ip.attaque.eau, ip.attaque.electricite,
					ip.attaque.feu, ip.attaque.pierre,
					ip.attaque.plante, ip.defense.air,
					ip.defense.eau, ip.defense.electricite,
					ip.defense.feu, ip.defense.pierre,
					ip.defense.plante);
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
			&id, &ip.espece, &ip.niveauVie,
			&ip.niveauVieMax, &ip.experience, &ip.rapidite,
			&ip.attaque.air, &ip.attaque.eau,
			&ip.attaque.electricite, &ip.attaque.feu,
			&ip.attaque.pierre, &ip.attaque.plante,
			&ip.defense.air, &ip.defense.eau,
			&ip.defense.electricite, &ip.defense.feu,
			&ip.defense.pierre, &ip.defense.plante);
	ip.nom = ipedia[ip.espece - 1].nom;
	return ip;
}

int diminutionVie(Ipemon ipA, Ipemon ipD) {
	int d;
	int dif = (ipA.experience + ipD.experience)/2;
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

void combat(char* ps, int vl) {
	Ipemon ipmnF, ipmnA;
	int rand, flag = 1, degat = 0;
	rand = random() % nbIpemon;
	ipmnF = ipedia[rand];
	bzero(buf, BUFFER);
	sprintf(buf, "DUEL_OK foret %d", ipmnF.rapidite);
	envoi(buf);
	if (vl > ipmnF.rapidite) {
		while (flag) {
			bzero(buf, BUFFER);
			buf = reception();
			ipmnA = stringToIpemon(buf);
			degat = diminutionVie(ipmnA, ipmnF);
			ipmnF.niveauVie -= degat;
			printf("%d\n",ipmnF.niveauVie);
			if ((ipmnF.niveauVie)>0) {
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
				buf = reception();
				char* res = strstr(buf, "OK");
				if (res == NULL) {
					flag = 0;
					puts("Adversaire a perdu");
				}
			} else {
				idn++;
				bzero(buf, BUFFER);
				strcpy(buf, "0:A:KO");
				envoi(buf);
				bzero(buf, BUFFER);
				ipmnF.niveauVie= ipmnF.niveauVieMax;
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
				flag = 0;
			}
		}
	} else {
		while (flag) {
			bzero(buf, BUFFER);
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
			bzero(buf, BUFFER);
			buf = reception();
			char* res = strstr(buf, "OK");
			if (res == NULL) {
				flag = 0;
				puts("Adversaire a perdu");
			} else {
				bzero(buf, BUFFER);
				buf = reception();
				ipmnA = stringToIpemon(buf);
				degat = diminutionVie(ipmnA, ipmnF);
				ipmnF.niveauVie -= degat;
				if (ipmnF.niveauVie > 0) {
					bzero(buf, BUFFER);
					strcpy(buf, "0:A:OK");
					envoi(buf);
				} else {
					idn++;
					bzero(buf, BUFFER);
					strcpy(buf, "0:A:KO");
					envoi(buf);
					bzero(buf, BUFFER);
					sprintf(buf,
							"%d:%d:%d:%d:%d:%d:(%d:%d:%d:%d:%d:%d):(%d:%d:%d:%d:%d:%d)",
							idn, ipmnF.espece, ipmnF.niveauVie,
							ipmnF.niveauVieMax, ipmnF.experience,
							ipmnF.rapidite, ipmnF.attaque.air,
							ipmnF.attaque.eau, ipmnF.attaque.electricite,
							ipmnF.attaque.feu, ipmnF.attaque.pierre,
							ipmnF.attaque.plante, ipmnF.defense.air,
							ipmnF.defense.eau, ipmnF.defense.electricite,
							ipmnF.defense.feu, ipmnF.defense.pierre,
							ipmnF.defense.plante);
					envoi(buf);
					flag = 0;
				}

			}

		}
	}
}

int main(int argc, char** argv) {
	ipedia = generationListe();
	ouvertureLiaison();
	int rand;
	while (1) {
		buf = reception();
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
		} else if (strncmp(buf, "DUEL_INIT", 9) == 0) {
//debug
			printf("%s\n", buf);
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
			printf("%s,%d\n", pseudo, val);
			combat(pseudo, val);
		}
	}
	close(sock);
	return 0;
}
