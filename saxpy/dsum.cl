__kernel void dsum(__global float *src, __global float *dst, float factor)
{
  int i = get_global_id(0);
  dst[i] += src[i] + src[i];
}
