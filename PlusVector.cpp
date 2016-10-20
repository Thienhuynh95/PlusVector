#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define ARRAY_SIZE 10

using namespace std;

int convertToString(const char *filename, std::string& s)
{
	size_t size;
	char*  str;
	std::fstream f(filename, (std::fstream::in | std::fstream::binary));

	if (f.is_open())
	{
		size_t fileSize;
		f.seekg(0, std::fstream::end);
		size = fileSize = (size_t)f.tellg();
		f.seekg(0, std::fstream::beg);
		str = new char[size + 1];
		if (!str)
		{
			f.close();
			return 0;
		}

		f.read(str, fileSize);
		f.close();
		str[size] = '\0';
		s = str;
		delete[] str;
		return 0;
	}
	cout << "Error: failed to open file\n:" << filename << endl;
	return 1;
}

int main()
{
	cl_context context;
	cl_kernel kernel;
	cl_command_queue command_queue;
	cl_program program;
	cl_int err_code;
	cl_uint num_of_platforms = 0;
	cl_platform_id platform_id;
	cl_device_id device_id;
	cl_uint num_of_devices = 0;
	int status;


	cl_mem inputA, inputB, output;
	//tao doi tuong buffer cho cac thong tin work_item, work_group, so' work_group, so' dimension su dung
	cl_mem numidbuf, groupidbuf, localidbuf, dimenbuf;
	int arrayA[ARRAY_SIZE] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	int arrayB[ARRAY_SIZE] = { 3, 1, 2, 7, 4, 6, 3, 9, 3, 7 };
	int results[ARRAY_SIZE] = { 0 };
	cl_uint dimen[1];
	size_t numid[1];
	size_t groupid[ARRAY_SIZE];
	size_t localid[ARRAY_SIZE];


	if (clGetPlatformIDs(1, &platform_id, &num_of_platforms) != CL_SUCCESS) {
		printf("Failed to get platform id\n");
		return EXIT_FAILURE;
	}

	if (clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &num_of_devices) != CL_SUCCESS) {
		printf("Failed to get device id\n");
		return EXIT_FAILURE;
	}

	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err_code);

	command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &err_code);

	const char *filename = "OpenCLFile.cl";
	string sourceStr;
	status = convertToString(filename, sourceStr);
	const char *source = sourceStr.c_str();
	size_t sourceSize[] = { strlen(source) };
	program = clCreateProgramWithSource(context, 1, &source, sourceSize, &err_code);
	if (clBuildProgram(program, 0, NULL, NULL, NULL, NULL) != CL_SUCCESS) {
		printf("Error\n");
		return EXIT_FAILURE;
	}


	kernel = clCreateKernel(program, "addingVector", &err_code);


	inputA = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int)* ARRAY_SIZE, NULL, NULL);
	inputB = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int)* ARRAY_SIZE, NULL, NULL);
	output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int)* ARRAY_SIZE, NULL, NULL);


	numidbuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(size_t), NULL, NULL);
	dimenbuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_uint), NULL, NULL);
	groupidbuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(size_t)* ARRAY_SIZE, NULL, NULL);
	localidbuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(size_t)* ARRAY_SIZE, NULL, NULL);


	clEnqueueWriteBuffer(command_queue, inputA, CL_TRUE, 0, sizeof(int)* ARRAY_SIZE, arrayA, 0, NULL, NULL);
	clEnqueueWriteBuffer(command_queue, inputB, CL_TRUE, 0, sizeof(int)* ARRAY_SIZE, arrayB, 0, NULL, NULL);


	clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputA);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &inputB);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &output);
	clSetKernelArg(kernel, 3, sizeof(cl_mem), &numidbuf);
	clSetKernelArg(kernel, 4, sizeof(cl_mem), &groupidbuf);
	clSetKernelArg(kernel, 5, sizeof(cl_mem), &localidbuf);
	clSetKernelArg(kernel, 6, sizeof(cl_mem), &dimenbuf);



	size_t total_work_item = ARRAY_SIZE;
	size_t group = 5;

	clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &total_work_item, &group, 0, NULL, NULL);

	clFinish(command_queue);

	// Doc ket qua tu buffer qua result
	clEnqueueReadBuffer(command_queue, output, CL_TRUE, 0, sizeof(int)* ARRAY_SIZE, results, 0, NULL, NULL);
	// doc thong tin numgroup, group id, num group
	clEnqueueReadBuffer(command_queue, numidbuf, CL_TRUE, 0, sizeof(size_t), numid, 0, NULL, NULL);
	clEnqueueReadBuffer(command_queue, groupidbuf, CL_TRUE, 0, sizeof(size_t)* ARRAY_SIZE, groupid, 0, NULL, NULL);
	clEnqueueReadBuffer(command_queue, localidbuf, CL_TRUE, 0, sizeof(size_t)* ARRAY_SIZE, localid, 0, NULL, NULL);
	clEnqueueReadBuffer(command_queue, dimenbuf, CL_TRUE, 0, sizeof(cl_uint), dimen, 0, NULL, NULL);

	// In ket qua
	printf("Result:\n");
	cout << "Dang duoc chay voi " << numid[0] << " group tren " << dimen[0] << " dimension\n";
	for (int i = 0; i < ARRAY_SIZE; i++){
		cout << results[i] << " dang duoc thuc thi tai group " << groupid[i] << " tai work item " << localid[i];
		printf("\n");
	}
	printf("\n");

	clReleaseMemObject(output);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);

	return EXIT_SUCCESS;
}
