__kernel void block_encrypt (__global char *input, __global char *inp_key, __global char *output) {
	
	int id = get_global_id(0);
	
	int BSIZE = 256;
	
	int si = id * BSIZE;
	int ei = (id + 1) * BSIZE;
	
	char *key = (char *)malloc(sizeof(char) * BSIZE);
	
	unsigned long int ctr, t, base = 76895643;
	int ii = 0;
	ctr = (base + (id * 394670133)) % 4294967296;
	t = ctr;
	
	for (int ii = 0; ii < 32; ii++) {
		
		key[ii] = inp_key[ii];
		
		unsigned long int temp2 = ctr % 230 + 26;
		unsigned long n1 = ctr/256;
		
		n1 = ctr + (temp2 * 16777216);// 2^24
		ctr = n1;
		key[ii] = (key[ii] + (ctr % 230 + 26)) % 230 + 26;
		ctr /= 256;
		
		if(ctr == 0) {
			ctr = t;
		}
		
		int a[32];
		char temp[32];
		int i, j, k, temp1, size = 32;
		int b[32];
		
		for(i = 0; i < 32; i++) {
			b[i] = i;
		}
		
		j = 0;
		
		while (size > 0) {
			
			temp[i] = key[i] % 230 + 26;
			key[i] /= 256;
			
			k = temp1 % size;
			a[j++] = b[k];
			i = k;
			
			while (i < size) {
				b[i] = b[i+1];
				i += 1;
			}
			size -= 2;
		}
		
		i = 0;
		while (i < 32) {
			temp[i] = key[a[i]];
			i++;
		}
		
		i = 0;
		while (i < 32) {
			key[i] = temp[i];
			i++;
		}
	}
	int i = 0;
	while (i < 32) {
		char z = input[i];
		output[i] = z ^ key[i];
		i++;
	}
	
}

