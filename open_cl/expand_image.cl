// expands an image by some factor
// uses a kernel_mask to calculate new positions
// from neighbouring pixels.

// image is actually taken as just an array of floats

// kernel mask must be of length 9 * exp_factor^2
// and arranged as indicated below.

__kernel void expand_image_k_mask(__global float* img, __global float* exp_im,  
				  __global float* kernel_mask, uint im_width, 
				  uint im_height, uint exp_factor)
{
  size_t i = get_global_id(0);
  int x = i % im_width;
  int y = i / im_width;
  
  int beg_x = x > 0 ? -1 : 0;
  int beg_y = y > 0 ? -1 : 0;

  int end_x = x < im_width ? 1 : 0;
  int end_y = y < im_height ? 1 : 0;

  int sub_pixel = -1;
  float dst_sum, k_sum;
  int k_offset = 0;
  for(int dy=0; dy < exp_factor; ++dy){
    for(int dx=0; dx < exp_factor; ++dx){
      sub_pixel = dy * exp_factor + dx;
      dst_sum = 0.0;
      k_sum = 0;
      for(int xp=beg_x; xp <= end_x; ++xp){
	for(int yp=beg_y; yp <= end_y; ++yp){
	  k_offset = (9 * sub_pixel) + (3 * (1+yp)) + xp + 1; 
	  //	  dst_sum += img[(x+xp) + (y+yp) * im_width] * kernel_mask[(sub_pixel * 9) +  xp + yp * 3] ;

	  dst_sum += img[ i + xp + yp * im_width] * kernel_mask[k_offset];
	  //dst_sum += img[ i + xp + yp * im_width];
	  //k_sum += 1.0;
	  //dst_sum = img[i];
	  //	  dst_sum += img[(x+xp) + (y+yp) * im_width] ;
	  k_sum += kernel_mask[k_offset];
	  // neighbouring pixels..
	}
      }
      //      dst_sum = x % 32 ? 0.0 : 1.0;
      //dst_sum = 1.0 * img[i];
      dst_sum = !k_sum ? 0 : dst_sum / k_sum;
      //exp_im[ (dx + (x * exp_factor)) + (im_width * exp_factor)*(dy + (y * exp_factor)) ] = 0.1;
      exp_im[ (dx + x * exp_factor) + (im_width * exp_factor)*(dy + y * exp_factor) ] = dst_sum;
    }
  }
}
