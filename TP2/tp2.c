#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <mpi.h>

#include <unistd.h>


#define Data_Send 1
#define Res_Send 2
#define End 3

#define N 8192
#define num_lines 4


int*  v;
int** m;
int* result;
int* vals;
int* cols;
int* rows;
unsigned int tam=0;
double time_init;

double clear_cache[30000000];

int * resultSeq;


void init_mat(){
    
    v = malloc (N* sizeof(int));	
	
	
	m = malloc(N* sizeof(int *));

	for(unsigned i=0;i<N;i++){
		m[i] = malloc(N* sizeof(int));
	}


	result = calloc(N, sizeof(int));
	resultSeq = calloc(N, sizeof(int));

}


void clear_Cache () {
   for (unsigned i = 0; i < 30000000; i++)
        clear_cache[i] = i;
}



//Funcao que gera o vetor
void generateVetor(){
int i;
srand ( time(NULL) );
for(unsigned i=0;i<N;i++){
	
	v[i] = rand()%100;

	}
}


// Função que gera uma matriz aleatória com muitos zeros
void generateMatrix(int* aux){
	
    int work_batch = 1;
		
	int contador = 0;
	aux[0] = 0;
    srand ( time(NULL)*101 );
	for(unsigned int i = 0; i < N; i++){
		if(i==num_lines*work_batch){
			aux[work_batch] = tam;
			work_batch++;
		}
		for(unsigned int j = 0; j < N; j++){

			if(contador%10 < 6 ){
				m[i][j] = 0;
               
			}			
			else{
			 m[i][j] = rand()%100;
			 tam++;

			}			
			contador++;

		}

	}
	aux[work_batch] = tam;
	
}

//Funcao que constroi a matriz em formato COO 
void getCooFormat(){

	vals  = malloc(tam * sizeof(int));
	cols = malloc(tam * sizeof(int));
	rows = malloc(tam * sizeof(int));


int v = 0;

	for (unsigned i=0;i<N;i++){
		for(unsigned j=0;j<N;j++){
			if(m[i][j]!=0){				
			rows[v] = i; 
            cols[v] = j; 
            vals[v] = m[i][j];				

				v++;
			}
		}
	}
}
//função que faz o calculo da multiplicaco da matriz com o vetor
//Versao sequencial

void multMatrix_seq(){

    for (unsigned int i = 0; i < tam; i++){
    	resultSeq[rows[i]] += v[cols[i]] * vals[i];
    }
	
}

void free_memory(){

        free(cols);
        free(rows);
        free(vals);
        free(v);
        free(result);
        free(m);
}

void start(){

	double time = MPI_Wtime();
	time_init  = time * 1000;
}

double stop(){

	double time = MPI_Wtime();
	double time_end = time * 1000;

	return time_end - time_init;
}

void compare(int size) {
	int i, v = 1;
	for(i = 0;i < size; i++) {
		printf("RES: %d -> RESEQ: %d\n",result[i],resultSeq[i]);
		if(result[i] != resultSeq[i]) {
			v = 0;
		}
	}
	printf("DEU: %d\n",v);
}

int main(int argc, char **argv){

    int rank,num_workers,num_proc,data_size,offset,iter,i;

    MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   //número de trabalhadores que recebem dados
	num_workers = num_proc - 1;

	/*--------------------------------------------- master --------------------------------------------------------*/

	if(rank == 0) { 

		int num_chunks = N / num_lines;
		//array que armazena onde cada parte do array começa
	
		int *work_ind = malloc(num_chunks * sizeof(int));
		
		int worker_rank;

        init_mat();
        
	    generateMatrix(work_ind);
        
	    getCooFormat();
	    generateVetor();
	    
	    clear_Cache();

	    start();
	
	    multMatrix_seq();

	    printf("\nSequential time: %f miliseconds\n\n", stop());

	    clear_Cache();

	    start();

	    int work_batch = 0;
	    int collected = 0; //elementos recebidos 
		// Envia primeiro lote 
		for(i = 1 ; i < num_proc; i++) {
			
			offset = work_ind[work_batch];
			data_size = work_ind[work_batch+1] - offset;

			// Envia tam(elementos diferentes de zero) total
			MPI_Send(&tam, 1, MPI_INT, i, Data_Send, MPI_COMM_WORLD);

			// Envia vetor v apenas uma vez
			MPI_Send(v, N, MPI_INT, i, Data_Send, MPI_COMM_WORLD);

			// Envia número de elementos que slave vai processar
			MPI_Send(&data_size, 1, MPI_INT, i, Data_Send, MPI_COMM_WORLD);

			//Envia iteração do lote de cada slave
			MPI_Send(&work_batch, 1, MPI_INT, i, Data_Send, MPI_COMM_WORLD);
			
			// Enviar trabalho
			MPI_Send(rows+offset, data_size, MPI_INT, i, Data_Send, MPI_COMM_WORLD);
			MPI_Send(cols+offset, data_size, MPI_INT, i, Data_Send, MPI_COMM_WORLD);
			MPI_Send(vals+offset, data_size, MPI_INT, i, Data_Send, MPI_COMM_WORLD);
			work_batch++;
		}


		while(work_batch < num_chunks) {

			MPI_Recv(&iter, 1, MPI_INT, MPI_ANY_SOURCE, Data_Send, MPI_COMM_WORLD, &status);
 			worker_rank = status.MPI_SOURCE;
 			offset = work_ind[iter];
 			data_size = work_ind[iter+1] - offset; 

			// Master recebe trabalho dos escravos
			MPI_Recv(result+(iter*num_lines), num_lines, MPI_INT, worker_rank, Res_Send, MPI_COMM_WORLD, &status);
			collected++;

			offset = work_ind[work_batch];
			data_size = work_ind[work_batch+1] - offset;

			MPI_Send(&data_size, 1, MPI_INT, worker_rank, Data_Send, MPI_COMM_WORLD);

			MPI_Send(&work_batch, 1, MPI_INT, worker_rank, Data_Send, MPI_COMM_WORLD);
			
			// Mandar novo bloco de dados para o slave(enquanto houver dados)
			MPI_Send(rows+offset, data_size, MPI_INT, worker_rank, Data_Send, MPI_COMM_WORLD);
			MPI_Send(cols+offset, data_size, MPI_INT, worker_rank, Data_Send, MPI_COMM_WORLD);
			MPI_Send(vals+offset, data_size, MPI_INT, worker_rank, Data_Send, MPI_COMM_WORLD);
			work_batch++;
		}
       
       // Receber o trabalho das slaves que ainda faltam
          while(collected < num_chunks) {	
			MPI_Recv(&iter, 1, MPI_INT, MPI_ANY_SOURCE, Data_Send, MPI_COMM_WORLD, &status);
 			worker_rank = status.MPI_SOURCE;

			
			MPI_Recv(result+(iter*num_lines), num_lines, MPI_INT, worker_rank, Res_Send, MPI_COMM_WORLD, &status);
			collected++;
		}       

		// AVisar slaves para parar
		for(i = 1 ; i < num_workers+1; i++) {
			MPI_Send(&iter, 1, MPI_INT, i, End, MPI_COMM_WORLD);
		}

		printf("\nTime for Parallel: %f miliseconds\n",stop());

		//compare(N);   	 	
 	}

 	else { 
 		/*---------------------------------------------------- Slaves ----------------------------------------------------------------*/
		
 		v = malloc(N * sizeof(int));
 		result = calloc(N, sizeof(int));		

		int iter=0;

		// Recebem o total de elementos diferentes de 0(tam)
		MPI_Recv(&tam, 1, MPI_INT, 0, Data_Send, MPI_COMM_WORLD, &status);
 		  		
		//Recebe e guarda Vetor v  
 		MPI_Recv(v, N, MPI_INT, 0, Data_Send, MPI_COMM_WORLD, &status);

		while(1) {

 			MPI_Recv(&data_size, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
 			if(status.MPI_TAG == End) {
 				break;
 			}

 			// Recebe iteração para saber onde começa a trabalhar
 			MPI_Recv(&iter, 1, MPI_INT, 0, Data_Send, MPI_COMM_WORLD, &status);

			rows = malloc(data_size*sizeof(int));
			cols = malloc(data_size*sizeof(int));
			vals = malloc(data_size*sizeof(int));

 			// Recebe dados que tem de processar 
 			MPI_Recv(rows, data_size, MPI_INT, 0, Data_Send, MPI_COMM_WORLD, &status);
 			MPI_Recv(cols, data_size, MPI_INT, 0, Data_Send, MPI_COMM_WORLD, &status);
 			MPI_Recv(vals, data_size, MPI_INT, 0, Data_Send, MPI_COMM_WORLD, &status);
 			
 	
 			for(i = 0; i < data_size; i++) {
 				result[ rows[i] ] +=  v[ cols[i] ] * vals[i];
 			}	


 			//Enviar resultado de volta pois a Master é que junta o resultado final
 			MPI_Send(&iter,1,MPI_INT, 0, Data_Send, MPI_COMM_WORLD);

 			//Envia resultados
 			MPI_Send(result+(iter*num_lines),num_lines,MPI_INT,0,Res_Send,MPI_COMM_WORLD);
 			
 			free(rows);
			free(cols);
			free(vals);
 		}
 	}

 	
	MPI_Finalize();
	
	return 0;
}



		

   
