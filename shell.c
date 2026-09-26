#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef struct processd {
	int prc; //номер про-са в массиве строк
	int arg1; //номер 1 аргумента
	int argn; //номер последнего аргумента
	int mod; //запись-1, запись в конец-3
	int modin; //чтение-2
	int fl; //номер файла писать
	int flin; //номер файла читать
	} process;
	
typedef struct operd {
	int posprc; //номер левого про-са в массиве структур process
	int opr; //оператор (&&-1 ||-2 |-3) > (;-4) по приоритету 
	} oper;
	
const char fl1[]=">", fl2[]="<", fl3[]=">>"; 
const char opr1[]="&&", opr2[]="||", opr3[]="|", opr4[]=";";
const char flsmb[][3]={"",">","<",">>"};
const char opsmb[][3]={"","&&","||","|",";"};
int argc;
char **argv;
int cntprc,cntopr;
process (*arrprocess)[];
oper (*arroper)[];

void func_constr (void) {
	int i;
	
	i=1; cntprc=cntopr=0;
	arrprocess = NULL;
	arroper = NULL;
	/*arroper=(oper (*)[])realloc(arroper, (cntopr+1)*sizeof(oper));
	(*arroper)[cntopr].posprc=NULL;
	(*arroper)[cntopr].opr=4;*/
	while (i<argc) {
		
		if (strcmp(argv[i],opr1)==0) { //&&
			cntopr++;
			arroper=(oper (*)[])realloc(arroper, (cntopr+1)*sizeof(oper));
			(*arroper)[cntopr].posprc=cntprc;
			(*arroper)[cntopr].opr=1;
			i++;
		} else if (strcmp(argv[i],opr2)==0) { //||
			cntopr++;
			arroper=(oper (*)[])realloc(arroper, (cntopr+1)*sizeof(oper));
			(*arroper)[cntopr].posprc=cntprc;
			(*arroper)[cntopr].opr=2;
			i++;
		} else if (strcmp(argv[i],opr3)==0) { //|
			cntopr++;
			arroper=(oper (*)[])realloc(arroper, (cntopr+1)*sizeof(oper));
			(*arroper)[cntopr].posprc=cntprc;
			(*arroper)[cntopr].opr=3;
			i++;
		} else if (strcmp(argv[i],opr4)==0) { //;
			cntopr++;
			arroper=(oper (*)[])realloc(arroper, (cntopr+1)*sizeof(oper));
			(*arroper)[cntopr].posprc=cntprc;
			(*arroper)[cntopr].opr=4;
			i++;
		} else { //process
			
			cntprc++;
			arrprocess=(process (*)[])realloc(arrprocess, (cntprc+1)*sizeof(process));
			(*arrprocess)[cntprc].prc=i++;
			(*arrprocess)[cntprc].arg1=0;
			(*arrprocess)[cntprc].mod=0;
			(*arrprocess)[cntprc].modin=0;
			while ((i<argc) && (strcmp(argv[i],opr1)) && (strcmp(argv[i],opr2)) && (strcmp(argv[i],opr3)) && (strcmp(argv[i],opr4))) {
				if (strcmp(argv[i],fl1)==0){ //>
					(*arrprocess)[cntprc].mod=1;
					(*arrprocess)[cntprc].fl=++i;
					i++;
					//break;
				} else if (strcmp(argv[i],fl2)==0){ //<
					(*arrprocess)[cntprc].modin=2;
					(*arrprocess)[cntprc].flin=++i;
					i++;
					//break;
				} else if (strcmp(argv[i],fl3)==0){ //>>
					(*arrprocess)[cntprc].mod=3;
					(*arrprocess)[cntprc].fl=++i;
					i++;
					//break;
				} else { //аргументы процесса
					(*arrprocess)[cntprc].argn=i;
					if ((*arrprocess)[cntprc].arg1==0) (*arrprocess)[cntprc].arg1=i;
					i++;
				}
			}
			
		}
		
	}

}

/*void sort_opr (void) {
	int i,j;
	oper a;
	
	for (i=2;i<=cntopr;i++) {
		for (j=i-1;j>=1;j--) {
			if (((*arroper)[j].opr==4) && ((*arroper)[j+1].opr<4)) {
				a=(*arroper)[j];
				(*arroper)[j]=(*arroper)[j+1];
				(*arroper)[j+1]=a;
			} else break;
		}
	}
	
}*/

void doing_opr (void) {
	int i,pid,schpr,schop,dsr,status,fler;
	int fd[2];
	char *(*args)[];
	
	schpr=1; schop=1; args=NULL;
	while (schpr<=cntprc) {
			
			pipe(fd);
			pid=fork();
			if (pid<0) {perror("fork failed"); exit(1);}
			
			if (pid==0) { //сын
				if ((*arroper)[schop].opr==3) dup2(fd[1],1); //подготовка для конвейера
				close(fd[0]); close(fd[1]);
				
				if ((*arrprocess)[schpr].mod==1) { //запись в файл
					dsr=open(argv[(*arrprocess)[schpr].fl],O_WRONLY|O_CREAT,0777);
					dup2(dsr,1); close(dsr);
				} else if ((*arrprocess)[schpr].mod==3) {
					dsr=open(argv[(*arrprocess)[schpr].fl],O_RDWR|O_CREAT,0777);
					dup2(dsr,1); close(dsr);
				}
				
				if ((*arrprocess)[schpr].modin==2) { //чтение из файла
					dsr=open(argv[(*arrprocess)[schpr].flin],O_RDONLY|O_CREAT,0777);
					dup2(dsr,0); close(dsr);
				}
				
				args=(char *(*)[])realloc(args,sizeof(char *) * ((*arrprocess)[schpr].argn-(*arrprocess)[schpr].arg1+3));
				(*args)[0]=argv[(*arrprocess)[schpr].prc];
				for (i=(*arrprocess)[schpr].arg1; i<=(*arrprocess)[schpr].argn; i++) {
					(*args)[1+i-(*arrprocess)[schpr].arg1]=argv[i];
				}
				(*args)[1+i-(*arrprocess)[schpr].arg1]=NULL;
				
				execvp(argv[(*arrprocess)[schpr].prc], *args);
				
				perror("exec failed");
				exit(1);
			}
			
			//отец
			
			
			
			/*waitpid(pid, &status, 0); //узнаем успешен ли exec
			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status) != 0) {  //задать вопрос по поводу wait
					fler=1;
				} else {
					fler=0;
				}
			}*/
			
			schpr++; //увеличили счетчик процессов
			
			while (schpr<=cntprc) { //обрабатываем операции
				
				if ((*arroper)[schop].opr==4) {close(fd[0]); close(fd[1]); wait(NULL); schop++; break;} // ;
				else if ((*arroper)[schop].opr==3) {dup2(fd[0],0); close(fd[0]); close(fd[1]); schop++; break;} // |
				else  { 
					
					close(fd[0]); close(fd[1]);
					
					waitpid(pid, &status, 0); //узнаем успешен ли exec
					if (WIFEXITED(status)) {
						if (WEXITSTATUS(status) != 0) {  
							fler=1;
						} else {
							fler=0;
						}
					}
					
					if ((*arroper)[schop].opr==2) { // ||
						if (fler) {schop++; break;} //произошла ошибка c 1 операндом
						else {schop++; schpr++; continue;} //нет ошибки c 1 операндом
					} else { // &&
						if (fler) {schop++; schpr++; continue;} //произошла ошибка c 1 операндом
						else {schop++; break;} //нет ошибки c 1 операндом
					}
				}
				
			}
			
			
			
		
	}
	
	
	
}


int main (int margc, char **margv) {
	//int i,j;
	//process a;
	//oper b;
	argc=margc; argv=margv;
	func_constr();
	//sort_opr();
	
	printf("SHELL начал работу\n");
	
	/*for (i=1; i<argc; i++) printf("%s ",argv[i]);
	printf("\n");
	
	for (i=1; i<=cntprc;i++) {
		a=(*arrprocess)[i];
		printf("Process %s ",argv[a.prc]);
		if (a.arg1) for (j=a.arg1; j<=a.argn; j++)  printf("%s ",argv[j]);
		if (a.mod) printf("%s %s ",flsmb[a.mod],argv[a.fl]);
		if (a.modin) printf("%s %s",flsmb[a.modin],argv[a.flin]);
		printf("\n");
	}
		
	for (i=1; i<=cntopr;i++) {
		b=(*arroper)[i];
		printf("%d to do operator  %s left process is %s\n",i,opsmb[b.opr],argv[(*arrprocess)[b.posprc].prc]);
	}*/
	
	doing_opr();
	
	while(wait(NULL)!=-1);
	
	printf("SHELL закончил работу\n");

return 0;
}
