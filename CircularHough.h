//
// Created by User on 04/12/2023.
//
#include "iostream"
#include "vector"
#include "cmath"
#include <sycl/sycl.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>
//#include "set"

#ifndef TEST_C___CIRCULARHOUGH_H
#define TEST_C___CIRCULARHOUGH_H



class CircularHough{
public:
    float max_r{};
    CircularHough(const int a_min, const int b_min, const int a_max, const int b_max, sycl::queue q, int data_size);

    //std::set<std::vector<int>> indicies;
    int min_a, max_a, min_b, max_b, min_r;
    int* a_values;
    int* b_values;
    int* bin_set;
    int* filtered_bins; 
    //float (*ptr)[3];
    float** ptr{};
    int size_a;
    int size_b;
    int hits{};
    //int (*accumaltor)[600][600];
	int* accumaltor; 
    SYCL_EXTERNAL static float r_ab(float x, float y, int a, int b);
    //float(*foward_transform(float(*data)[2]))[3];
    SYCL_EXTERNAL void foward_transform(float x, float y, int glob_index) const;
    float get_num_rows() const;
    std::vector<std::vector<float>> back_transform(std::vector<std::vector<float>> data, const std::vector<float>& track,
                                                    int size) const;
    SYCL_EXTERNAL static int mapToBin(float value, float min_val, float max_val, int num_bins);
    SYCL_EXTERNAL static float getBinCenter(int bin_index, float min_val, float max_val, int num_bins);
    // std::set<std::vector<int>> getIndicies() const;
    std::vector<std::vector<float>> getmostfreq(int cutoff);
    float get_max_r() const;

    SYCL_EXTERNAL static int flattenIndex(int i, int j, int k, int dim1, int dim2, int dim3);
	static std::vector<int> unflattenIndex(int flattenedIndex, int size); 
};


#endif //TEST_C___CIRCULARHOUGH_H
