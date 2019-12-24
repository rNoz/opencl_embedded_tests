__kernel void
dmul(__global float* src, __global float* dst, float factor)
{
  int i = get_global_id(0);
  dst[i] += 2.0f * src[i];
}
