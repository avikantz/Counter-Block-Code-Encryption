#include "mpi.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void shufflearray(int a[32], unsigned long key)
{
	int i, j, k, temp, size = 32;
	int b[32];
	for(i = 0; i < 32; i++)
		b[i] = i;
	j = 0;
	while(size > 0)
	{
		temp = key % 256;
		key /= 256;
		k = temp % size;
		a[j++] = b[k];
		for(i = k; i < size; i++)
		{
			b[i] = b[i+1];
		}
		size--;
	}
}

void transposition(char k[32], unsigned long key)
{
	int a[32];
	char temp[32];
	shufflearray(a,key);
	for (int i = 0; i < 32; i++)
		temp[i] = k[a[i]];
	for (int i = 0; i < 32; i++)
		k[i] = temp[i];
}

unsigned long int circular_shift(unsigned long int n)
{
	unsigned long int temp = n % 256;
	temp = temp >> 8; 
	temp += (temp * 16777216);// 2^24
	return temp;
}

void substitution(char k[32], unsigned long key)
{
	char z;
	long t = key;
	for (int i = 0; i < 32; i++)
	{
		key = circular_shift(key);
		z = k[i];
		k[i] = (z + (t % 230 + 26)) % 230 + 26;
		t /= 256;
		if(t == 0)
			t = key;
	}
}

void xor_op(char a[32], char k[32])
{
	int i, j;
	char temp[32], z;
	std::strcpy(temp,k);
	for (i = 0; i < 32; i++)
	{
		z = a[i];	
		a[i] = char((int(z) ^ int(temp[i])) % 256);		
	}	
}

int main (int a, char * b[]) 
{

	int rank, size;

	MPI_Init(&a, &b);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	char key[32], block[32], ch;
	int n, i;
	unsigned long ctr, temp2;
	char outdata[257];

	char indata[257] = "Lorem ipsum dolor sit er elit lamet, consectetaur cillium adipisicing pecu, sed do eiusmod tempor labore et dolore magna aliqua. sed do eiusmod tempor labore et dolore magna aliqua. eiusmod tempor labore et dolore magna aliqua. sed do eiusmod tempor labore";
	
	if (rank == 0) 
	{

		printf("\nPlain text:\n%s\n", indata);
		ctr = ((375623 * 48257) % 3294967296) + 1000000000;
		char temp1[50];
		
		strcpy(temp1, "kr3i4go9uowlj&^RIGS@(Njf9p8Y2EIT");
		for (i = 0; i < 32; i++)
			key[i] = temp1[i];
	}

	MPI_Bcast(key, 32, MPI_CHAR, 0, MPI_COMM_WORLD);
	MPI_Bcast(&ctr, 1, MPI_LONG, 0, MPI_COMM_WORLD);
	MPI_Scatter(indata, 32, MPI_CHAR, block, 32, MPI_CHAR, 0, MPI_COMM_WORLD);
	// MPI_Scatter(arr, 1, MPI_LONG, &rec, 1, MPI_LONG, 0, MPI_COMM_WORLD);
	
	ctr += ((rank * 245318257) % 3294967296) +1000000000;

	for(i = 0; i < 10; i++)
	{	
		substitution(key, ctr);
		temp2 = ctr;
		ctr += ((temp2 + (789789789)) % 3294967296) +1000000000;
		transposition(key, ctr);
	}//modified fiestel structure
	xor_op(block, key);

	MPI_Gather(block, 32, MPI_CHAR, outdata, 32, MPI_CHAR, 0, MPI_COMM_WORLD);

	if(rank==0)
	{
		outdata[256] = '\0';
		printf("\nEncrypted text:\n%s\n", outdata);
		// printf("\n----------------END----------------\n");
	}

	MPI_Finalize();
	return 0;
}