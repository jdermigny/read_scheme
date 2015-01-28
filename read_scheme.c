#include<stdio.h>
#include<stdlib.h>

#define MAX_CHAR_LENGTH 128
#define MAX_TRANSITIONS 128

#define TRUE 1
#define FALSE 0

typedef enum {
	BEGIN, READLEVEL, READTRANS
}		read_state;

struct TRANSITION {
	double		Gamma;
	double		BR;
	double		BR_err;
	struct LEVEL   *finalLevel;
};


struct LEVEL {
	double		Energy;
	struct TRANSITION TranArray[MAX_TRANSITIONS];
	struct TRANSITION *currentTran;
};

struct LEVEL   *
levelLookup(double levelEnergy, struct LEVEL *firstLevel, size_t numLevels)
{
	for (size_t index = 0; index < numLevels; index++) {
		if ((firstLevel + index)->Energy == levelEnergy) {
			return (firstLevel + index);
		}
	}
	return NULL;
}


struct LEVEL   *
processTransition(char *bufferstring, struct LEVEL *initialLevel, struct LEVEL *firstLevel, size_t numLevel)
{
	double		GammaEnergy;
	double		LevelEnergy;
	double		BR;
	double		BR_err;

	if (sscanf(bufferstring, "%lf %lf %lf", &LevelEnergy, &BR,&BR_err) != 3) {
		printf("error in transition processing...\n");
		return NULL;
	}
	struct LEVEL   *finalLevel;
	finalLevel = levelLookup(LevelEnergy, firstLevel, numLevel);
	if (finalLevel == NULL){
		return NULL;
	}



	GammaEnergy = (initialLevel->Energy - finalLevel->Energy);

	(initialLevel->currentTran)->Gamma = GammaEnergy;
	(initialLevel->currentTran)->finalLevel = finalLevel;
	(initialLevel->currentTran)->BR = BR;
	(initialLevel->currentTran)->BR_err = BR_err;

	(initialLevel->currentTran)++;
	return finalLevel;


}
struct LEVEL   *
processLevel(char *bufferstring, struct LEVEL *decaylevel)
{
	char		garbage   [MAX_CHAR_LENGTH];
	double		energy;

	if (sscanf(bufferstring, "%s %lf", garbage, &energy) == 0) {
		printf("string mismatch in procesLevel\n");
		return NULL;
	} else {
		printf("LEVEL %f ADDED \n", energy);
		decaylevel->Energy = energy;
		decaylevel->currentTran = &(decaylevel->TranArray[0]);
		return decaylevel;
	}

}


int
main(int argc, char** argv)
{

	if(argc!=2){
		printf("incorrect number of arguments.:\ncorrect syntax: ./read_scheme myscheme.dat\n");
		return(EXIT_FAILURE);
	}


	FILE           *decayinput;
	decayinput = fopen(argv[1], "r");
	if(decayinput==NULL){
		printf("filename does not exist!\n");
		return(EXIT_FAILURE);
	}

	FILE *output;
	char tempfilename[128];
	printf("enter the filename for output:\n");
	scanf("%s",tempfilename);	
	output = fopen(tempfilename,"w");

	char		BUFFER    [MAX_CHAR_LENGTH];
	struct LEVEL	decaylevel[100];;
	struct LEVEL   *decayPtr = &decaylevel[0];
	int		numLevels = 0;
	struct LEVEL   *tmpPtr;
	int error_flag = FALSE;
	//read in line
		while (fgets(BUFFER, MAX_CHAR_LENGTH - 1, decayinput) != NULL) {
		if (BUFFER[0] == 'L')
			if (processLevel(&BUFFER[0], decayPtr) != NULL) {
				numLevels++;
				decayPtr++;
			}
	}

	rewind(decayinput);

	decayPtr = &decaylevel[0];
	unsigned int	ptrIndex = 0;
	read_state	status = BEGIN;
	//read in lines again
		while (fgets(BUFFER, MAX_CHAR_LENGTH - 1, decayinput) != NULL) {
		if (BUFFER[0] == 'L') {
			if (status == BEGIN)
				status = READTRANS;
			else
				decayPtr++;
			printf("Processing transitions for level: E=%f keV\n", (decayPtr)->Energy);
		} else if ((BUFFER[0] != '\n')) {
			tmpPtr = processTransition(&BUFFER[0], decayPtr, &decaylevel[0], numLevels);
			if (tmpPtr != NULL)
				printf("\tLEVEL LOOKUP SUCCEEDED, E=%f\n", tmpPtr->Energy);
			else{
				error_flag = TRUE;
				printf("#########################################\n");
				printf("LEVEL-LOOKUP FAILED ON STRING: %s!",&BUFFER[0] );
				printf("#########################################\n");
				printf("\nMake sure all transition descriptions match levels!\n\tie. LEVEL 4221.1 != 4221.2\n\n");
				fclose(decayinput);
				return(EXIT_FAILURE);
				}
		}
	}

	fclose(decayinput);



	struct TRANSITION *tmp_tran;
	struct LEVEL   *groundstate;
	struct LEVEL   *roofstate;
	groundstate = &decaylevel[0];
	roofstate = &decaylevel[numLevels - 1];
	decayPtr = groundstate;;

	/************************************************************************************
	 *	This section of the code prints an output usable by R Longland's LenaSUM code
	 *	as well as the geant4 LENA simulation.
	 *************************************************************************************/
	fprintf(output,"N-Levels\n");
	fprintf(output,"%d\n\n", numLevels);

	fprintf(output,"Energy-Levels\n");
	while (decayPtr <= roofstate) {
		if(decayPtr==roofstate){
		fprintf(output,"%.2f\t%.2f\t%.2f\n", decayPtr->Energy, 1.0, 0.0);
		decayPtr++;
		continue;
		}
		fprintf(output,"%.2f\t%.2f\t%.2f\n", decayPtr->Energy, 0.0, 0.0);
		decayPtr++;
	}

	decayPtr = groundstate;
	fprintf(output,"\nB-Values\n");
	while ((decayPtr <= roofstate)) {
		tmp_tran = (decayPtr->currentTran - 1);
		while ((tmp_tran - &(decayPtr->TranArray[0])) >= 0) {
			fprintf(output,"%ld\t%ld\t%.4f\t%.4f\n", decayPtr - groundstate, tmp_tran->finalLevel - groundstate, (tmp_tran->BR)*.01,  (tmp_tran->BR_err)*.01);
			tmp_tran--;
		}
		decayPtr++;
	}
fclose(output);
printf("\nresults written to %s.\n\thave a nice day!\n",tempfilename);
return(EXIT_SUCCESS);
	
}
