#pragma OPENCL EXTENSION cl_nvidia_printf: enable
// TODO: Add OpenCL kernel code here.
void kernel addingVector(global const int* A, global const int* B, global int* C, global size_t *numid, global size_t *groupid, global size_t *localid, global uint *dimension){
	int i = 0;
	size_t global_id = get_global_id(i);
	C[global_id] = A[global_id] + B[global_id];
	numid[0] = get_num_groups(i);
	groupid[global_id] = get_group_id(i);
	localid[global_id] = get_local_id(i);
	dimension[0] = get_work_dim();

}
