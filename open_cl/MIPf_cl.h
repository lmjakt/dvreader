#ifndef MIP_CL_H
#define MIP_CL_H

#include <CL/cl.h>

class ImStack;

class MIPf_cl
{
 public:
  MIPf_cl();
  ~MIPf_cl();

  ImStack* projectStack(ImStack* im_stack);
  
 private:
  cl_platform_id platform_id;
  cl_device_id device_id;
  cl_uint num_devices;
  cl_uint num_platforms;

  cl_context context;
  cl_command_queue command_que;
  
  cl_program program;
  cl_kernel kernel;

  // and some functions
  void init_kernel();   // read and compile the kernel
  void project(float* img, float* pro);
};

#endif
