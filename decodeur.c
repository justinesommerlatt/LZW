#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>


#define END 0
#define MAXL 3840

FILE * output = NULL;

typedef struct {
	char * mots[MAXL];
	int nb_mots;
} dico_t;

typedef struct {
	char * text;
	int nb_lettres;
} texte;

int fd;
int continuer = 1;
int afficher = 0;
int couple[2];

static dico_t DICO;

static texte texte_decode;

void error(char * s){
	fprintf(stderr, "%s\n", s);
	exit(0);
}

void recursive_binary(unsigned int number){
    if (number >> 1) {
        recursive_binary(number >> 1);
    }
    putc((number & 1) ? '1' : '0', stdout);
}

void print_binary(unsigned int number){
	recursive_binary(number);
	putc('\n', stdout);
}

void decoder_triplet(int triplet[3]){
	int i;
	int qinf;
	int qsup;
	int first_octet;
	int second_octet;
	for( i = 1; i <= 3; i++){
		switch(i){
			case 1:
				first_octet = triplet[0] << 4;
				break;
			case 2:
				qsup = triplet[1] & 0b1111;
				qinf = triplet[1] >> 4;
				first_octet = first_octet | qinf;
				second_octet = qsup << 8;
				break;
			case 3:
				second_octet = second_octet | triplet[2];
				break;
		}
	}

	couple[0] = first_octet;
	couple[1] = second_octet;
	if(couple[1] == END) continuer = 0;
}

void decoder_couple(int couple[2]){
	int i;
	static char m;
	static char M[MAXL];
	static char S[MAXL];
	static int cpt = 0;
	for(i = 0; i < 2; i++){

		int lgth = strlen(S);

		if(couple[i] < 256){
			int j;
			m = (char) couple[i];
			S[lgth] = m;
			S[lgth+1] = '\0';
			for( j = 0; j < DICO.nb_mots && strcmp(S,DICO.mots[j]); j +=1 ) ; //presence de S+m dans le dico ?
			if( j == DICO.nb_mots){		//pas present
				if(DICO.nb_mots != 3840){
					DICO.mots[DICO.nb_mots] = (char *)malloc((lgth+1)*sizeof(char));
					strcpy(DICO.mots[DICO.nb_mots++], S);
				}
				fprintf(output,"%c", m);
				S[lgth] = '\0'; // on enlève c		
				S[0] = m; // on réinitialise la chaîne avec c
				S[1] = '\0';
			}
		}else if (couple[i] == DICO.nb_mots){
			S[lgth] = S[lgth - 1];
			S[lgth + 1] = '\0';
			if(DICO.nb_mots != 3840){
				DICO.mots[DICO.nb_mots] = (char *)malloc((lgth+1)*sizeof(char));
				strcpy(DICO.mots[DICO.nb_mots++], S);
			}
		}
		else{
			int j;
			int k;

			int lgthM = strlen(DICO.mots[couple[i]]);

			for(j = 0; j < lgthM; j++) M[j] = DICO.mots[couple[i]][j];
			M[j] = '\0';											//m = valeur(c)
			for(j = 0; j < lgthM; j++){

				lgth = strlen(S);
				S[lgth] = M[j];
				S[lgth+1] = '\0';


				for(k=0; k < DICO.nb_mots && strcmp(S,DICO.mots[k]); k+=1 ) ; //presence de S+Cm dans le dico ?
				if( k == DICO.nb_mots){		//pas present

					fprintf(output,"%s", M);

					if(DICO.nb_mots != 3840){
						DICO.mots[DICO.nb_mots] = (char *)malloc((lgth+1)*sizeof(char));
						strcpy(DICO.mots[DICO.nb_mots++], S);
					}
					S[0] = M[0];
					S[1] = '\0';			
				}
			}
		}
	}



}

void recevoir( int oc ) {
	static int cpt = 0;
	static int triplet[3];
	int i;
	cpt += 1;
	 if( cpt == 3 ) {

		cpt = 0; // le triplet est comple, on recommence avec un nouveau triplet
		triplet[2] = oc;	 			
		/**
		* @brief decoder_triplet renvoie le couple d'entiers
		* correspondant au triplet passé en argument
		*/

		decoder_triplet( triplet);

		/**
		* @brief À l'aide de l'algorithme LZW les couples sont traduits
		* en caractères qui sont rangés dans fdNewBook.
		*/

		decoder_couple( couple );

	}
	else triplet[cpt-1] = oc;

}


int main( int argc, char ** argv ) {
	int octet;
	texte_decode.nb_lettres = 1;
	texte_decode.text = (char*) malloc(sizeof(char));
	texte_decode.text[0] = '\0';
	
	if((output = fopen("output", "w")) == NULL) fprintf(stderr, "erreur fopen\n");

	static int cpt = 0;

	if((fd = open("pipe", O_RDONLY)) == -1) perror("open after mkfifo");

	int j;
	for(j = 0; j < 256; j++){
			DICO.mots[j] = (char *)malloc(sizeof(char));
			DICO.mots[j][0] = (char)j;
			DICO.mots[j][1] = '\0';
	}
	DICO.nb_mots = 256;
	do{

		if(read(fd, &octet, sizeof(int)) == -1) error("read main");
		
		recevoir(octet);

	}while(continuer);



	close(fd);
	fprintf(output, "\n");
	if(remove("pipe") == -1) error("remove");
	
	for(int i = 0; i < DICO.nb_mots; i++)
	{
		free(DICO.mots[i]);
	}
	fclose(output);
	exit(0);
}