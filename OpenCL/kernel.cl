__kernel void block_encrypt (__global char *input, __global char *inp_key, __global char *output) {
	
	int id = get_global_id(0);
	
	int BSIZE = 256; // Block size here
	
	// Start and end indices
	int si = id * BSIZE;
	int ei = (id + 1) * BSIZE;
	
	// Get the new key
	// inp_key is 32B long, key is 256B long
	char *key = (char *)malloc(sizeof(char) * BSIZE);
	for (int i = 0; i < BSIZE; ++i) {
//		char z = inp_key[i % 32];
//		if (z >= 'A' && z <= 'Z') {
//			key[i] = (z + id)
//		}
//		Potentially unsecure, might have '\0' in the string
//		key[i] = (z + id) % 128;
		
//		Just rotate the key bits instead?
		key[i] = inp_key[(i + id) % 32];
	}
	
//	printf("\ninp_key = %s, key = %s", inp_key, key);
	
	int k = 0;
	for (int i = si; i < ei; ++i) {
		char z = input[i];
		output[i] = z ^ key[k++];
	}
}
