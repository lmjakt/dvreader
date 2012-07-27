#include "MIPf_cl.h"
#include "clError.h"
#include "../imageBuilder/imStack.h"
#include <fstream>
#include <iostream>
#include <vector>

const char* const kernel_source = "/home/martin/cellular/dvreader/mainline/open_cl/vector_f_mip_kernel.cl";

MIPf_cl::MIPf_cl()
{
  // set some stuff to null.
  platform_id = NULL;
  device_id = NULL;
  num_devices = 0;
  num_platforms = 0;
  
  context = NULL;
  command_que = NULL;
  program = NULL;
  kernel = NULL;

  init_kernel();
}

MIPf_cl::~MIPf_cl()
{
  // all of the following return a cl_uint error code
  // we should naturally check this, but for first implementation
  // let's try without.
  clFlush(command_que);
  clFinish(command_que);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseCommandQueue(command_que);
  clReleaseContext(context);
}

ImStack* MIPf_cl::projectStack(ImStack* im_stack)
{
  size_t im_size = im_stack->w() * im_stack->h();
  if(!im_size) return(im_stack);
  unsigned int depth = im_stack->d();
  unsigned int channel_no = im_stack->ch();
  cl_int ret = 0;

  
  cl_mem pro_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, 
				      sizeof(float) * im_size, NULL, &ret);
  cl_mem img_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, 
				      sizeof(float) * im_size, NULL, &ret);
  report_error_pf("cl_mem_object created", ret);

  // time information:
  cl_ulong kernel_time = 0;
  cl_ulong img_enque_time = 0; 

  float** projection_data = new float*[channel_no];
  for(uint i=0; i < channel_no; ++i){
    projection_data[i] = new float[im_size];
    memcpy(projection_data[i], im_stack->image(i, 0), sizeof(float) * im_size);

    ret = clEnqueueWriteBuffer(command_que, pro_mem_obj, CL_TRUE, 0,
			       sizeof(float) * im_size, projection_data[i], 0, NULL, NULL);
    report_error_pf("clEnqWriteBuffer for projection_data", ret);
    
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&pro_mem_obj);
    report_error_pf("clSetKernelArg for projection_data", ret);
    
    for(uint z=1; z < depth; ++z){
      cl_event write_img_event;
      cl_ulong start, end;
      ret = clEnqueueWriteBuffer(command_que, img_mem_obj, CL_TRUE, 0,
				 sizeof(float) * im_size, im_stack->image(i, z), 0, NULL, &write_img_event);
      if(ret) report_error_pf("clEnqWriteBuffer for img_data", ret);

      ret = clGetEventProfilingInfo(write_img_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
      ret = clGetEventProfilingInfo(write_img_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
      img_enque_time += (end - start);
      std::cout << "Time taken to enque img buffer: " << (end-start) / 1000 << " us" << std::endl;

      ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&img_mem_obj);
      if(ret) report_error_pf("clSetKernelArg for img_data", ret);

      // and then magically we will run the thing..
      size_t global_item_size = im_size;   /// DANGER. WE NEED TO BE SMARTER ABOUT ALLOCATION.
      size_t local_item_size = 64; // No particular reason. global_item_size = n * local_item_size
      cl_event mip_event;
      ret = clEnqueueNDRangeKernel(command_que, kernel, 1, NULL,
				   &global_item_size, &local_item_size, 0, NULL, &mip_event);
      if(ret) report_error_pf("clEnqueNDRangeKernel", ret);

      // wait for it to complete
      ret = clWaitForEvents(1, &mip_event);
      if(ret) report_error_pf("clWaitForEvents for projection_data", ret);

      // lets report on time taken.
      ret = clGetEventProfilingInfo(mip_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
      ret = clGetEventProfilingInfo(mip_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
      
      kernel_time += (end - start);
      std::cout << "\tTime taken to mip one slice: " << (end-start)/1000  << " us" << std::endl;
    }
    // the below should be movied out of the inner loop!!
    ret = clEnqueueReadBuffer(command_que, pro_mem_obj, CL_TRUE, 0,
			      sizeof(float) * im_size, projection_data[i], 0, NULL, NULL);
    if(ret) report_error_pf("clEnqReadBuffer for projection_data", ret);
  }
  std::cout << "Total kernel time to project: " << kernel_time/1000 << " us" << std::endl;
  std::cout << "Total img_enque time        : " << img_enque_time/1000 << std::endl;
  // make sure it's all done. I don't think this should be necessary, but..
  clFlush(command_que);
  clFinish(command_que);
  clReleaseMemObject(pro_mem_obj);
  clReleaseMemObject(img_mem_obj);
  
  // and make an imStack from the projection_data.
  std::vector<channel_info> ch_info = im_stack->c_info();
  ImStack* pro_stack = new ImStack(projection_data, ch_info,
				   im_stack->xp(), im_stack->yp(), im_stack->zp() + (depth / 2),
				   im_stack->w(), im_stack->h(), 1);
  return(pro_stack);
}

// we will assume that we have a GPU to work on. If not, this function
// won't work
void MIPf_cl::init_kernel()
{
  std::ifstream in(kernel_source, std::ios::binary);
  if(!in){
    std::cerr << "Unable to open kernel source file" << std::endl;
    return;
  }
  //in.seekg(ios_base::beg);
  in.seekg(0, std::ios::end);
  ssize_t end_pos = in.tellg();
  std::cout << "in.tellg() reports : " << in.tellg() << std::endl;
  //std::streampos end_pos = in.tellg();
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
  ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_GPU, 1,
			&device_id, &num_devices );
  if(ret != CL_SUCCESS){
    std::cerr << "clGetDevice returned with error: " << (int)ret << std::endl;
    return;
  }
  if(num_devices != 1){
    std::cerr << "Num Devices not equal to 1" << std::endl;
    return;
  }

  // Lets get some information about the card that we have.
  cl_uint compute_unit_no;
  ret = clGetDeviceInfo(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &compute_unit_no, NULL);
  cl_ulong global_mem_size;
  ret = clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &global_mem_size, NULL);
  cl_ulong local_mem_size;
  ret = clGetDeviceInfo(device_id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &local_mem_size, NULL);
  size_t device_max_work_group_size;
  ret = clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &device_max_work_group_size, NULL);
  
  std::cout << "Device properties:" << std::endl
	    << "Max compute units: " << compute_unit_no << std::endl
	    << "Max work group   : " << device_max_work_group_size << std::endl
	    << "Global memory    : " << global_mem_size << std::endl
	    << "Local memory     : " << local_mem_size << std::endl;
  
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
  // if the above fails, I suppose that program will end up being equal to NULL
  // but I haven't found that in the documentation yet.

  // we have a MEMORY LEAK here as I haven't deleted the buffer for the program.
  // this shouldn't be required after compilation, but yet again, I've not found
  // the answer to this.
}
				      
