#ifndef MIP_CL_H
#define MIP_CL_H

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "oCL_base.h"


class ImStack;

// The following are defined by the parent class and are set by the
// parental init_kernel function called by the parent constructor

/* cl_platform_id platform_id; */
/* cl_device_id device_id; */
/* cl_uint num_devices; */
/* cl_uint num_platforms; */

/* cl_context context; */
/* cl_command_queue command_que; */

/* cl_program program; */
/* cl_kernel kernel; */

/* size_t local_item_size; */


class MIPf_cl : public OCL_base
{
 public:
  MIPf_cl();
  ~MIPf_cl();

  ImStack* projectStack(ImStack* im_stack);

 private:

  // and some functions
  //void project(float* img, float* pro);
};

#endif
