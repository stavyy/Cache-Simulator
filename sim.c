//Stavros Avdella
//3939968

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void arrays2D(int numofSets, int assoc);
void simulate_access(int numofSets, long long int address, char rw, int assoc, int replacement, int wb, float *writes, float *reads, int *hits, float *misses, int numLine);
int LRU(int set, int assoc);
int FIFO(int set, int assoc);


// Making these 2D arrays global to avoid confusion when sending through functions
long long int  **tag_array;
long long int **lru_position;
int **dirty;


int main(int argc, char **argv)
{
    FILE *input;
    FILE *output;

    // Variables being read from command line
    int cache_size = atoi(argv[1]);
    int assoc = atoi(argv[2]);
    int replacement = atoi(argv[3]);
    int wb = atoi(argv[4]);
    char inputFile[300];
    strcpy(inputFile, argv[5]);

    int hits = 0; // # of hits
    float misses = 0; // # of misses
    float reads = 0; // # of reads to mem
    float writes = 0; // # of writes to mem
    char addrString[20];
    long long int address; // Address read from file
    char rw; // Tells you whether its a read or a write
    int numLine = 0; // Used for the LRU
    float missRatio; // Used to calculate miss ratio

    int numofSets = (cache_size / (64 * assoc));

    // Open Files
    if((input = fopen(inputFile, "r")) == NULL)
    {
        fprintf(stderr, "Cannot open file!\n");
        return 0;
    }

    if((output = fopen("results.txt", "w")) == NULL)
    {
        fprintf(stderr, "Cannot create file!\n");
        return 0;
    }

    // Allocate space for 2D arrays using malloc and then initialize everything to zero with a for loop
    arrays2D(numofSets, assoc);

    // Read through file
    while(fscanf(input, "%c %s\n", &rw, addrString) != EOF)
    {
        numLine++;

        // This completes the number of  writes for a writethrough
        if(rw == 'W' && wb == 0)
            writes++;

        address = strtol(addrString, NULL, 16);

        simulate_access(numofSets, address, rw, assoc, replacement, wb, &writes, &reads, &hits, &misses, numLine);
    }

    // Print results to file
    missRatio = (misses/(hits + misses));

    fprintf(output, "%f\n", missRatio); // miss ratio
    fprintf(output, "%f\n", writes); // writes to mem
    fprintf(output, "%f\n", reads); // reads to mem

    // Close Files
    fclose(input);
    fclose(output);
}

void simulate_access(int numofSets, long long int address, char rw, int assoc, int replacement, int wb, float *writes, float *reads, int *hits, float *misses, int numLine)
{
    int i,j, index, empty = 1;
    int set = ((address / 64) % numofSets);
    long long int tag = address / 64;

    // Check through assoc for a hit, if nothing is found in set, miss
    for(i = 0; i < assoc; i++)
    {
        if(tag == tag_array[set][i])
        {
            *hits += 1;

            // Update LRU
            lru_position[set][i] = numLine;

            // Update dirty bit
            if(rw == 'W')
                dirty[set][i] = 1;

            return;
        }
    }

    // Otherwise, a miss because no match was found in the set
    *misses += 1;

    // Read to memory everytime there is a miss.
    *reads += 1;

// _______________________________READ/WRITE_________________________________

// Empty?
    for(j = 0; j < assoc && empty == 1; j++)
    {
        if(tag_array[set][j] == 0)
        {
            index = j;
            empty = 0;
        }
    }

    // Check LRU or FIFO
    if(empty == 1)
    {
        if(replacement == 0)
        {
            index = LRU(set, assoc);

        }
        else
            index = FIFO(set, assoc);
    }

    // Increment writes for a dirty bit
    if(dirty[set][index] == 1 && wb == 1)
        *writes += 1;

    // Update Tag
    tag_array[set][index] = tag;

    // Dirty bit
    if(rw == 'R')
        dirty[set][index] = 0;
    if(rw == 'W')
        dirty[set][index] = 1;

		// Update LRU
		lru_position[set][index] = numLine;

}

int FIFO(int set, int assoc)
{
	int tag;

	for(int j = 0; j < assoc - 1; j++)
	{
		tag_array[set][j] = tag_array[set][j+1];
		dirty[set][j] = dirty[set][j+1];
	}
	tag_array[set][assoc-1] = tag;

    return assoc - 1;
}

int LRU(int set, int assoc)
{
    int x, min, minIndex;

    min = lru_position[set][0];
    for(x = 0; x < assoc; x++)
    {
        if(lru_position[set][x] < min)
        {
            min = lru_position[set][x];
            minIndex = x;
        }
    }

    return minIndex;
}

void arrays2D(int numofSets, int assoc)
{
    int i, j;

    tag_array = (long long int **) malloc(numofSets * sizeof(long long int *));
    for(i = 0; i < numofSets; i++)
        tag_array[i] =  (long long int *) malloc(assoc * sizeof(long long int));

    lru_position = (long long int **) malloc(numofSets * sizeof(long long int *));
    for(i = 0; i < numofSets; i++)
        lru_position[i] = (long long int *) malloc(assoc * sizeof(long long int));

    dirty = (int **) malloc(numofSets * sizeof(int *));
    for(i = 0; i < numofSets; i++)
        dirty[i] = (int *) malloc(assoc * sizeof(int));

    // Anitialize to zero
    for(i = 0; i < numofSets; i++)
    {
        for(j = 0; j < assoc; j++)
        {
            tag_array[i][j] = 0;
            dirty[i][j] = 0;
        }
        for(j = 0; j < assoc; j++)
        {
            lru_position[i][j] = j;
        }
    }
}
