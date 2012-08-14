#include "ImExpand_cl.h"
#include "clError.h"
#include "../imageBuilder/imStack.h"
#include <iostream>

const char* kernel_source = "/home/martin/cellular/dvreader/mainline/open_cl/expand_image.cl";
const char* kernel_name = "expand_image_k_mask";

ImExpand_cl::ImExpand_cl()
  : OCL_base(kernel_source, kernel_name, true)
{
  kernel_mask = 0;
  local_item_size = 256;
}

ImExpand_cl::~ImExpand_cl()
{
  delete []kernel_mask;
}

// work on one channel only to begin with.
ImStack* ImExpand_cl::expandStack(ImStack* source_stack, unsigned int exp_factor, float sigma)
{
  expansion_factor = exp_factor;
  prepare_kernel_mask(expansion_factor, sigma);
  if(!kernel_mask){
    std::cerr << "ImExpand_cl::expandStack kernel_mask creation failed" << std::endl;
    return(0);
  }

  cl_uint w = source_stack->w();
  cl_uint h = source_stack->h();
  cl_uint d = source_stack->d();

  cl_uint im_size = w * h;
  cl_uint exp_im_size = im_size * expansion_factor * expansion_factor;
  cl_uint kernel_mask_size = exp_factor * exp_factor * 9;

  size_t global_item_size = im_size;
  if(global_item_size % local_item_size){
    std::cerr << "global_item_size is not a multiple of the global_item_size" << std::endl;
    return(0);
  }

  if(! (w * h * h) ){
    std::cerr << "ImExpand_cl::expandStack: empty stack obtained returning NULL stack" << std::endl;
    return(0);
  }
  if(exp_factor < 2){
    std::cerr << "ImExpand_cl::expandStack: exp_factor < 2 makes no sense?" << std::endl;
    return(0);
  }
  
  // could run out of memory here..
  float* dest = new float[ d * exp_im_size ];
  
  cl_int ret;
  // memobjects for the things..
  cl_mem source_mem = clCreateBuffer(context, CL_MEM_READ_ONLY,
				     sizeof(float) * im_size, NULL, &ret);
  cl_mem dest_mem = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
				   sizeof(float) * exp_im_size, NULL, &ret);
  cl_mem kernel_mask_mem = clCreateBuffer(context, CL_MEM_READ_ONLY,
					  sizeof(float) * kernel_mask_size, NULL, & ret);

  ret = clEnqueueWriteBuffer(command_que, kernel_mask_mem, CL_TRUE, 0,
			     sizeof(float) * kernel_mask_size, kernel_mask, 0, NULL, NULL);
  report_error_pf("enque write buffer 1", ret);
  ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&kernel_mask_mem);
  report_error_pf("setKernelArg 2", ret);
  ret = clSetKernelArg(kernel, 3, sizeof(uint), (void*)&w);
  report_error_pf("setKernelArg 3", ret);
  ret = clSetKernelArg(kernel, 4, sizeof(uint), (void*)&h);
  report_error_pf("setKernelArg 4", ret);
  ret = clSetKernelArg(kernel, 5, sizeof(uint), (void*)&expansion_factor);
  report_error_pf("setKernelArg 5", ret);

  // but I don't think I should need to enqueueWriteBuffer here; It should be enough to 
  // read it at the end. The kernel argument though does need to be set. 
  // Check later to see what's necessary.
  ret = clEnqueueWriteBuffer(command_que, dest_mem, CL_TRUE, 0,
			     sizeof(float) * exp_im_size, dest, 0, NULL, NULL);
  report_error_pf("enque_write buffer 2", ret);
  ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&dest_mem);
  report_error_pf("setKernelArg 1", ret);

  
  // source and destination need to be set independently
  for(cl_uint z=0; z < d; ++z){
    ret = clEnqueueWriteBuffer(command_que, source_mem, CL_TRUE, 0,
			       sizeof(float) * im_size, source_stack->l_image(0, z), 0, NULL, NULL);
    report_error_pf("clEnqueueWriteBuffer", ret);
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&source_mem);
    report_error_pf("setKernelArg", ret);

    //and then magically:
    cl_event exp_event;
    ret = clEnqueueNDRangeKernel(command_que, kernel, 1, NULL,
				 &global_item_size, &local_item_size, 0, NULL, &exp_event);
    report_error_pf("clEnqueueNDRangeKernel", ret);

    // wait for it to finish before we go to the next one (otherwise I suppose the enquewritebuffers
    // will end up doing someting strange?
    ret = clWaitForEvents(1, &exp_event);
    report_error_pf("clWaitForEvents", ret);

    ret = clReleaseEvent(exp_event);
    report_error_pf("clReleaseEvent", ret);

    ret = clEnqueueReadBuffer(command_que, dest_mem, CL_TRUE, 0,
			      sizeof(float) * exp_im_size, (dest + exp_im_size * z), 0, NULL, NULL);
    report_error_pf("clEnqueueReadBuffer", ret);
  }
  // no need to delete the kernel_mask memory. That wil lbe taken care of by the destructor.
  ret = clReleaseMemObject(source_mem);
  ret = clReleaseMemObject(dest_mem);
  ret = clReleaseMemObject(kernel_mask_mem);
  report_error_pf("clRelaseMemObject", ret);
  // and make an image stack of the object
  ImStack* dest_stack = new ImStack(dest, source_stack->cinfo(0), source_stack->xp(), 
				    source_stack->yp(), source_stack->zp(),
				    w * expansion_factor, h * expansion_factor, d);
  return(dest_stack);
}

void ImExpand_cl::prepare_kernel_mask(unsigned int exp_factor, float sigma)
{
  if(exp_factor < 2)
    return;
  if(kernel_mask)
    delete []kernel_mask;
  kernel_mask = new float[ 9 * exp_factor * exp_factor ];
  
  // kernel_mask entries are grouped by sub_pixel (column, row), contributing pixel (column row)
  // and the mask contribution calculated using a gaussian.
  int sub_pixel=-1;
  for(unsigned int y=0; y < exp_factor; ++y){
    float sub_pixel_y = ((float)y + 0.5) / (float)exp_factor;
    for(unsigned int x=0; x < exp_factor; ++x){  // here x and y refer to the subpixel locations.
      float sub_pixel_sum = 0;
      float sub_pixel_x = ((float)x + 0.5) / (float)exp_factor;
      sub_pixel = y * exp_factor + x;
      for(int yp=-1; yp <= 1; yp++){
	float pixel_y = float(yp) + 0.5;
	for(int xp=-1; xp <=1; xp++){
	  float pixel_x = float(xp) + 0.5;
	  float d = 	 
	    (pixel_x - sub_pixel_x) * (pixel_x - sub_pixel_x) +
	    (pixel_y - sub_pixel_y) * (pixel_y - sub_pixel_y);
			 
	  int offset = sub_pixel * 9 + (3 * (yp+1)) + xp + 1;
	  // a gaussian equation with sigma = 1
	  // maximal d (r^2) is (1.5^2 + 1.5^2) = 4.5
	  // with sigma = 1 this gives a 20% contribution which seems reasonable.
	  kernel_mask[ offset ] = exp( -d/ (2.0*sigma*sigma) ) ;
	  sub_pixel_sum += exp( -d / 2.0 );
	  std::cout << "\t" << offset  << "\t" << yp << "," << xp << " : " << d << " --> " << exp(-d / 2.0) << std::endl;
	}
      }
      std::cout << y << "," << x << "  : " << sub_pixel_sum << std::endl;
    }
  }
}
