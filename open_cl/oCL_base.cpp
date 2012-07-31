#include "oCL_base.h"
#include "clError.h"
#include <iostream>
#include <fstream>

OCL_base::OCL_base(const char* kernel_source, bool compile_source)
{
  platform_id = NULL;
  device_id = NULL;
  num_devices = 0;
  num_platforms = 0;
  
  context = NULL;
  command_que = NULL;
  program = NULL;
  kernel = NULL;
  
  local_item_size = 128;

  init_kernel(kernel_source, compile_source);
}

OCL_base::~OCL_base()
{
    // all of the following return a cl_uint error code
  // we should naturally check this, but for first implementation
  // let's try without.

  std::cout << "OCL_base destructor ? incurred here" << std::endl;

  clFlush(command_que);
  clFinish(command_que);
  
  cl_int ret;

  ret = clReleaseKernel(kernel);
  if(ret) report_error_pf("release kernel", ret);
  ret = clReleaseProgram(program);
  if(ret) report_error_pf("release program", ret);
  ret = clReleaseCommandQueue(command_que);
  if(ret) report_error_pf("release command_queue", ret);
  ret = clReleaseContext(context);
  if(ret) report_error_pf("releaseContext", ret);

}

void OCL_base::set_local_item_size(size_t li_size)
{
  local_item_size = li_size;
}

void OCL_base::device_properties()
{
  cl_int ret = 0;
  cl_uint compute_unit_no = 0;
  ret = clGetDeviceInfo(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &compute_unit_no, NULL);
  cl_ulong global_mem_size = 0;
  ret = clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &global_mem_size, NULL);
  cl_ulong local_mem_size = 0;
  ret = clGetDeviceInfo(device_id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &local_mem_size, NULL);
  size_t device_max_work_group_size = 0;
  ret = clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &device_max_work_group_size, NULL);
  
  std::cout << "Device properties:" << std::endl
	    << "Max compute units: " << compute_unit_no << std::endl
	    << "Max work group   : " << device_max_work_group_size << std::endl
	    << "Global memory    : " << global_mem_size << std::endl
	    << "Local memory     : " << local_mem_size << std::endl;
}

void OCL_base::init_kernel(const char* kernel_source, bool compile_source)
{
  if(!compile_source){
    std::cerr << "Binary sources not supported yet" << std::endl;
    return;
  }

  std::ifstream in(kernel_source, std::ios::binary);
  if(!in){
    std::cerr << "Unable to open kernel source file" << std::endl;
    return;
  }
  in.seekg(0, std::ios::end);
  ssize_t end_pos = in.tellg();
  std::cout << "in.tellg() reports : " << in.tellg() << std::endl;
  in.seekg(0, std::ios::beg);
  if(!end_pos){
    std::cerr << "Unable to read from kernel source file" << std::endl;
    return;
  }
  char* kernel_buffer = new char[ (size_t)end_pos + 1 ];
  kernel_buffer[end_pos] = 0;  // this may not be needed.
  in.read(kernel_buffer, end_pos);
  if(in.gcount() != end_pos){
    std::cerr << "Unable to read to end of kernel source file: " << end_pos << " != " << in.gcount() << std::endl;
    delete []kernel_buffer;
    return;
  }
  
  cl_int ret = 0; // return value. Use the same for all.
  
  ret = clGetPlatformIDs(1, &platform_id, &num_platforms);
  if(ret) report_error_pf("clGetPlatformIDs", ret);
  ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_GPU, 1,
			&device_id, &num_devices );
  if(ret) report_error_pf("clGetDeviceIDs", ret);
  if(ret != CL_SUCCESS){
    std::cerr << "clGetDevice returned with error: " << (int)ret << std::endl;
    return;
  }

  context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
  report_error_pf("clCreateContext", ret);

  command_que = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret);
  report_error_pf("clCreateCommandQueue", ret);

  program = clCreateProgramWithSource(context, 1, (const char**)&kernel_buffer, 
				      (const size_t*)&end_pos, &ret);
  report_error_pf("clCreateProgramWithSource", ret);

  clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
  report_error_pf("clBuildProgram", ret);

  kernel = clCreateKernel(program, "vector_f_mip", &ret);
  report_error_pf("clCreateKernel", ret);
  
  delete []kernel_buffer;
  
}
