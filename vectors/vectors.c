/*

Sources:
http://www.eriksmistad.no/getting-started-with-opencl-and-gpu-computing/

*/

// openCL headers

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SOURCE_vector_len (0x100000)

#define CL_CHECK(_expr)                                                        \
  do {                                                                         \
    cl_int _err = _expr;                                                       \
    if (_err == CL_SUCCESS)                                                    \
      break;                                                                   \
    fprintf(stderr, "OpenCL Error: '%s' returned %d!\n", #_expr, (int)_err);   \
    abort();                                                                   \
  } while (0)

#define CL_CHECK_ERR(_expr)                                                    \
  ({                                                                           \
    cl_int _err = CL_INVALID_VALUE;                                            \
    typeof(_expr) _ret = _expr;                                                \
    if (_err != CL_SUCCESS) {                                                  \
      fprintf(stderr, "OpenCL Error: '%s' returned %d!\n", #_expr, (int)_err); \
      abort();                                                                 \
    }                                                                          \
    _ret;                                                                      \
  })

enum Operation
{
  OP_ADD,
  OP_MUL,
};

int
main(int argc, char** argv)
{

  int vector_len = 1024;
  char* vector_str = getenv("VECTOR");
  if (vector_str != NULL) {
    vector_len = atoi(vector_str);
  }
  printf("vector: %d\n", vector_len);
  char* check_str = getenv("CHECK");
  bool check_res = false;
  if (check_str != NULL && atoi(check_str) > 0) {
    check_res = true;
  }
  printf("check results: %s\n", check_res > 0 ? "true" : "false");

  int deviceId = 0;
  int platformId = 0;
  char* platform_str = getenv("PLATFORM");
  if (platform_str != NULL) {
    platformId = atoi(platform_str);
  }
  char* device_str = getenv("DEVICE");
  if (device_str != NULL) {
    deviceId = atoi(device_str);
  }
  printf("using platform.device: %d.%d\n", platformId, deviceId);

  // Allocate memories for input arrays and output array.
  float* A = (float*)malloc(sizeof(float) * vector_len);
  float* B = (float*)malloc(sizeof(float) * vector_len);

  // Output
  float* C = (float*)malloc(sizeof(float) * vector_len);

  // Initialize values for array members.
  int i = 0;
  for (i = 0; i < vector_len; ++i) {
    A[i] = i + 1;
    B[i] = (i + 1) * 2;
  }

  // Load kernel from file vecAddKernel.cl

  FILE* kernelFile;
  char* kernelSource;
  size_t kernelSize;

  char* kernelfile;
  if (argc >= 2) {
    kernelfile = argv[1];
  } else {
    printf("usage: <kernel file.cl>\n");
    exit(1);
  }
  enum Operation op;
  char operation[7];
  memcpy(operation, kernelfile, 6);
  operation[6] = '\0';
  if (strcmp(operation, "vecadd") == 0) {
    op = OP_ADD;
    printf("operation: vecadd\n");
  } else if (strcmp(operation, "vecmul") == 0) {
    op = OP_MUL;
    printf("operation: vecmul\n");
  } else {
    printf("not recognized operation (vecmul|vecadd) in kernelfile\n");
    exit(1);
  }

  kernelFile = fopen(kernelfile, "r");

  if (!kernelFile) {

    fprintf(stderr, "No file named vecAddKernel.cl was found\n");

    exit(-1);
  }
  kernelSource = (char*)malloc(MAX_SOURCE_vector_len);
  kernelSize = fread(kernelSource, 1, MAX_SOURCE_vector_len, kernelFile);
  fclose(kernelFile);

  // Getting platform and device information
  // cl_device_id device = NULL;
  // cl_uint retNumDevices;
  // cl_uint retNumPlatforms;

  cl_platform_id platforms[10];
  cl_device_id devices[10];
  CL_CHECK(clGetPlatformIDs(10, platforms, NULL));
  CL_CHECK(clGetDeviceIDs(
    platforms[platformId], CL_DEVICE_TYPE_ALL, 10, devices, NULL));

  cl_device_id device = devices[deviceId];

  size_t max_wg_size;
  CL_CHECK(clGetDeviceInfo(device,
                           CL_DEVICE_MAX_WORK_GROUP_SIZE,
                           sizeof(max_wg_size),
                           &max_wg_size,
                           NULL));
  printf("max wg size: %ld\n", max_wg_size);

  // Creating context.
  cl_context context =
    CL_CHECK_ERR(clCreateContext(NULL, 1, &device, NULL, NULL, &_err));

  // Creating command queue
  cl_command_queue commandQueue;
  cl_command_queue_properties qproperties = { 0 };
  commandQueue = CL_CHECK_ERR(
    clCreateCommandQueueWithProperties(context, device, &qproperties, &_err));

  // cl_command_queue commandQueue = clCreateCommandQueue(context, device, 0,
  // &ret);

  // Memory buffers for each array
  cl_mem aMemObj = CL_CHECK_ERR(clCreateBuffer(
    context, CL_MEM_READ_ONLY, vector_len * sizeof(float), NULL, &_err));
  cl_mem bMemObj = CL_CHECK_ERR(clCreateBuffer(
    context, CL_MEM_READ_ONLY, vector_len * sizeof(float), NULL, &_err));
  cl_mem cMemObj = CL_CHECK_ERR(clCreateBuffer(
    context, CL_MEM_WRITE_ONLY, vector_len * sizeof(float), NULL, &_err));

  // Copy lists to memory buffers
  CL_CHECK(clEnqueueWriteBuffer(commandQueue,
                                aMemObj,
                                CL_TRUE,
                                0,
                                vector_len * sizeof(float),
                                A,
                                0,
                                NULL,
                                NULL));
  CL_CHECK(clEnqueueWriteBuffer(commandQueue,
                                bMemObj,
                                CL_TRUE,
                                0,
                                vector_len * sizeof(float),
                                B,
                                0,
                                NULL,
                                NULL));

  // Create program from kernel source
  cl_program program =
    CL_CHECK_ERR(clCreateProgramWithSource(context,
                                           1,
                                           (const char**)&kernelSource,
                                           (const size_t*)&kernelSize,
                                           &_err));

  // Build program
  CL_CHECK(clBuildProgram(program, 1, &device, NULL, NULL, NULL));

  // Create kernel
  cl_kernel kernel = CL_CHECK_ERR(clCreateKernel(program, operation, &_err));

  // Set arguments for kernel
  cl_int ret;
  ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&aMemObj);
  ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&bMemObj);
  ret |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&cMemObj);
  if (ret != CL_SUCCESS) {
    fprintf(stderr, "OpenCL Error: '%s' returned %d!\n", "clSetKernelArg", ret);
    abort();
  }

  // Execute the kernel
  size_t globalItemSize = vector_len;
  // size_t localItemSize = 64; // globalItemSize has to be a multiple of
  // localItemSize. 1024/64 = 16
  // ret = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
  // &globalItemSize, &localItemSize, 0, NULL, NULL);
  CL_CHECK(clEnqueueNDRangeKernel(
    commandQueue, kernel, 1, NULL, &globalItemSize, NULL, 0, NULL, NULL));

  // Read from device back to host.
  CL_CHECK(clEnqueueReadBuffer(commandQueue,
                               cMemObj,
                               CL_TRUE,
                               0,
                               vector_len * sizeof(float),
                               C,
                               0,
                               NULL,
                               NULL));

  // Write result
  /*
  for (i=0; i<vector_len; ++i) {

          printf("%f + %f = %f\n", A[i], B[i], C[i]);

  }
  */

  // Test if correct answer
  bool ok = true;
  for (i = 0; i < vector_len; ++i) {
    float check;
    if (op == OP_ADD) {
      check = A[i] + B[i];
    } else if (op == OP_MUL) {
      check = A[i] * B[i];
    }
    if (i < 4 || i > (vector_len - 5)) {
      printf("[%d] OpenCL (%.5f) Host (%.5f)\n", i, C[i], check);
    }
    if (check_res) {
      if (C[i] != check) {
        printf("[FAILURE] [%d] OpenCL (%.5f) Host (%.5f)\n", i, C[i], check);
        ok = false;
      }
    }
  }
  if (check_res && ok) {
    printf("Everything seems to work fine! \n");
  }

  // Clean up, release memory.
  ret = clFlush(commandQueue);
  ret |= clFinish(commandQueue);
  ret |= clReleaseCommandQueue(commandQueue);
  ret |= clReleaseKernel(kernel);
  ret |= clReleaseProgram(program);
  ret |= clReleaseMemObject(aMemObj);
  ret |= clReleaseMemObject(bMemObj);
  ret |= clReleaseMemObject(cMemObj);
  ret |= clReleaseContext(context);
  if (ret != CL_SUCCESS) {
    fprintf(stderr, "OpenCL Error: '%s' returned %d!\n", "clRelease...", ret);
    abort();
  }
  free(A);
  free(B);
  free(C);

  return 0;
}
