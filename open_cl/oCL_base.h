#ifndef OCL_BASE_H
#define OCL_BASE_H

// a base class for single kernel GPU implemented kernel functions.

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <string>

class OCL_base {
 public:
  OCL_base(const char* kernel_source, const char* kernel_name, bool compile_source=true);
  OCL_base(const char* kernel_source, const char* kernel_name, 
	   std::string define_statements, bool compile_source=true);

  ~OCL_base();

  void set_local_item_size(size_t li_size);
  void device_properties();
  void kernel_properties();

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
  void init_kernel(const char* kernel_source, const char* kernel_name, 
		   std::string define_statements, bool compile_source);   // read and compile the kernel
  cl_ulong time_command(cl_event* event);

};

#endif
