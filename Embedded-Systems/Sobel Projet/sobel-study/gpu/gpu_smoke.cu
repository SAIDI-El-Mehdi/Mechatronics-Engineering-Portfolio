#include <cstdio>
#include <cuda_runtime.h>

__global__ void smoke() {}

int main() {
    int drv = 0, rt = 0, count = 0;
    cudaError_t err;

    err = cudaDriverGetVersion(&drv);
    if (err != cudaSuccess) {
        std::printf("cudaDriverGetVersion failed: %s\n", cudaGetErrorString(err));
        return 1;
    }

    err = cudaRuntimeGetVersion(&rt);
    if (err != cudaSuccess) {
        std::printf("cudaRuntimeGetVersion failed: %s\n", cudaGetErrorString(err));
        return 1;
    }

    std::printf("CUDA driver API version : %d\n", drv);
    std::printf("CUDA runtime version    : %d\n", rt);

    err = cudaGetDeviceCount(&count);
    if (err != cudaSuccess) {
        std::printf("cudaGetDeviceCount failed: %s\n", cudaGetErrorString(err));
        return 1;
    }

    std::printf("CUDA device count       : %d\n", count);
    if (count < 1) {
        std::puts("No CUDA device found");
        return 1;
    }

    cudaDeviceProp prop{};
    err = cudaGetDeviceProperties(&prop, 0);
    if (err != cudaSuccess) {
        std::printf("cudaGetDeviceProperties failed: %s\n", cudaGetErrorString(err));
        return 1;
    }

    std::printf("Device 0 name           : %s\n", prop.name);
    std::printf("Compute capability      : %d.%d\n", prop.major, prop.minor);

    err = cudaFree(0);
    if (err != cudaSuccess) {
        std::printf("cudaFree(0) failed: %s\n", cudaGetErrorString(err));
        return 1;
    }

    smoke<<<1,1>>>();
    err = cudaGetLastError();
    if (err != cudaSuccess) {
        std::printf("kernel launch failed: %s\n", cudaGetErrorString(err));
        return 1;
    }

    err = cudaDeviceSynchronize();
    if (err != cudaSuccess) {
        std::printf("cudaDeviceSynchronize failed: %s\n", cudaGetErrorString(err));
        return 1;
    }

    std::puts("CUDA OK");
    return 0;
}
