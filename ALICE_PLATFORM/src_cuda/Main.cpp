
#ifdef _MAIN_
{

#include "kernel_test.cuh"

using namespace CUDA;

int main(int argc, char *argv[])
{
	const int arraySize = 5;
	const int a[arraySize] = { 1, 2, 3, 4, 5 };
	const int b[arraySize] = { 10, 20, 30, 40, 50 };
	int c[arraySize] = { 0 };

	parallelSum(c, a, b, arraySize);

}
}

#endif // _MAIN_