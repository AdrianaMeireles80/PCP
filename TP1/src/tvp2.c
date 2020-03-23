#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define N 16384

int v[N];
int m[N][N];
int resultado[N];
int** sparsa;
unsigned int t=0;

double clear_cache[30000000];


void clear_Cache () {
   for (unsigned i = 0; i < 30000000; ++i)
        clear_cache[i] = i;
}


//Funcao que gera o vetor
void generateVetor(int v[N]){
int i;
srand ( time(NULL) );
for(unsigned i=0;i<N;i++){
	
	v[i] = rand()%1000;

	}
}


// Função que gera uma matriz aleatória com muitos zeros
void generateMatrix( int x,int m[x][x]){
	
	float yolo=0.0;
	srand ( time(NULL)*101 );	
	int contador = 0;

	for(unsigned i = 0; i < x; i++){
		for(unsigned j = 0; j < x; j++){

			if(contador%10 < 6 ){
				m[i][j] = 0;
               yolo++;
			}			
			else{
			 m[i][j] = rand()%1000;
			}			
			contador++;
		
		}		 
	}
	
}

//Funcao que devolve o tamanho da matriz em formato COO dada a matriz esparsa 
int findTam(int matriz[N][N]){
	int tam =0;

	for (unsigned  i = 0; i < N; i++) {
        for (unsigned j = 0; j < N; j++) {
            if (matriz[i][j] != 0) 
                tam++;
        }
	}
	
	return tam;	
}

//Funcao que constroi a matriz em formato COO 
void getCooFormat(int matriz[N][N], int t){

sparsa = malloc(3 * sizeof(int *));
for(unsigned i=0; i<3;i++){
	sparsa[i]= malloc(t * sizeof(int));
}
int v = 0;

	for (unsigned i=0;i<N;i++){
		for(unsigned j=0;j<N;j++){
			if(matriz[i][j]!=0){				
			sparsa[0][v] = i; 
            sparsa[1][v] = j; 
            sparsa[2][v] = matriz[i][j];				

				v++;
			}
		}
	}
}
//função que faz o calculo da multiplicaco da matriz com o vetor
//Versao sequencial

void multMatrix_seq(int matriz[N][N], int x[N]){

	for (unsigned i = 0; i < N; i++){
		resultado [i] = 0;
	}


    for (unsigned i = 0; i < t; i++){
    	resultado[sparsa[0][i]] += x[sparsa[1][i]] * sparsa[2][i];
    }  
}

//função que faz o calculo da multiplicaco da matriz com o vetor
//Versao paralela
void multMatrix_par(int matriz[N][N], int x[N]){

	#pragma omp parallel 
	{	
	int result_aux[N];
	 
	for (unsigned i = 0; i < N; i++){
		resultado [i] = 0;
		result_aux[i] = 0;
	}

	#pragma omp for 	 
        for (unsigned i = 0; i < t; i++){
    	result_aux[sparsa[0][i]] += x[sparsa[1][i]] * sparsa[2][i];
        }
 
      
   	 	for(unsigned i=0;i<N;i++){
   	 		#pragma omp atomic
    		resultado[i] += result_aux[i];

    	 }
    }
}

void free_memory(){
        free(sparsa);
}

int main(int argc, char const *argv[]){
	
	generateMatrix(N,m);
	t= findTam(m);
	getCooFormat(m,t);
	generateVetor(v);
	clear_Cache();
	double start = omp_get_wtime();
	//multMatrix_seq(m,v);
	multMatrix_par(m,v);	
	double end = omp_get_wtime();
	double exec_time =(end - start)*1000;
	printf("the time difference is %g", exec_time);
    free_memory();
	return 0;
}