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

//void signal

typedef struct {
	char * mots[MAXL];
	int nb_mots;
} dico_t;

int fd;

static dico_t DICO;

void error(char * s){
	//fprintf(stderr, "%s\n", s);
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

void coder_couple(int couple[2]){
	int i;
	int mqinf = 0b1111;
	int mqsup = 0b11111111;
	int octet;
	int qinf;
	int qsup;
	static int cpt = 0;
	cpt ++;

	////fprintf(stderr, "\n(%d, %d) == DICO.mots[0] : %s\n", couple[0], couple[1], DICO.mots[couple[0]]);
	for( i = 1; i <= 3; i++){
		switch(i){
			case 1:
				octet = couple[0] >> 4;
				break;
			case 2:
				qinf = couple[0] & mqinf;
				qsup = couple[1] >> 8;
				octet = qinf << 4 | qsup;
				break;
			case 3:
				octet = couple[1] & mqsup;					
				break;
		}
		//fprintf(stderr, "%d,  ", octet);
		if(write(fd, &octet, sizeof(int)) == -1) error("write coder couple 1");
	}
	//fprintf(stderr, "on envoie : |%s| et |%s|\n\n", DICO.mots[couple[0]], DICO.mots[couple[1]]);
}


void emettre( const int code ) {
	static int cpt = 0;
	static int couple[2];
	if(code == END){
		if(cpt == 1) couple[1] = END;
		if(cpt == 0){
			couple[0] = END;
			couple[1] = END;
		}
		coder_couple(couple);
		exit(0);
	} 
	
	cpt += 1;

	if( cpt == 2 ) {
		cpt = 0; // le couple est complet on recommence avec un nouveau couple
		couple[1] = code;
		/**
		10 * @brief transforme le couple d'entier en triplet d'octets
		11 * pusi écrit le triplet dans le fichier binaire qui sera
		12 * communiqué au client.
		13 *
		**/
		coder_couple( couple );
	}
	else couple[0] = code;
}


int main( int argc, char ** argv ) {
	if(argc != 2) error("il faut donner le nom du fichier");
	char c;
	
	if(mkfifo("pipe", 777) == -1);// perror("mkfifo");
	if((fd = open("pipe", O_WRONLY)) == -1) perror("open after mkfifo");


	FILE * f = NULL;
	f = fopen(argv[1], "r");
	FILE * file = NULL;
	if((file = fopen("donne", "w")) == NULL) error("fopen");
	
	if(f == NULL) error("fopen");

	int j;
	char S[MAXL];

	for(j = 0; j < 256; j++){
			DICO.mots[j] = (char *)malloc(sizeof(char));
			DICO.mots[j][0] = (char)j;
			DICO.mots[j][1] = '\0';
	}

	DICO.nb_mots = 256;
	int lgth = 0;
	S[0] = '\0';
	int cpt = 0;

	while((c = getc(f)) != EOF){
		int i;
		int ind;
		lgth = strlen(S);
		S[lgth] = c;
		S[lgth+1] = '\0';
		for( i=0; i < DICO.nb_mots && strcmp(S,DICO.mots[i]); i+=1 ) ;
		if( i == DICO.nb_mots ) {
			
			if(DICO.nb_mots != 3840){
				DICO.mots[DICO.nb_mots] = (char *)malloc((lgth+1)*sizeof(char));
				strcpy(DICO.mots[DICO.nb_mots++], S);
			}
			

			S[lgth] = '\0'; // on enlève c
			for( i=0; i < DICO.nb_mots && strcmp(S,DICO.mots[i]); i+=1 ) ;

			emettre(i);
			if(!(cpt++%25)) fprintf(file, "\n");
			fprintf(file, "%d ", i);
			S[0] = c; // on réinitialise la chaîne avec c
			S[1] = '\0';
		}

	}

	emettre(END);
	fclose(f);
	close(fd);
	fclose(file);
	if(remove("pipe") == -1) error("remove");
	
	for(int i = 0; i < DICO.nb_mots; i++)
	{
		free(DICO.mots[i]);
	}
	exit(0);
}
