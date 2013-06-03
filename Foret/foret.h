#ifndef _foret
#define _foret

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

typedef struct {
	char* pseudo;
	struct sockaddr_in addr;
	Ipemon ipemon;
}Client;

int memeIP(struct sockaddr_in sin1, struct sockaddr_in sin2);

int getClient(struct sockaddr_in addr);


int getClient2(struct sockaddr_in addr, struct sockaddr_in* addrs);

void ouvertureLiaison(int port);

char* reception();

void envoi(char* buf);

Ipemon* generationListe();

void debug(Ipemon ip);

void affichageIpemon(Ipemon* ipedia);

Ipemon stringToIpemon(char* str);

int diminutionVie(Ipemon ipA, Ipemon ipD);

void traitement(char* buf);

int main(int argc, char** argv);

#endif
