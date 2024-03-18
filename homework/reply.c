#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>


int main(int argc, char **argv)
{

	int rank;
	int nProcesses;
    int rows;
    int columns;
    int numSteps = atoi(argv[3]);
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
	
    
    int *allNums;
    if(rank == 0){
        FILE* file = fopen(argv[1], "r");
        int **matrix;
        fscanf(file,"%d",&rows);
        fscanf(file,"%d",&columns);
        matrix = (int**)malloc(rows*sizeof(int*));
        allNums = (int *)malloc(columns * rows *sizeof(int));
        for(int i=0;i<rows;i++)
            for(int j=0;j<columns;j++)
                fscanf(file,"%d",&allNums[i*columns+j]);
            
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&rows,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&columns,1,MPI_INT,0,MPI_COMM_WORLD);
    int sizeOfVector = rows / nProcesses;
    int remainder = rows % nProcesses;
    int extra = sizeOfVector+1;
    
    int *counts = (int *)malloc(nProcesses * sizeof(int));
    int *displs = (int *)malloc(nProcesses * sizeof(int));
    int *displs2 = (int *)malloc(nProcesses * sizeof(int));
    int *counts2 = (int *)malloc(nProcesses * sizeof(int));

    for (int i = 0; i < nProcesses; i++) {
        if (i < remainder){
            counts[i] = extra*columns;
            counts2[i] = extra*columns;
        }
        else{
            counts[i] = sizeOfVector*columns;
            counts2[i] = sizeOfVector * columns;
        }
        if(i ==0)
        {
            displs[i] = 0;
            displs2[i] = 0;
        }
        else{
            displs[i] = displs[i - 1] + counts[i - 1];
            displs2[i] = displs2[i-1] + counts2[i-1];
        }
    }
    for(int i=1;i<nProcesses;i++)
        displs[i] -= columns;
    int localN;
    if(rank ==0  || rank== nProcesses-1)
        localN = counts[rank] + columns;
    else localN = counts[rank] + 2*columns;

    if(nProcesses != 1){
        for(int i=0;i<nProcesses;i++){
            if(i == 0 || i == nProcesses-1)
                counts[i] = counts[i] + columns;
            else counts[i] = counts[i] + 2 * columns;
        }
    }

    if(nProcesses == 1)
        localN -= columns;
    int * localNums;
    
    
    localNums = (int *) malloc(localN* sizeof(int));
    
    int *localNumsReply = (int *)malloc(localN*sizeof(int));
    MPI_Scatterv(allNums,counts,displs,MPI_INT,localNums,localN,MPI_INT,0,MPI_COMM_WORLD);
    for(int l = 0; l < numSteps; l++) {
        if (rank >0 && rank < nProcesses-1){
            for(int i=columns;i<localN-columns;i++){
                int counterOnes=0;
                if(i%columns < columns-1 && i%columns !=0){
                    if(localNums[i+1] == 1)
                        counterOnes++;
                    if(localNums[i+1+columns] ==1)
                        counterOnes++;
                    if(localNums[i+columns] == 1)
                        counterOnes++;
                    if(localNums[i-1] == 1)
                        counterOnes++;
                    if(localNums[i-1+columns] == 1)
                        counterOnes++;
                    if(localNums[i+1-columns] ==1)
                        counterOnes++;
                    if(localNums[i-columns] == 1)
                        counterOnes++;
                    if(localNums[i-1-columns] == 1)
                        counterOnes++;

                    if((counterOnes==3 || counterOnes == 6) && localNums[i]==0)
                        localNumsReply[i]=1;
                    
                    else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                        localNumsReply[i]=1;
                    else
                        localNumsReply[i]=0;
                }
                else if(i%columns == 0){
                    if(localNums[i+1] == 1)
                            counterOnes++;
                    if(localNums[i+1+columns] ==1)
                        counterOnes++;
                    if(localNums[i+columns] == 1)
                        counterOnes++;
                    if(localNums[i-columns] == 1)
                        counterOnes++;
                    if(localNums[i+1-columns] ==1)
                        counterOnes++;
                    if(counterOnes==3 && localNums[i]==0)
                        localNumsReply[i]=1;
                    else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                        localNumsReply[i]=1;
                    else
                        localNumsReply[i]=0;
                }
                else {
                    if(localNums[i-1] == 1)
                        counterOnes++;
                    if(localNums[i-1+columns] ==1)
                        counterOnes++;
                    if(localNums[i+columns] == 1)
                        counterOnes++;
                    if(localNums[i-1-columns] ==1)
                        counterOnes++;
                    if(localNums[i-columns] == 1)
                        counterOnes++;
                    
                    if(counterOnes==3 && localNums[i]==0)
                        localNumsReply[i]=1;
                    else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                        localNumsReply[i]=1;
                    else
                        localNumsReply[i]=0;
                }
            }
        }
        else if(rank == 0 && nProcesses !=1){
            for(int i=0;i<localN-columns;i++){
                int counterOnes = 0;
                if(i>=0 && i<columns) {
                    if(i<columns-1 && i!=0){
                        if(localNums[i+1] == 1)
                            counterOnes++;
                        if(localNums[i+1+columns] ==1)
                            counterOnes++;
                        if(localNums[i+columns] == 1)
                            counterOnes++;
                        if(localNums[i-1] == 1)
                            counterOnes++;
                        if(localNums[i-1+columns] == 1)
                            counterOnes++;

                        if(counterOnes==3 && localNums[i]==0)
                            localNumsReply[i]=1;
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else
                            localNumsReply[i]=0;
                    }
                    else if(i == 0){
                        if(localNums[i+1] == 1)
                            counterOnes++;
                        if(localNums[i+1+columns] ==1)
                            counterOnes++;
                        if(localNums[i+columns] == 1)
                            counterOnes++;
                        
                        if(counterOnes==3 && localNums[i]==0)
                            localNumsReply[i]=1;
                        
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else
                            localNumsReply[i]=0;
                    }
                    else {
                        if(localNums[i-1] == 1)
                            counterOnes++;
                        if(localNums[i-1+columns] ==1)
                            counterOnes++;
                        if(localNums[i+columns] == 1)
                            counterOnes++;
                        
                        if(counterOnes==3 && localNums[i]==0)
                            localNumsReply[i]=1;
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else {
                            localNumsReply[i]=0;
                        }
                    }

                }
                else {
                    if(i%columns<columns-1 && i%columns !=0){
                        if(localNums[i+1] == 1)
                            counterOnes++;
                        if(localNums[i+1+columns] ==1)
                            counterOnes++;
                        if(localNums[i+columns] == 1)
                            counterOnes++;
                        if(localNums[i-1] == 1)
                            counterOnes++;
                        if(localNums[i-1+columns] == 1)
                            counterOnes++;
                        if(localNums[i+1-columns] ==1)
                            counterOnes++;
                        if(localNums[i-columns] == 1)
                            counterOnes++;
                        if(localNums[i-1-columns] == 1)
                            counterOnes++;

                        if((counterOnes==3 || counterOnes == 6) && localNums[i]==0)
                            localNumsReply[i]=1;
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else
                            localNumsReply[i]=0;
                    }
                    else if(i % columns == 0){
                        if(localNums[i+1] == 1)
                            counterOnes++;
                        if(localNums[i+1+columns] ==1)
                            counterOnes++;
                        if(localNums[i+columns] == 1)
                            counterOnes++;
                        if(localNums[i-columns] == 1)
                            counterOnes++;
                        if(localNums[i+1-columns] ==1)
                            counterOnes++;
                        if(counterOnes==3 && localNums[i]==0)
                            localNumsReply[i]=1;
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else
                            localNumsReply[i]=0;
                    }
                    else {
                        if(localNums[i-1] == 1)
                            counterOnes++;
                        if(localNums[i-1+columns] ==1)
                            counterOnes++;
                        if(localNums[i+columns] == 1)
                            counterOnes++;
                        if(localNums[i-1-columns] ==1)
                            counterOnes++;
                        if(localNums[i-columns] == 1)
                            counterOnes++;
                        
                        if(counterOnes==3 && localNums[i]==0)
                            localNumsReply[i]=1;
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else
                            localNumsReply[i]=0;
                    }
                }
        }
        }
        else if(rank == 0 && nProcesses == 1){
            for(int i=0;i<localN;i++){
                int counterOnes = 0;
                if(i>=0 && i<columns) {
                    if(i<columns-1 && i!=0){
                        if(localNums[i+1] == 1)
                            counterOnes++;
                        if(localNums[i+1+columns] ==1)
                            counterOnes++;
                        if(localNums[i+columns] == 1)
                            counterOnes++;
                        if(localNums[i-1] == 1)
                            counterOnes++;
                        if(localNums[i-1+columns] == 1)
                            counterOnes++;

                        if(counterOnes==3 && localNums[i]==0)
                            localNumsReply[i]=1;
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else
                            localNumsReply[i]=0;
                    }
                    else if(i == 0){
                        if(localNums[i+1] == 1)
                            counterOnes++;
                        if(localNums[i+1+columns] ==1)
                            counterOnes++;
                        if(localNums[i+columns] == 1)
                            counterOnes++;
                        
                        if(counterOnes==3 && localNums[i]==0)
                            localNumsReply[i]=1;
                        
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else
                            localNumsReply[i]=0;
                    }
                    else {
                        if(localNums[i-1] == 1)
                            counterOnes++;
                        if(localNums[i-1+columns] ==1)
                            counterOnes++;
                        if(localNums[i+columns] == 1)
                            counterOnes++;
                        
                        if(counterOnes==3 && localNums[i]==0)
                            localNumsReply[i]=1;
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else {
                            localNumsReply[i]=0;
                        }
                    }

                }
                else {
                    if(i%columns<columns-1 && i%columns !=0){
                        if(localNums[i+1] == 1)
                            counterOnes++;
                        if(localNums[i+1+columns] ==1)
                            counterOnes++;
                        if(localNums[i+columns] == 1)
                            counterOnes++;
                        if(localNums[i-1] == 1)
                            counterOnes++;
                        if(localNums[i-1+columns] == 1)
                            counterOnes++;
                        if(localNums[i+1-columns] ==1)
                            counterOnes++;
                        if(localNums[i-columns] == 1)
                            counterOnes++;
                        if(localNums[i-1-columns] == 1)
                            counterOnes++;

                        if((counterOnes==3 || counterOnes == 6) && localNums[i]==0)
                            localNumsReply[i]=1;
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else
                            localNumsReply[i]=0;
                    }
                    else if(i % columns == 0){
                        if(localNums[i+1] == 1)
                            counterOnes++;
                        if(localNums[i+1+columns] ==1)
                            counterOnes++;
                        if(localNums[i+columns] == 1)
                            counterOnes++;
                        if(localNums[i-columns] == 1)
                            counterOnes++;
                        if(localNums[i+1-columns] ==1)
                            counterOnes++;
                        if(counterOnes==3 && localNums[i]==0)
                            localNumsReply[i]=1;
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else
                            localNumsReply[i]=0;
                    }
                    else {
                        if(localNums[i-1] == 1)
                            counterOnes++;
                        if(localNums[i-1+columns] ==1)
                            counterOnes++;
                        if(localNums[i+columns] == 1)
                            counterOnes++;
                        if(localNums[i-1-columns] ==1)
                            counterOnes++;
                        if(localNums[i-columns] == 1)
                            counterOnes++;
                        
                        if(counterOnes==3 && localNums[i]==0)
                            localNumsReply[i]=1;
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else
                            localNumsReply[i]=0;
                    }
                }
        }
        }
        else {
            for(int i=columns;i<localN;i++){
                int counterOnes = 0;
                if(i>localN-columns-1){
                    if( i < localN -1 && i%columns !=0 ) {
                        if(localNums[i+1] == 1)
                            counterOnes++;
                        if(localNums[i+1-columns] ==1)
                            counterOnes++;
                        if(localNums[i-columns] == 1)
                            counterOnes++;
                        if(localNums[i-1] == 1)
                            counterOnes++;
                        if(localNums[i-1-columns] == 1)
                            counterOnes++;

                        if(counterOnes==3 && localNums[i]==0){
                            localNumsReply[i]=1;
                        }
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else
                            localNumsReply[i]=0;
                    }
                    else if(i % columns == 0){
                        if(localNums[i+1] == 1)
                            counterOnes++;
                        if(localNums[i+1-columns] ==1)
                            counterOnes++;
                        if(localNums[i-columns] == 1)
                            counterOnes++;
                        
                        if(counterOnes==3 && localNums[i]==0)
                            localNumsReply[i]=1;
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else
                            localNumsReply[i]=0;
                    }
                    
                    else{
                        if(localNums[i-1] == 1)
                            counterOnes++;
                        if(localNums[i-1-columns] ==1)
                            counterOnes++;
                        if(localNums[i-columns] == 1)
                            counterOnes++;
                        
                        if(counterOnes==3 && localNums[i]==0){
                            localNumsReply[i]=1;
                        }
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else {
                            localNumsReply[i]=0;
                        }
                    }
                }
                else {
                    if(i%columns<columns-1 && i%columns !=0){
                        if(localNums[i+1] == 1)
                            counterOnes++;
                        if(localNums[i+1+columns] ==1)
                            counterOnes++;
                        if(localNums[i+columns] == 1)
                            counterOnes++;
                        if(localNums[i-1] == 1)
                            counterOnes++;
                        if(localNums[i-1+columns] == 1)
                            counterOnes++;
                        if(localNums[i+1-columns] ==1)
                            counterOnes++;
                        if(localNums[i-columns] == 1)
                            counterOnes++;
                        if(localNums[i-1-columns] == 1)
                            counterOnes++;

                        if((counterOnes==3 || counterOnes == 6)&& localNums[i]==0)
                            localNumsReply[i]=1;
                         else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else
                            localNumsReply[i]=0;
                    }
                    else if(i % columns == 0){
                        if(localNums[i+1] == 1)
                            counterOnes++;
                        if(localNums[i+1+columns] ==1)
                            counterOnes++;
                        if(localNums[i+columns] == 1)
                            counterOnes++;
                        if(localNums[i-columns] == 1)
                            counterOnes++;
                        if(localNums[i+1-columns] ==1)
                            counterOnes++;
                        if(counterOnes==3 && localNums[i]==0)
                            localNumsReply[i]=1;
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else
                            localNumsReply[i]=0;
                    }
                    else {
                        if(localNums[i-1] == 1)
                            counterOnes++;
                        if(localNums[i-1+columns] ==1)
                            counterOnes++;
                        if(localNums[i+columns] == 1)
                            counterOnes++;
                        if(localNums[i-1-columns] ==1)
                            counterOnes++;
                        if(localNums[i-columns] == 1)
                            counterOnes++;
                        
                        if(counterOnes==3 && localNums[i]==0)
                            localNumsReply[i]=1;
                        else if(localNums[i] == 1 && (counterOnes == 2 || counterOnes == 3 ) )
                            localNumsReply[i]=1;
                        else
                            localNumsReply[i]=0;
                    }
                }
            }
        }
        
        if(rank > 0 && rank < nProcesses-1)
        MPI_Gatherv(localNumsReply+columns, localN-2*columns, MPI_INT, allNums, counts2, displs2, MPI_INT, 0, MPI_COMM_WORLD);
        else if(rank == 0 && nProcesses !=1)
        MPI_Gatherv(localNumsReply, localN-columns, MPI_INT, allNums, counts2, displs2, MPI_INT, 0, MPI_COMM_WORLD);
        else if(rank == 0 && nProcesses == 1)
        MPI_Gatherv(localNumsReply, localN, MPI_INT, allNums, counts2, displs2, MPI_INT, 0, MPI_COMM_WORLD);
        else
        MPI_Gatherv(localNumsReply + columns, localN - columns, MPI_INT, allNums, counts2, displs2, MPI_INT, 0, MPI_COMM_WORLD);
        
        if(l != numSteps-1)
            MPI_Scatterv(allNums,counts,displs,MPI_INT,localNums,localN,MPI_INT,0,MPI_COMM_WORLD);
        
    }
    
    if(rank == 0){
        FILE * f = fopen(argv[2],"w");
        fprintf(f,"%d %d\n",rows,columns);
        for(int i=0;i<columns*rows;i++){
            if(i%columns == 0 && i!=0)fprintf(f,"\n");
            fprintf(f,"%d ",allNums[i]);
        }
    fprintf(f,"\n");
    }
	MPI_Finalize();
    return 0;

}