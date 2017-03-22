#include <iostream>
#include <cstring>
#include <fstream>
using namespace std;

int ni, nj, ftype;

enum file_types{TXT, PNG, JPG, BIN};

void open_read_file(fstream &fp)
{
	char fname[50];
	cout << "\nEnter the filename to be encrypted: ";
	cin >> fname;
	//std::strcpy(fname,"read.txt");

	fp.open(fname);
}//working

void open_write_file(fstream &fp)
{
	char fname[50];
	cout << "\nEnter the name of the new encrypted file: ";
	cin >> fname;
	//std::strcpy(fname,"write.txt");

	fp.open(fname, ios::out);
}//working

void readblock(char a[32], int block_num, fstream &fp)
{
	int flag = 0;
	ni = 32;
	fp.seekg((block_num * 32), fp.beg);
	for (int i = 0; i < 32; i++)
	{	
		if(flag == 0)
		{
			if(fp.eof())
			{
				flag = 1;
				ni = i;
			}
			fp.get(a[i]);
		}
		else
			a[i] = 142;	
	}		
}

void writeblock(char a[32], int block_num, fstream &fp, int flag)
{
	fp.seekg((block_num * 32), fp.beg);
	int i, j;
	if(flag == 0)
	{
		for (i = 0; i < 32; i++)
			fp.put(a[i]);
	}
	else
	{
		for (i = 0; i < (ni - 1); i++)
			fp.put(a[i]);
	}
}

void clear_block(char a[32])
{
	for(int i = 0; i < 8; i++)
		a[i] = 0;
}

void shufflearray(int a[32], unsigned long int key)
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

void transposition(char k[32], unsigned long int key)
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
	n >> 8; 
	n += (temp * 16777216);// 2^24
	return n;
}

void substitution(char k[32], unsigned long int &key)
{
	long int t = key;
	for (int i = 0; i < 32; i++)
	{
		key = circular_shift(key);
		k[i] = int(k[i] + char(t % 256)) % 256;
		t /= 256;
		if(t == 0)
			t = key;
	}
}

void key_encrypt(char k[32], unsigned long int key)
{

	substitution(k, key);
	key = (key + (7 * 394670133)) % 4294967296;
	transposition(k, key);
}

void xor_op(char a[32], char k[32])
{
	int i, j;
	char temp[32];
	std::strcpy(temp,k);
	for (i = 0; i < 32; i++)
		a[i] = char((int(a[i]) ^ int(temp[i])) % 256);		
}

int main()
{
	int num_of_blocks, flen, i, j;
	unsigned long int ctr, base = 76895643;
	char key[32], block[32];
	char temp[33] = "thisisa32bytekeywithsomextrachar";
	
	cout << "\nEnter the 32 character key: ";
	//cin >> temp;
	for(i = 0; i < 32; i++)
			key[i] = temp[i];

	fstream fp1, fp2; 
	open_read_file(fp1);	
	open_write_file(fp2);

	fp1.seekg(0, fp1.end);
	flen = fp1.tellg();
	fp1.seekg(0, fp1.beg);
	num_of_blocks = (flen / 32);

	//cout << num_of_blocks;
	for(i = 0; i < num_of_blocks; i++)
	{
		readblock(block, i, fp1);
		ctr = (base + (i * 394670133)) % 4294967296;
		key_encrypt(key, ctr);
		xor_op(block, key);
		writeblock(block, i, fp2, 0);
		clear_block(block);
	}
	
	readblock(block, i, fp1);
	ctr = (base + (i * 394670133)) % 4294967296;
	key_encrypt(key, ctr);
	xor_op(block, key);
	writeblock(block, i, fp2, 1);
	
	fp1.close();
	fp2.close();
	cout << "\n\n------------END------------\n\n";
	return 0;
}