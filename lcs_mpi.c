#include <mpi.h>
#include "util.h"

int main(int argc, char *argv[]) {
	int num_procs, myid, name_len;
	char proc_name[MPI_MAX_PROCESSOR_NAME];

	int SUBY_SIZE;
	int SUBMAT_SIZE;
	int **fTab;
	int xSize,ySize,num_blocksX,num_blocksY,sendYSize;
	char **subX;
	char **listSubY;
	int initInfo[2];
	int traceInfo[2];
	char *lineX=NULL; 
	char *mySubY=NULL;
	char *recvX=NULL;

	MPI_Status status;
	MPI_Request request;
	double timer_1;
	int i,j,b,k;
	int r,c,rSize,cSize,addR,addC;
	int d = 0;

	MPI_Init(&argc,&argv);

	MPI_Comm_size(MPI_COMM_WORLD,&num_procs);

	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	MPI_Get_processor_name(proc_name, &name_len);

	if ( myid == 0 ) {

		size_t lnlen1,lnlen2;
		char *lineY=NULL;
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

		//printf("Processor %d out of %d, executing on %s. Read files of size %d and %d \n",myid, num_procs, proc_name,xSize,ySize);

		SUBMAT_SIZE = ceil((double)xSize / num_procs); 
		SUBY_SIZE = ceil((double)ySize / num_procs);

		initInfo[0] = xSize;
		initInfo[1] = ySize;
		subX = (char **)malloc(sizeof(char *) * num_procs);

		for (b = 0; b < num_procs; b++) {
			if (((b+1) * SUBMAT_SIZE) <= xSize) {
				subX[b] = (char *)malloc(sizeof(char) * SUBMAT_SIZE);
				strncpy(subX[b],lineX+(b*SUBMAT_SIZE),SUBMAT_SIZE);
			}
			else {
				subX[b] = (char *)malloc(sizeof(char) * (xSize - b*SUBMAT_SIZE));
				strncpy(subX[b],lineX+(b*SUBMAT_SIZE),
					xSize - b*SUBMAT_SIZE);
			}
		}

		listSubY = (char **)malloc(sizeof(char*) * num_procs);

		for (b = 0; b < num_procs; b++) {
			if (((b+1) * SUBY_SIZE) <= ySize) {
				listSubY[b] = (char *)malloc(sizeof(char) * SUBY_SIZE);
				strncpy(listSubY[b],lineY+(b*SUBY_SIZE),SUBY_SIZE);
			}
			else {
				listSubY[b] = (char *)malloc(sizeof(char) * (ySize - b*SUBY_SIZE));
				strncpy(listSubY[b],lineY+(b*SUBY_SIZE), ySize - b*SUBY_SIZE);
			}
		}

		MPI_Bcast(initInfo,5,MPI_INT,0,MPI_COMM_WORLD);
		MPI_Bcast(lineX,xSize,MPI_CHAR,0,MPI_COMM_WORLD);

		for (k = 1; k < num_procs; k++) {
			//printf("Send to %d this::: %s \n",k,listSubY[k]);
			MPI_Send(listSubY[k],SUBY_SIZE,MPI_CHAR,k,0,MPI_COMM_WORLD);
		}

		rSize = util_min(SUBY_SIZE,ySize); 

		fTab = (int **)malloc(sizeof(int *)*(rSize+1));
		for (i = 0; i <= rSize ; i++) {
			fTab[i] = (int *)malloc((xSize+1) * sizeof(int));
		}
		for ( i = 0; i <= rSize; i++) {
			fTab[i][0] = 0;
		}
		for ( j = 0; j <= xSize; j++) {
			fTab[0][j] = 0;
		}

		clear_timer(timer_1);
  		start_timer(timer_1);

		for (c = 0; c <= num_procs && d < (num_procs*2-1); c++) {

			if (c == num_procs) {
				MPI_Barrier(MPI_COMM_WORLD);
				d++;
				c--;
				continue;
			}
			while (c > (d - myid)) {
				MPI_Barrier(MPI_COMM_WORLD);
				d++;
			}
			if (c < num_procs-1)
				cSize = SUBMAT_SIZE;
			else
				cSize = xSize - c*SUBMAT_SIZE;
			addC = c * SUBMAT_SIZE;
			for (i = 0; i < rSize; i++) {
				for (j = 0; j < cSize ; j++) {
					if (subX[c][j] == listSubY[0][i]) {
						fTab[i+1][j+addC+1] =
				       			fTab[i][j+addC] + 1;
					}	
					else {
						fTab[i+1][j+addC+1] =
							util_max(fTab[i][j+addC+1],
								fTab[i+1][j+addC]);
					}
				}
			}	
			if (num_procs > 1) 
			MPI_Isend(fTab[rSize]+addC,SUBMAT_SIZE,MPI_INT,myid+1,1,
					MPI_COMM_WORLD,&request);
		}

	/*printf("Proc %d \n",myid);
	for ( i = 0; i <= rSize; i++) {
		for ( j = 0; j <= xSize; j++) {
			printf("%d\t",fTab[i][j]);
		}
		printf("\n");
	}*/

// Working Processes
	} else {
		MPI_Bcast(initInfo,5,MPI_INT,0,MPI_COMM_WORLD);
		xSize = initInfo[0];
		ySize = initInfo[1];
		SUBY_SIZE = ceil((double)ySize / num_procs);
		//printf("%d %d \n",xSize,num_procs);
		SUBMAT_SIZE = ceil((double)xSize / num_procs); 
		lineX = (char *)malloc(sizeof(char)*(xSize));
		MPI_Bcast(lineX,xSize,MPI_CHAR,0,MPI_COMM_WORLD);

		subX = (char **)malloc(sizeof(char *) * num_procs);
		for (b = 0; b < num_procs; b++) {
			if (((b+1) * SUBMAT_SIZE) <= xSize) {
				subX[b] = (char *)malloc(sizeof(char) * (SUBMAT_SIZE));
				strncpy(subX[b],lineX+(b*SUBMAT_SIZE),SUBMAT_SIZE);
			}
			else {
				subX[b] = (char *)malloc(sizeof(char) * (xSize - b*SUBMAT_SIZE));
				strncpy(subX[b],lineX+(b*SUBMAT_SIZE),(xSize - b*SUBMAT_SIZE));
			}
		}

		mySubY = (char *)malloc(sizeof(char) * SUBY_SIZE);
		MPI_Recv(mySubY,SUBY_SIZE,MPI_CHAR,0,0,MPI_COMM_WORLD,&status);
		//printf("Processor %d has this str %s %d \n",myid,mySubY,strlen(mySubY));

		if (myid < num_procs-1)
			rSize = SUBY_SIZE;
		else
			rSize = ySize - myid*SUBY_SIZE;

		fTab = (int **)malloc((rSize+1) * sizeof(int *));
		for (i = 0; i <= rSize ; i++) {
			fTab[i] = (int *)malloc((xSize+1) * sizeof(int));
		}
		for ( i = 0; i <= rSize; i++) {
			fTab[i][0] = 0;
		}
		for ( j = 0; j <= xSize; j++) {
			fTab[0][j] = 0;
		}

		for (c = 0; c <= num_procs && d < (num_procs*2-1); c++) {

			if (c == num_procs) {
				MPI_Barrier(MPI_COMM_WORLD);
				d++;
				c--;
				continue;
			}
			while (c > (d - myid)) {
				MPI_Barrier(MPI_COMM_WORLD);
				d++;
			}
			if (c < num_procs-1)
				cSize = SUBMAT_SIZE;
			else
				cSize = xSize - c*SUBMAT_SIZE;
			addC = c * SUBMAT_SIZE;
			MPI_Recv(fTab[0]+addC,SUBMAT_SIZE,MPI_INT,myid-1,1,
					MPI_COMM_WORLD,&status);
			for (i = 0; i < rSize; i++) {
				for (j = 0; j < cSize ; j++) {
					if (subX[c][j] == mySubY[i]) {
						fTab[i+1][j+addC+1] =
				       			fTab[i][j+addC] + 1;
					}	
					else {
						fTab[i+1][j+addC+1] =
							util_max(fTab[i][j+addC+1],
								fTab[i+1][j+addC]);
					}
				}
			}	
			if (myid < num_procs-1) {
			MPI_Isend(fTab[rSize]+addC,SUBMAT_SIZE,MPI_INT,myid+1,1,
					MPI_COMM_WORLD,&request);
			}
		}
	/*printf("Proc %d \n",myid);
	for ( i = 0; i <= rSize; i++) {
		for ( j = 0; j <= xSize; j++) {
			printf("%d\t",fTab[i][j]);
		}
		printf("\n");
	}*/
	}
	
	char res[util_max(rSize,xSize)];
	int count,currentJ;

	//traceback
	if (myid == 0) {
		count = 0;
		currentJ = xSize;
		if (num_procs > 1) {
			MPI_Recv(traceInfo,2,MPI_INT,myid+1,2,
					MPI_COMM_WORLD,&status);
			count = traceInfo[0];
			currentJ = traceInfo[1];
			MPI_Recv(res,count,MPI_CHAR,myid+1,3,
					MPI_COMM_WORLD,&status);
		} 
		traceback(listSubY[0],lineX,fTab,rSize,currentJ,&count,res);
		res[count] = '\0';

  		stop_timer(timer_1);
  		printf("Time Taken: %.2lfs\n", get_timer(timer_1));
		printf("Length = %d\n",count);

		for (i = count-1; i >=0; i--) 
			printf("%c",res[i]);
		printf("\n");
		
	} 
	else if (myid == num_procs-1) {
		count = 0;
		k = traceback(mySubY,lineX,fTab,rSize,xSize,&count,res);
		res[count]='\0';
		traceInfo[0] = count;
		traceInfo[1] = k;
  		MPI_Send(traceInfo,2,MPI_INT,myid-1,2,MPI_COMM_WORLD);
		MPI_Send(res,count,MPI_CHAR,myid-1,3,MPI_COMM_WORLD);
	} else {
		MPI_Recv(traceInfo,2,MPI_INT,myid+1,2,
				MPI_COMM_WORLD,&status);
		count = traceInfo[0];
		currentJ = traceInfo[1];
		MPI_Recv(res,count,MPI_CHAR,myid+1,3,
				MPI_COMM_WORLD,&status);

		k = traceback(mySubY,lineX,fTab,rSize,currentJ,&count,res);
		res[count]='\0';
		traceInfo[0] = count;
		traceInfo[1] = k;
  		MPI_Send(traceInfo,2,MPI_INT,myid-1,2,MPI_COMM_WORLD);
		MPI_Send(res,count,MPI_CHAR,myid-1,3,MPI_COMM_WORLD);
	}
	
	MPI_Finalize();
	return 0;
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

//traceback
int traceback(char *lineY, char *lineX, int **fTab, int i, int j, int *count, char *res) {

	while (i != 0 && j != 0) {
		if (lineX[j-1] == lineY[i-1]) {
			res[*count] = lineX[j-1];
			(*count)++;
			i--;
			j--;
		}
		else if (fTab[i-1][j] == fTab[i][j]) {
			i--;
		}
		else {
			j--;
		}
	}
	if (j == 0 && i > 0) 
		return -1;
	else if (i == 0 && j >= 0)
		return j;
}
