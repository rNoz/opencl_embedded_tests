# OpenCL Embedded Tests

```
make build
make clean
```

# Vectors

Supports 2 types of operations using 3 vectors of floats:

1. vecadd: 2 vector addition
2. vecmul: 2 vector multiplication

It knows the type of operation based on the name of the kernel. Variants of every kernel
can be tried if you keep the first 6 chars (`vecadd.v1.cl`, `vecmul.opt.cl`, etc).

It accepts the following env vars:
- VECTOR: (int) number of elements per vector
- CHECK: (int) 1|0 to check the results in the host side
- PLATFORM: (int) OpenCL platform 
- DEVICE: (int) OpenCL device 

Usage examples:

```
cd vectors
VECTOR=12 CHECK=1 sudo -E ./build/vectors vecadd.cl
VECTOR=24 sudo -E ./build/vectors vecadd.cl
PLATFORM=1 VECTOR=1024 sudo -E ./build/vectors vecmul.cl
```

# Saxpy

It multiplies each element of a vector by a factor, and stores it in another vector. 

To try another behaviors regarding OpenCL, this program performs many clEnqueueReadBuffer and clEnqueueWriteBuffer operations (one per vector element).

Then, this program is extended to support other kernels. It accepts:

1. saxpy: vector by factor
2. dsum: sum the same vector with itself
3. dmul: multiply the vector by 2.0

It knows the type of operation based on the name of the kernel. Variants of every kernel
can be tried if you keep the first chars (`saxpy.v1.cl`, `dsum.opt.cl`, etc).

It accepts the following env vars:
- VECTOR: (int) number of elements per vector
- CHECK: (int) 1|0 to check the results in the host side
- FACTOR: (float) factor to multiply each element
- PLATFORM: (int) OpenCL platform 
- DEVICE: (int) OpenCL device 

```
cd saxpy
FACTOR=3.1415 VECTOR=12 CHECK=1 sudo -E ./build/saxpy saxpy.cl
PLATFORM=1 FACTOR=2.0 VECTOR=24 CHECK=1 sudo -E ./build/saxpy saxpy.cl
PLATFORM=1 VECTOR=24 CHECK=1 sudo -E ./build/saxpy dsum.cl
VECTOR=24 CHECK=1 sudo -E ./build/saxpy dmul.cl
```
