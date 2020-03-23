#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <sched.h>

#define N 4096

int*  v;
int** m;
int* result;
int* resultado;
int* val;
int* cols;
int* rows;
unsigned int t=0;
double time_init;

double clear_cache[30000000];



void init_mat(){

	
	v = malloc (N* sizeof(int));
	
	m = malloc(N* sizeof(int *));

	for(unsigned i=0;i<N;i++){
		m[i] = malloc(N* sizeof(int));
	}

	result = malloc(N*sizeof(int));
	resultado = malloc(N*sizeof(int));
}


void clear_Cache () {
   for (unsigned i = 0; i < 30000000; ++i)
        clear_cache[i] = i;
}



//Funcao que gera o vetor
void generateVetor(){
int i;
srand ( time(NULL) );
for(unsigned i=0;i<N;i++){
	
	v[i] = rand()%100;

       //printf("%d\n",v[i]);

	}
}


// Função que gera uma matriz aleatória com muitos zeros
void generateMatrix(){
	
	float yolo=0.0;
	srand ( time(NULL)*101 );	
	int contador = 0;

	for(unsigned i = 0; i < N; i++){
		for(unsigned j = 0; j < N; j++){

			if(contador%10 < 6 ){
				m[i][j] = 0;
               yolo++;
			}			
			else{
			 m[i][j] = rand()%100;
			}			
			contador++;
	
		}		 
	}
	
}

//Funcao que devolve o tamanho da matriz em formato COO dada a matriz esparsa 
int findTam(){
	

	for (unsigned  i = 0; i < N; i++) {
        for (unsigned j = 0; j < N; j++) {
            if (m[i][j] != 0) 
                t++;
        }
	}
	
	return t;	

}

//Funcao que constroi a matriz em formato COO 
void getCooFormat(){

	val  = malloc(t * sizeof(int));
	cols = malloc(t * sizeof(int));
	rows = malloc(t * sizeof(int));


int v = 0;

	for (unsigned i=0;i<N;i++){
		for(unsigned j=0;j<N;j++){
			if(m[i][j]!=0){				
			rows[v] = i; 
            cols[v] = j; 
            val[v] = m[i][j];				

				v++;
			}
		}
	}
}
//função que faz o calculo da multiplicaco da matriz com o vetor
//Versao sequencial

void multMatrix_seq(){

	for (unsigned i = 0; i < N; i++){
		resultado[i] = 0;
	}


    for (unsigned i = 0; i < t; i++){
    	resultado[rows[i]] += v[cols[i]] * val[i];
    }

}

//função que faz o calculo da multiplicaco da matriz com o vetor
//Versao paralela
void multMatrix_par(){

	#pragma omp parallel
	#pragma omp proc_bind
	{	
	int result_aux[N];
	 
	for (unsigned i = 0; i < N; i++){
		result [i] = 0;
		result_aux[i] = 0;
	}

	#pragma omp for 	 
        for (unsigned i = 0; i < t; i++){
    	result_aux[rows[i]] += v[cols[i]] * val[i];
        }
 
      
   	 	for(unsigned i=0;i<N;i++){
   	 	#pragma omp atomic 
    		result[i]+= result_aux[i];

    	 }
    }

}
void compare() {
	int i, v = 1;
	for(i = 0;i < N; i++) {
		//printf("RES: %d -> RESEQ: %d\n",result[i],resultado[i]);
		if(result[i] != resultado[i]) {
			v = 0;
		}
	}
	printf("DEU: %d\n",v);
}


void free_memory(){

        free(cols);
        free(rows);
        free(val);
        free(v);
        free(result);
        free(m);
}

void start(){

	double time = omp_get_wtime();
	time_init  = time * 1000;
}

double stop(){

	double time = omp_get_wtime();
	double time_end = time * 1000;

	return time_end - time_init;
}

int main(int argc, char const *argv[]){
	
	init_mat();
	generateMatrix();
	findTam();
	getCooFormat();
	generateVetor();
	clear_Cache();

	//Versão sequencial
	start();
	
	multMatrix_seq();

	printf("\nSequential time: %f miliseconds\n\n", stop());

	//Versão paralela
	start();

	multMatrix_par();
	
 	compare();

	printf("\nParallel time: %f miliseconds\n\n", stop());	

        free_memory();

	return 0;


}
