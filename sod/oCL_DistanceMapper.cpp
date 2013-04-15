#include "oCL_DistanceMapper.h"
#include "../open_cl/oCL_base.h"
#include "../open_cl/clError.h"
#include <string.h>

const char* kernel_source = "/home/martin/cellular/dvreader/mainline/sod/move_deltoids.cl";
const char* kernel_name = "move_deltoids";

OCL_DistanceMapper::OCL_DistanceMapper()
  : OCL_base(kernel_source, kernel_name, true)
{
  dimFactors = 0;
}

OCL_DistanceMapper::~OCL_DistanceMapper()
{
  if(dimFactors)
    delete []dimFactors;
}

std::vector<stressInfo> OCL_DistanceMapper::reduce_dimensions(float* points, unsigned int node_no,
							     unsigned int starting_dimensionality,
							     unsigned int target_dimensionality,
							     unsigned int iterations,
							     float* distances)
{
  std::vector<stressInfo> stress_data;
  // check whether kernel and stuff is set up correctly //
  if(!kernel)
    std::cerr << "Kernel not defined" << std::endl;
  if(!context)
    std::cerr << "No context specified" << std::endl;
  if(!command_que)
    std::cerr << "No command queue specified" << std::endl;
  if(!kernel || !context || !command_que)
    return(stress_data);
  
  if(!starting_dimensionality || !target_dimensionality){
    std::cerr << "OCL_DistanceMapper null starting or target dimensionality "
	      << "Start: " << starting_dimensionality << "  Target: " << target_dimensionality 
	      << std::endl;
    return(stress_data);
  }

  if(target_dimensionality > starting_dimensionality){
    uint d = target_dimensionality;
    target_dimensionality = starting_dimensionality;
    starting_dimensionality = d;
  }
  if(!node_no){
    std::cerr << "OCL_DistanceMapper null node_no" << std::endl;
    return(stress_data);
  }
  // Set up the dimFactor vector defining the extent of different dimensions
  dimensionality = starting_dimensionality;
  t_dimensionality = target_dimensionality;
  if(dimFactors) delete []dimFactors;
  dimFactors = new float[dimensionality];
  for(uint i=0; i < dimensionality; ++i)
    dimFactors[i] = 1.0;
  
  // let's set up a global item size, as a multiple of local_item_size
  // at the moment local_item_size is hardcoded into the constructor, but
  // it can be changed via a call to setLocalItemSize()

  size_t global_item_size = local_item_size;
  while(global_item_size < node_no)
    global_item_size += local_item_size;
  
  std::cout << "local item size is : " << local_item_size << "\n"
	    << "global item size is: " << global_item_size << std::endl;

  // set up the appropriate buffers
  float* points_j = new float[ node_no * dimensionality ];
  memset((void*)points_j, 0, sizeof(float) * node_no * dimensionality);

  float* stress = new float[ node_no ];
  float* force_vector = new float[ node_no * dimensionality ];
  
  cl_int ret = 0;

  //  unsigned int* error_buffer = new unsigned int[node_no];

  // And then we need to create a set of mem objects
  // note we use NULL here since we are not asking the GPU to use
  // host memory
  cl_mem pos_i_obj = clCreateBuffer(context, CL_MEM_READ_WRITE,
				    sizeof(float) * node_no * dimensionality, NULL, &ret);
  cl_mem pos_j_obj = clCreateBuffer(context, CL_MEM_READ_WRITE,
				    sizeof(float) * node_no * dimensionality, NULL, &ret);
  cl_mem stress_obj = clCreateBuffer(context, CL_MEM_READ_WRITE,
				     sizeof(float) * node_no, NULL, &ret);
  cl_mem dim_factors_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
					  sizeof(float) * dimensionality, NULL, &ret);
  cl_mem distances_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
					sizeof(float) * node_no * node_no, NULL, &ret);
  
  cl_mem force_vector_obj = clCreateBuffer(context, CL_MEM_READ_WRITE,
					   sizeof(float) * node_no * dimensionality, NULL, &ret);

  //cl_mem error_buffer_obj = clCreateBuffer(context, CL_MEM_READ_WRITE,
  //					   sizeof(unsigned int) * node_no, NULL, &ret);
  //
  if(ret) report_error_pf("end of clCreateBuffer section", ret);
  ret = 0;

  //// enque the resulting buffers
  ret = clEnqueueWriteBuffer(command_que, pos_j_obj, CL_TRUE, 0,
			     sizeof(float) * node_no * dimensionality, (void*)points_j,
			     0, NULL, NULL);

  ret = clEnqueueWriteBuffer(command_que, stress_obj, CL_TRUE, 0,
			     sizeof(float) * node_no, (void*)stress,
			     0, NULL, NULL);

  ret = clEnqueueWriteBuffer(command_que, distances_obj, CL_TRUE, 0,
			     sizeof(float) * node_no * node_no, (void*) distances,
			     0, NULL, NULL);
  
  ret = clEnqueueWriteBuffer(command_que, force_vector_obj, CL_TRUE, 0,
			     sizeof(float) * dimensionality * node_no, (void*)force_vector,
			     0, NULL, NULL);

  //ret = clEnqueueWriteBuffer(command_que, error_buffer_obj, CL_TRUE, 0,
  //			     sizeof(unsigned int) * node_no, (void*)error_buffer,
  //			     0, NULL, NULL);

  // though this only checks the last command.
  if(ret) report_error_pf("end of clEnqueueWriteBuffer section", ret);
  
  /// and then we can finally set the arguments, before looping through and getting
  /// the kernel to do it's thing.


  ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&pos_j_obj);
  ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&stress_obj);
  ret = clSetKernelArg(kernel, 3, sizeof(uint), &dimensionality);
  ret = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*)&distances_obj);
  ret = clSetKernelArg(kernel, 6, sizeof(uint), (void*)&node_no);
  ret = clSetKernelArg(kernel, 7, sizeof(cl_mem), (void*)&force_vector_obj);
  //ret = clSetKernelArg(kernel, 8, sizeof(cl_mem), (void*)&error_buffer_obj);

  if(ret) report_error_pf("end of clSetKernelArg section", ret);

  // initially write the loop in the stupid way.
  // 1. enque the initial points (pos_i_obj) using the points
  // 2. call clEnqueueNDRangeKernel
  // 3. Wait
  // 4. call clEnqueueReadBuffer to read pos_j_obj to points
  // 5. repeat from 1
  //
  // strictly we could just pass leave the memory on the GPU
  // but ask the kernel to map from i to j or j to i depending
  // on whether odd or even.
  // 
  for(unsigned int i=0; i < iterations; ++i){
    ret = clEnqueueWriteBuffer(command_que, pos_i_obj, CL_TRUE, 0,
			       sizeof(float) * node_no * dimensionality, (void*)points,
			       0, NULL, NULL);
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&pos_i_obj);

    ret = clEnqueueWriteBuffer(command_que, dim_factors_obj, CL_TRUE, 0,
			      sizeof(float) * dimensionality, (void*)dimFactors,
			      0, NULL, NULL);
    ret = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void*)&dim_factors_obj);
    
    
    cl_event move_event;
    ret = clEnqueueNDRangeKernel(command_que, kernel, 1, NULL,
				 &global_item_size, &local_item_size, 0, NULL, &move_event);
    if(ret) report_error_pf("clEnqueueNDRangeKernel", ret);
    
    ret = clWaitForEvents(1, &move_event);
    if(ret) report_error_pf("clWaitForEvents", ret);
    // read the old points to the new ones
    ret = clEnqueueReadBuffer(command_que, pos_j_obj, CL_TRUE, 0,
			      sizeof(float) * node_no * dimensionality,
			      points, 0, NULL, NULL);
    
    // read the stress vector
    ret = clEnqueueReadBuffer(command_que, stress_obj, CL_TRUE, 0,
			      sizeof(float) * node_no,
			      stress, 0, NULL, NULL);
    // here we should do something useful with points and stress, but let's check
    // if the thing compiles first
    float stress_sum = 0;
    for(uint j=0; j < node_no; ++j){
      //   std::cout << stress[j] << "  ";
      stress_sum += stress[j];
    }

    std::cout << "\n" << i << " : " << stress_sum << std::endl;
    
    //ret = clEnqueueReadBuffer(command_que, stress_obj, CL_TRUE, 0,
    //			      sizeof(float) * node_no, error_buffer, 0, NULL, NULL);
    //for(unsigned int j=0; j < node_no; ++j)
    //  std::cout << error_buffer[j] << "  ";
    //std::cout << std::endl;

    shrink_dimensionality(iterations);
    
  }
  //  delete []error_buffer; // and maybe the others as well?
  return(stress_data);  // actually a waste of time..
}


// use a parallel shrink for dims above t_dimensionality
void OCL_DistanceMapper::shrink_dimensionality(unsigned int iter_no)
{
  float r_factor = (1.0 / 0.9) / (float)iter_no;
  for(uint i=t_dimensionality; i < dimensionality; ++i)
    dimFactors[i] = (dimFactors[i] - r_factor) < 0 ? 0 : (dimFactors[i] - r_factor);
  
}
