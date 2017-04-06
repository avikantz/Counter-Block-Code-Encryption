/**
 *	Counter Block Coding
 *
 *
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <OpenCL/opencl.h>

#define KEY "b82z58YF2f8oo0cKmyE4Te6NM6spuM21" // Static 256 bit (32B) key

#define BSIZE 256

#define DATA_SIZE (1024) // Max source size

void execute_parallel (char *inname) {
	
	int err;                            // error code returned from api calls
	
	size_t global_work_size;			// global domain size for our calculation
	size_t local_work_size;				// local domain size for our calculation
	
	cl_platform_id platform_id;			// compute platform id
	cl_device_id device_id;				// compute device id
	cl_context context;					// compute context
	cl_command_queue command_queue;		// compute command queue
	cl_program program;					// compute program
	cl_kernel kernel;					// compute kernel
	
	cl_mem mem_input;					// device memory used for the input array
	cl_mem mem_key;
	cl_mem mem_output;					// device memory used for the output array
	
	size_t i;
	
	// Handle input and output
	
	FILE *infile = fopen(inname, "r");
	
	fseek(infile, 0, SEEK_END);
	size_t infile_size = ftell(infile);
	fseek(infile, 0, SEEK_SET);
	
	size_t block_count = infile_size / BSIZE + 1;
	
	size_t mem_size = block_count * BSIZE * sizeof(char);
	size_t key_size = BSIZE * sizeof(char);
	
	//	printf("File size = %li, creating blocks of size = %li * %d\n", infile_size, block_count, BSIZE);
	
	char *input_data = (char *)malloc(mem_size);
	fread(input_data, sizeof(char), infile_size, infile);
	
	// Appened random characters to make it a multiple of BSIZE
	for (i = infile_size; i < block_count * BSIZE; ++i) {
		input_data[i] = '=';
	}
	
	//	printf("\n\nInput data: \n");
	//	for (i = 0; i < block_count * BSIZE; ++i) {
	//		printf("%c", input_data[i]);
	//	}
	//	printf("\n\n------------------\n\n");
	
	
	char *output_data = (char *)malloc(mem_size);
	
	
	
	// 1. Load the kernel code for the glory of the Sontaran Empire
	
	FILE *kernel_code_file = fopen("kernel.cl", "r");
	if (kernel_code_file == NULL) {
		printf("Kernel loading failed.");
		exit(1);
	}
	
	char *source_str = (char *)malloc(DATA_SIZE);
	size_t source_size = fread(source_str, 1, DATA_SIZE, kernel_code_file);
	source_str[source_size] = '\0'; // VERY IMPORTANT, cause random C stuff.
	
	fclose(kernel_code_file);
	
	
	
	// 2. Get platform and device information (Connect to a compute device)
	
	err = clGetPlatformIDs(1, &platform_id, NULL);
	err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
	
	if (err != CL_SUCCESS) {
		printf("Error: Failed to create a device group!\n");
		return EXIT_FAILURE;
	}
	
	
	
	// 3. Create an OpenCL context
	
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	
	if (!context) {
		printf("Error: Failed to create a compute context!\n");
		return EXIT_FAILURE;
	}
	
	
	
	// 4. Create a command queue
	
	command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE	, &err);
	
	if (!command_queue) {
		printf("Error: Failed to create a command commands!\n");
		return EXIT_FAILURE;
	}
	
	
	
	// 5. Create memory buffers on the device for the kernel args
	
	mem_input = clCreateBuffer(context, CL_MEM_READ_ONLY, mem_size, NULL, NULL);
	mem_key = clCreateBuffer(context, CL_MEM_READ_ONLY, key_size, NULL, NULL);
	mem_output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, mem_size, NULL, NULL);
	
	if (!mem_input || !mem_output) {
		printf("Error: Failed to allocate device memory!\n");
		exit(1);
	}
	
	
	
	// 6. Write the memory contents to the device memory
	
	err  = clEnqueueWriteBuffer(command_queue, mem_input, CL_TRUE, 0, mem_size, input_data, 0, NULL, NULL);
	err |= clEnqueueWriteBuffer(command_queue, mem_key, CL_TRUE, 0, key_size, KEY, 0, NULL, NULL);
	
	if (err != CL_SUCCESS) {
		printf("Error: Failed to write to source array!\n");
		exit(1);
	}
	
	
	
	// 7. Create a program from kernel source
	
	program = clCreateProgramWithSource(context, 1, (const char **)&source_str, NULL, &err);
	
	if (!program) {
		printf("Error: Failed to create compute program!\n");
		return EXIT_FAILURE;
	}
	
	
	
	// 8. Build the kernel program executable
	
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	
	if (err != CL_SUCCESS) {
		size_t len;
		char buffer[2048];
		printf("Error: Failed to build program executable!\n");
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
		printf("%s\n", buffer);
		exit(1);
	}
	
	
	
	// 9. Create the OpenCL kernel object in the program we wish to run
	
	kernel = clCreateKernel(program, "block_encrypt", &err);
	
	if (!kernel || err != CL_SUCCESS) {
		printf("Error: Failed to create compute kernel!\n");
		exit(1);
	}
	
	
	
	// 10. Set the arguments to our compute kernel
	
	err  = 0;
	err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem_input);
	err  = clSetKernelArg(kernel, 1, sizeof(cl_mem), &mem_key);
	err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &mem_output);
	
	if (err != CL_SUCCESS) {
		printf("Error: Failed to set kernel arguments! %d\n", err);
		exit(1);
	}
	
	
	// 11. Execute the kernel on device
	
	global_work_size = block_count;
	local_work_size = 1;
	
	//	printf("Global work size = %li, Local work size = %li\n", global_work_size, local_work_size);
	
	cl_event event;
	
	err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, &event);
	
	if (err) {
		printf("Error: Failed to execute kernel!\n");
		return EXIT_FAILURE;
	}
	
	clFinish(command_queue); // Wait for the command commands to get serviced before reading back results
	
	cl_ulong time_start, time_end;
	double total_time;
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
	total_time = (double)time_end - time_start;
	
	// 12. Read the memory buffer from the device memory and copy to local memory
	
	err = clEnqueueReadBuffer(command_queue, mem_output, CL_TRUE, 0, mem_size, output_data, 0, NULL, NULL );
	
	if (err != CL_SUCCESS) {
		printf("Error: Failed to read output array! %d\n", err);
		exit(1);
	}
	
	// Limit the output file size and write to outfile
	//	output_data[infile_size] = '\0';
	
	//	FILE *outfile = fopen("output.txt", "r+");
	//	printf("\n\n----------------\nOutput data: \n");
	//	for (i = 0; i < block_count * BSIZE; ++i) {
	//		printf("%c", output_data[i]);
	//		fputc(input_data[i], outfile);
	//	}
	//	printf("\n\n------------------\n\n");
	
	cl_ulong resolution = 40;
//	clGetDeviceInfo(1, CL_DEVICE_PROFILING_TIMER_RESOLUTION, sizeof(cl_ulong), &resolution, NULL);
	
	printf("%lf\n", total_time * resolution / NSEC_PER_SEC);
	
	// 13. Clean shit up
	
	clReleaseMemObject(mem_input);
	clReleaseMemObject(mem_output);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);
}

int ni, nj, ftype;

enum file_types{TXT, MP4, PNG, JPEG, BIN};

void readblock(char a[8][4], int block_num, FILE *fp) {
	int flag = 0;
	fseek(fp, block_num * 32, SEEK_SET);
	//cout << "\n";
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if(flag == 0)
			{
				if(getc(fp) == EOF)
				{
					flag = 1;
					ni = i;
					nj = j;
				}
				else
					a[i][j] = getc(fp);
			}
			else
				a[i][j] = 142;
		}
	}
}

void writeblock(char a[8][4], int block_num, FILE *fp, int num_of_blocks) {
	fseek(fp, block_num * 32, SEEK_SET);
	int i, j;
	if(block_num == (num_of_blocks - 1))
	{
		for (i = 0; i < ni; i++)
			for (j = 0; j < 4; j++)
				putc(a[i][j], fp);
		i = ni;
		for (j = 0; j <= nj; j++)
			putc(a[i][j], fp);
	}
	else
	{
		for (i = 0; i < 8; i++)
			for (j = 0; j < 4; j++)
				putc(a[i][j], fp);
	}
}

void clear_block(char a[8][4]) {
	for(int i = 0; i < 8; i++)
		for(int j = 0; j < 4; j++)
			a[i][j] = 0;
}

long circular_shift(unsigned long n) {
	int temp = n % 256;
	long nn = n / 256;
	nn = n + (temp * 4194304);
	return nn;
}

void row_shift(char a[8][4]) {
	char temp;
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
		{
			temp = a[i][j];
			a[i][j] = a[4 + i][j];
			a[4 + i][j] = temp;
		}
}

void substitution(char k[8][4], unsigned long int key) {
	long int t = key;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			k[i][j] = (k[i][j] + (t % 256)) % 256;
			t /= 256;
		}
		circular_shift(key);
		t = key;
	}
}

void key_encrypt(char k[8][4], unsigned long int key) {
	key = circular_shift(key);
	substitution(k, key);
	row_shift(k);
}

void xor_op(char a[8][4], char k[8][4], char ch) {
	int i, j;
	if((ch == 'D')||(ch == 'd'))
		for (i = 0; i < 8; i++)
			for (j = 0; j < 4; j++)
				k[i][j] = (255 ^ k[i][j]) % 256;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 4; j++) {
			a[i][j] = (a[i][j] ^ k[i][j]) % 256;
		}
	}
}

void execute_sequential(char *inname) {
	int num_of_blocks, flen, i, j;
	unsigned long int ctr, base = 76895643;
	char choice, temp[40], key[8][4], block[8][4];
	
	clock_t start_time, end_time;
	double total_time;
	
	start_time = clock();
	
	//cin >> temp;
	strcpy(temp, "thisisa32bytekeywithsomextrachar");
	for(i = 0; i < 8; i++)
		for(j = 0; j < 4; j++)
			key[i][j] = temp[(i * 8) + j];
	
	FILE *fp1, *fp2;
	fp1 = fopen(inname, "r");
	fp2 = fopen("text2.txt", "w+");
	
	fseek(fp1, 0, SEEK_END);
	flen = ftell(fp1);
	fseek(fp1, 0, SEEK_SET);
	if(flen % 32 != 0)
		num_of_blocks = (flen / 32) + 1;
	else
		num_of_blocks = (flen / 32);
	
	for(i = 0; i < num_of_blocks; i++) {
		readblock(block, i, fp1);
		ctr = (base + (i * 394670133)) % 2147483648;
		key_encrypt(key, ctr);
		xor_op(block, key, choice);
		writeblock(block, i, fp2, num_of_blocks);
		clear_block(block);
	}
	fclose(fp1);
	fclose(fp2);
	
	end_time = clock();
	total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
	
	printf("%lf, ", total_time);
	
	return 0;
}

int main (int argc, char** argv) {
	
	char inname[32];
	char *names[32] = {"info", "info2", "info3", "info4", "info5", "info6", "info7", "info8", "info9", "info10" };
	for (int i = 0; i < 10; ++i) {
		strcpy(inname, names[i]);
		execute_sequential(inname);
		execute_parallel(inname);
	}

	return 0;
	
}

