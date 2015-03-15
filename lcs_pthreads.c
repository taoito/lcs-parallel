#include <pthread.h>
#include "util.h"

void *computeBlock(void * myid);

pthread_barrier_t barr;
int NUM_THREADS;
int SUBY_SIZE;
int SUBMAT_SIZE;
int **fTab;
int xSize,ySize,num_blocksX,num_blocksY;
char **subX;
char **subY;

int main(int argc, char *argv[]) {
	if (argc != 4) {
		printf("Missing arguments: lcs input1 input2 numThreads \n");
		exit(0);
	}
	NUM_THREADS = atoi(argv[3]);
	pthread_t threads[NUM_THREADS];
	//printf("PThreads Num Threads = %d\n",NUM_THREADS);
	double timer_1;
	int i,j,worker,b,k;
	size_t lnlen1,lnlen2;
	char *lineX=NULL; char *lineY=NULL;
	FILE *inputFile1; FILE *inputFile2;

	inputFile1 = fopen(argv[1],"r");
	inputFile2 = fopen(argv[2],"r");
	xSize = getline(&lineX,&lnlen1,inputFile1);
	ySize = getline(&lineY,&lnlen2,inputFile2);
	if (lineX[xSize-1] == '\n')
		xSize--;
	if (lineY[ySize-1] == '\n')
		ySize--;
	fclose(inputFile1); fclose(inputFile2);

	fTab = (int **)malloc((ySize+1) * sizeof(int *));
	for (i = 0; i <= ySize ; i++) {
		fTab[i] = (int *)malloc((xSize+1) * sizeof(int));
	}

	//printf("Size %d and %d \n",ySize,xSize);

	//SUBY_SIZE = ceil((double)ySize / NUM_THREADS); 
	SUBY_SIZE = 100;
	SUBMAT_SIZE = ceil((double)xSize / NUM_THREADS); 
	num_blocksY = ceil((double)ySize / SUBY_SIZE); 
	num_blocksX = ceil((double)xSize / SUBMAT_SIZE); 

	pthread_barrier_init(&barr,NULL,util_min(num_blocksY,NUM_THREADS));

	subX = (char **)malloc(sizeof(char *) * num_blocksX);
	subY = (char **)malloc(sizeof(char *) * num_blocksY);

	for (b = 0; b < num_blocksX; b++) {
		subX[b] = (char *)malloc(sizeof(char) * (SUBMAT_SIZE+1));
		if (((b+1) * SUBMAT_SIZE) <= xSize) {
			strncpy(subX[b],lineX+(b*SUBMAT_SIZE),SUBMAT_SIZE);
		}
		else {
			strncpy(subX[b],lineX+(b*SUBMAT_SIZE),
					xSize - b*SUBMAT_SIZE);
			subX[b][xSize - b*SUBMAT_SIZE] = '\0';
		}
	}

	for (b = 0; b < num_blocksY; b++) {
		subY[b] = (char *)malloc(sizeof(char) * (SUBY_SIZE+1));
		if (((b+1) * SUBY_SIZE) <= ySize) {
			strncpy(subY[b],lineY+(b*SUBY_SIZE),SUBY_SIZE);
			subY[b][SUBY_SIZE] = '\0';
		}
		else {
			strncpy(subY[b],lineY+(b*SUBY_SIZE), ySize - b*SUBY_SIZE);
			subY[b][ySize - b*SUBY_SIZE] = '\0';
		}
	}

	//printf("num blocks %d %d \n",num_blocksY,num_blocksX);

	for ( i = 0; i <= ySize; i++) {
		fTab[i][0] = 0;
	}
	for ( j = 0; j <= xSize; j++) {
		fTab[0][j] = 0;
	}

	clear_timer(timer_1);
  	start_timer(timer_1);

	for (worker = 0; worker < NUM_THREADS; worker++) {	
		pthread_create(&threads[worker],NULL,computeBlock,(void *)&worker);
	}

	for (worker = 0; worker < NUM_THREADS; worker++) {
		pthread_join(threads[worker],NULL);
	}
	/*for ( i = 0; i <= ySize; i++) {
		for ( j = 0; j <= xSize; j++) {
			printf("%d\t",fTab[i][j]);
		}
		printf("\n");
	}*/

  	stop_timer(timer_1);
  	printf("Time Taken: %.2lfs\n", get_timer(timer_1));

	traceback(lineY,lineX,fTab,ySize,xSize,0);
	printf("\n");

	free(lineX); free(lineY);
	for (i = 0; i < num_blocksX; i++) 
		free(subX[i]);
	free(subX);
	for (i = 0; i < num_blocksY; i++) 
		free(subY[i]);
	free(subY);
	for (i = 0; i <= ySize; i++) 
		free(fTab[i]);
	free(fTab);
}

void *computeBlock(void * myid) {

	int id = *(int *)myid ;
	int i,d,j,r,c,rSize,cSize,addR,addC;

	d = 0;
    	for (r = id; r < num_blocksY; r = r + NUM_THREADS) {
		addR = r * SUBY_SIZE;
		rSize = strlen(subY[r]);
		//printf ("at %d NEW - %s\n",id,subY[r]);

		for (c = 0; c <= num_blocksX &&
				d < (num_blocksY+num_blocksX-1) ; c++) { 
	
			if (c == num_blocksX && d < (num_blocksY-1)) {
				break;
			}
			else if (c == num_blocksX && d >= (num_blocksY-1)) {
				pthread_barrier_wait(&barr);
				d++;
				c--;
				continue;
			}
			while (c > (d-r) ) {
				pthread_barrier_wait(&barr);
				d++;
	//			printf("Thread %d agree %d\n",r,d);
			}

			cSize = strlen(subX[c]);
			addC = c * SUBMAT_SIZE;
		//	printf ("at %d and - %s\n",id,subX[c]);

			for (i = 0; i < rSize; i++) {

				for (j = 0; j < cSize ; j++) {

					if (subX[c][j] == subY[r][i]) {
						fTab[i+addR+1][j+addC+1] =
				       			fTab[i+addR][j+addC] + 1;
					}	
					else {
						fTab[i+addR+1][j+addC+1] =
							util_max(fTab[i+addR][j+addC+1],
								fTab[i+addR+1][j+addC]);
					}
				}
			}	
   		}
	}
 	pthread_exit(NULL);
}

double wClockSeconds(void)
{
#ifdef __GNUC__
  struct timeval ctime;

  gettimeofday(&ctime, NULL);

  return (double)ctime.tv_sec + (double).000001*ctime.tv_usec;
#else
  return (double)time(NULL);
#endif
}

int traceback(char *lineY, char *lineX, int **fTab, int i, int j, int count) {
	if (i == 0 || j == 0) {
		printf("Length = %d\n",count);
		return;
	}
	if (lineX[j-1] == lineY[i-1]) {
		count++;
		traceback(lineY,lineX,fTab,i-1,j-1,count);
		printf("%c",lineX[j-1]);
	}
	else if (fTab[i-1][j] == fTab[i][j]) {
		traceback(lineY,lineX,fTab,i-1,j,count);
	}
	else {
		traceback(lineY,lineX,fTab,i,j-1,count);
	}
}
