__kernel void vector_f_mip(__global float* img, __global float* pro)
{
  // img is two dimensional, but can be treated as a one dimensional string
  // for this.
  int i = get_global_id(0);
  pro[i] = img[i] > pro[i] ? img[i] : pro[i];
}
