#ifndef IMEXPAND_CL_H
#define IMEXPAND_CL_H

#include "oCL_base.h"
#include <CL/cl.h>

class ImStack;

class ImExpand_cl : public OCL_base
{
 public:
  ImExpand_cl();
  ~ImExpand_cl();

  ImStack* expandStack(ImStack* source_stack, unsigned int exp_factor, float sigma=1);

 private:
  float* kernel_mask;  // of length 9 * exp_factor^2. (See vector_f_mip_kernel.cl for how to pack it)
  cl_uint expansion_factor;
  cl_uint kernel_mask_size;
  
  void prepare_kernel_mask(unsigned int exp_factor, float sigma);
  
};
  

#endif
