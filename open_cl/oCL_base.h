#ifndef OCL_BASE_H
#define OCL_BASE_H

// a base class for single kernel GPU implemented kernel functions.

#include <CL/cl.h>

class OCL_base {
 public:
  OCL_base(const char* kernel_source, bool compile_source=true);
  ~OCL_base();

  void set_local_item_size(size_t li_size);
  void device_properties();

 protected:
  cl_platform_id platform_id;
  cl_device_id device_id;
  cl_uint num_devices;
  cl_uint num_platforms;
  
  cl_context context;
  cl_command_queue command_que;
  
  cl_program program;
  cl_kernel kernel;
  
  size_t local_item_size;
  
  // and some functions
  void init_kernel(const char* kernel_source, bool compile_source);   // read and compile the kernel

};

#endif
