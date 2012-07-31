#include "MIPf_cl.h"
#include "clError.h"
#include "../imageBuilder/imStack.h"
#include <fstream>
#include <iostream>
#include <vector>

const char* const kernel_source = "/home/martin/cellular/dvreader/mainline/open_cl/vector_f_mip_kernel.cl";

MIPf_cl::MIPf_cl()
  : OCL_base(kernel_source, true)
{
  // setup is handled by the parent class
}

MIPf_cl::~MIPf_cl()
{
  // mostly handled by the parental class
}

ImStack* MIPf_cl::projectStack(ImStack* im_stack)
{
  // check whether kernel and stuff is set up correctly //
  if(!kernel)
    std::cerr << "Kernel not defined" << std::endl;
  if(!context)
    std::cerr << "No context specified" << std::endl;
  if(!command_que)
    std::cerr << "No command queue specified" << std::endl;
  if(!kernel || !context || !command_que)
    return(0);

  size_t im_size = im_stack->w() * im_stack->h();
  if(!im_size) return(0);
  
  if(im_size % local_item_size){
    std::cerr << "image size is not a multiple of local_item_size: " << im_size << " % " 
	      << local_item_size << " = " << im_size % local_item_size << std::endl;
    return(0);
  }

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
      // making clEnqueueWriteBuffer non-blocking (CL_FALSE) speeds up the event reported time, but
      // does not seem to have much effect on the overall speed.
      ret = clEnqueueWriteBuffer(command_que, img_mem_obj, CL_TRUE, 0,
				 sizeof(float) * im_size, im_stack->image(i, z), 0, NULL, &write_img_event);
      if(ret) report_error_pf("clEnqWriteBuffer for img_data", ret);

      ret = clGetEventProfilingInfo(write_img_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
      ret = clGetEventProfilingInfo(write_img_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
      img_enque_time += (end - start);
      //      std::cout << "Time taken to enque img buffer: " << (end-start) / 1000 << " us" << std::endl;

      ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&img_mem_obj);
      if(ret) report_error_pf("clSetKernelArg for img_data", ret);

      // and then magically we will run the thing..
      size_t global_item_size = im_size;   /// DANGER. WE NEED TO BE SMARTER ABOUT ALLOCATION.
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
      ret = clReleaseEvent(write_img_event);
      if(ret) report_error_pf("clReleaseEvent write_imge_event", ret);
      ret = clReleaseEvent(mip_event);
      if(ret) report_error_pf("clReleaseEvent write_imge_event", ret);
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
				      
