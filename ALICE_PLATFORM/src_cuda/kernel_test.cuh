#pragma once

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <iostream>

namespace CUDA
{
	void parallelSum(int *c, const int *a, const int *b, unsigned int size);
}
