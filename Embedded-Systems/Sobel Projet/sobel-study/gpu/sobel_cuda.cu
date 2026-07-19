#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cuda_runtime.h>

namespace fs = std::filesystem;

static bool load_pgm(const std::string& path, std::vector<unsigned char>& img, int& w, int& h) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;

    std::string magic;
    f >> magic;
    if (magic != "P5") return false;

    f >> w >> h;
    int maxv;
    f >> maxv;
    f.get();

    if (w <= 0 || h <= 0 || maxv != 255) return false;

    img.resize(static_cast<size_t>(w) * h);
    f.read(reinterpret_cast<char*>(img.data()), img.size());
    return f.good();
}

static bool save_pgm(const std::string& path, const std::vector<unsigned char>& img, int w, int h) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    f << "P5\n" << w << " " << h << "\n255\n";
    f.write(reinterpret_cast<const char*>(img.data()), img.size());
    return f.good();
}

__global__ void sobel_kernel(const unsigned char* in, unsigned char* out, int w, int h) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x <= 0 || y <= 0 || x >= w - 1 || y >= h - 1) return;

    int idx = y * w + x;

    int a00 = in[(y - 1) * w + (x - 1)];
    int a01 = in[(y - 1) * w + x];
    int a02 = in[(y - 1) * w + (x + 1)];
    int a10 = in[y * w + (x - 1)];
    int a12 = in[y * w + (x + 1)];
    int a20 = in[(y + 1) * w + (x - 1)];
    int a21 = in[(y + 1) * w + x];
    int a22 = in[(y + 1) * w + (x + 1)];

    int gx = -a00 + a02 - 2 * a10 + 2 * a12 - a20 + a22;
    int gy = -a00 - 2 * a01 - a02 + a20 + 2 * a21 + a22;

    int mag = (int)sqrtf((float)(gx * gx + gy * gy));
    if (mag > 255) mag = 255;
    out[idx] = (unsigned char)mag;
}

int main() {
    const std::string input_dir = "C:\\sobel-study\\gpu_input";
    const std::string output_dir = "C:\\sobel-study\\output\\gpu\\v5_cuda";
    const std::string results_dir = "C:\\sobel-study\\results\\gpu";
    const int runs = 30;
    const int warmup = 5;

    std::ofstream raw_csv(results_dir + "\\raw_results.csv");
    std::ofstream summary_csv(results_dir + "\\summary.csv");

    raw_csv << "image,resolution,run_id,h2d_ms,kernel_ms,d2h_ms,total_ms\n";
    summary_csv << "image,resolution,mean_h2d_ms,mean_kernel_ms,mean_d2h_ms,mean_total_ms,fps\n";

    std::vector<fs::path> files;
    for (const auto& e : fs::directory_iterator(input_dir)) {
        if (e.is_regular_file() && e.path().extension() == ".pgm") {
            files.push_back(e.path());
        }
    }
    std::sort(files.begin(), files.end());

    if (files.empty()) {
        std::cerr << "No .pgm files found in " << input_dir << "\n";
        return 1;
    }

    for (const auto& p : files) {
        std::vector<unsigned char> host_in, host_out;
        int w = 0, h = 0;

        if (!load_pgm(p.string(), host_in, w, h)) {
            std::cerr << "Failed to load " << p.filename().string() << "\n";
            continue;
        }

        host_out.assign((size_t)w * h, 0);

        unsigned char* d_in = nullptr;
        unsigned char* d_out = nullptr;
        size_t bytes = (size_t)w * h;

        cudaMalloc(&d_in, bytes);
        cudaMalloc(&d_out, bytes);
        cudaMemset(d_out, 0, bytes);

        dim3 block(16, 16);
        dim3 grid((w + block.x - 1) / block.x, (h + block.y - 1) / block.y);

        for (int i = 0; i < warmup; ++i) {
            cudaMemcpy(d_in, host_in.data(), bytes, cudaMemcpyHostToDevice);
            sobel_kernel<<<grid, block>>>(d_in, d_out, w, h);
            cudaDeviceSynchronize();
            cudaMemcpy(host_out.data(), d_out, bytes, cudaMemcpyDeviceToHost);
        }

        std::vector<double> h2d_times, kernel_times, d2h_times, total_times;

        for (int run = 1; run <= runs; ++run) {
            cudaEvent_t e0, e1, e2, e3;
            cudaEventCreate(&e0);
            cudaEventCreate(&e1);
            cudaEventCreate(&e2);
            cudaEventCreate(&e3);

            cudaEventRecord(e0);

            cudaMemcpy(d_in, host_in.data(), bytes, cudaMemcpyHostToDevice);
            cudaEventRecord(e1);

            sobel_kernel<<<grid, block>>>(d_in, d_out, w, h);
            cudaEventRecord(e2);

            cudaMemcpy(host_out.data(), d_out, bytes, cudaMemcpyDeviceToHost);
            cudaEventRecord(e3);
            cudaEventSynchronize(e3);

            float t_h2d = 0.0f, t_kernel = 0.0f, t_d2h = 0.0f, t_total = 0.0f;
            cudaEventElapsedTime(&t_h2d, e0, e1);
            cudaEventElapsedTime(&t_kernel, e1, e2);
            cudaEventElapsedTime(&t_d2h, e2, e3);
            cudaEventElapsedTime(&t_total, e0, e3);

            h2d_times.push_back(t_h2d);
            kernel_times.push_back(t_kernel);
            d2h_times.push_back(t_d2h);
            total_times.push_back(t_total);

            raw_csv << p.filename().string() << "," << w << "x" << h << "," << run << ","
                    << t_h2d << "," << t_kernel << "," << t_d2h << "," << t_total << "\n";

            cudaEventDestroy(e0);
            cudaEventDestroy(e1);
            cudaEventDestroy(e2);
            cudaEventDestroy(e3);
        }

        auto mean_of = [](const std::vector<double>& v) {
            double s = 0.0;
            for (double x : v) s += x;
            return s / (double)v.size();
        };

        double mean_h2d = mean_of(h2d_times);
        double mean_kernel = mean_of(kernel_times);
        double mean_d2h = mean_of(d2h_times);
        double mean_total = mean_of(total_times);
        double fps = (mean_total > 0.0) ? (1000.0 / mean_total) : 0.0;

        summary_csv << p.filename().string() << "," << w << "x" << h << ","
                    << mean_h2d << "," << mean_kernel << "," << mean_d2h << ","
                    << mean_total << "," << fps << "\n";

        save_pgm(output_dir + "\\" + p.filename().string(), host_out, w, h);

        std::cout << "[DONE] " << p.filename().string()
                  << " | mean_total=" << mean_total << " ms"
                  << " | mean_kernel=" << mean_kernel << " ms"
                  << " | fps=" << fps << "\n";

        cudaFree(d_in);
        cudaFree(d_out);
    }

    std::cout << "GPU benchmark finished.\n";
    return 0;
}
