#pragma once

#if CUDA_FOUND
	#if COMPILE_FOR_CUDA
		#include <thrust/host_vector.h>
		#define GPU_PREFIX __device__
		#define COMPILE_FOR_GPU 1
        #define GRAPHICS_LIBRARY_HAS_NAMESPACE 0
	#else
		#define GPU_PREFIX 	
		#define COMPILE_FOR_GPU 0
        #define GRAPHICS_LIBRARY_HAS_NAMESPACE 1
	#endif
#else
	#define GPU_PREFIX
	#define COMPILE_FOR_GPU 0
    #define GRAPHICS_LIBRARY_HAS_NAMESPACE 1
#endif


