//
// Created by User on 04/12/2023.
//

#include "CircularHough.h"



CircularHough::CircularHough(const int a_min, const int b_min, const int a_max, const int b_max, sycl::queue q, int data_size)
{
	
	//accumaltor = static_cast<int*>(sycl::malloc_shared(61 * 61 * 600 * sizeof(int), q)); 
	//a_values = static_cast<int*>(sycl::malloc_host(60 * sizeof(int), q)); 
	//b_values = static_cast<int*>(sycl::malloc_host(60 * sizeof(int), q));
	//bin_set = static_cast<int*>(sycl::malloc_shared(data_size * 60 * 60 * sizeof(int), q));
	//filtered_bins = static_cast<int*>(sycl::malloc_shared(2232600 * sizeof(int), q));

	/*
	for(int i = a_min; i < a_max; i+=3)
	{
		a_values[(i+a_max)/3] = i;
	}

	for(int i = b_min; i < b_max; i+=3)
        {
                b_values[(i+b_max)/3] = i;
        }
	*/




	//for(int i = 0; i < 61*61*600; i++){

	 //   accumaltor[i] = 0; 
	//}

	min_a = a_min;
	min_b = b_min;
	min_r = 0;
	max_a = a_max;
	max_b = b_max;
	size_a = a_max-a_min;
	size_b = b_max-b_min;
}

SYCL_EXTERNAL float CircularHough::r_ab(float x, float y, int a, int b)
{
    float r_squared = ((x-a) * (x-a)) + ((y-b) * (y-b));
    return r_squared;
}

SYCL_EXTERNAL int CircularHough::flattenIndex(int i, int j, int k, int dim1, int dim2, int dim3) {
    return (i * dim2 * dim3) + (j * dim3) + k;
}

std::vector<int> CircularHough::unflattenIndex(int flattenedIndex, int size) {
	std::vector<int> indices(3, 0);
    int sizeSquared = size * size;
    indices[0] = flattenedIndex / sizeSquared;                      
    int remainder = flattenedIndex % sizeSquared;
    indices[1] = remainder / size;                                       
    indices[2] = remainder % size;                                       
    return indices;
	
}

SYCL_EXTERNAL int CircularHough::mapToBin(float value, float min_val, float max_val, int num_bins)
{
    // Clamp the value to the range [min_val, max_val]
    value = std::min(std::max(value, min_val), max_val);
    // Map the value to the range [0, num_bins - 1]
    return static_cast<int>((value - min_val) / (max_val - min_val) * (num_bins-1));
}

SYCL_EXTERNAL float CircularHough::getBinCenter(int bin_index, float min_val, float max_val, int num_bins) {
    // Calculate the width of each bin
    float bin_width = (max_val - min_val) / num_bins;
    // Calculate the center of the specified bin
    return min_val + bin_width * (bin_index + 0.5);
}


SYCL_EXTERNAL void CircularHough::foward_transform(float x, float y, int glob_index) const
{
    //max_r = 170;
    int bin_a, bin_b, bin_r, index, idx;
    float r_squared, r;
    //[[intel::loop_coalesce(2)]]	
    for(int j = 0; j < 60; j++)
    {           
        for(int k = 0; k < 60; k++)
        {
            r_squared = r_ab(x, y, a_values[j], b_values[k]);
            r = std::sqrt(r_squared);
            bin_a = mapToBin(a_values[j], -90, 90, 61);
            bin_b = mapToBin(b_values[k], -90, 90, 61);
            bin_r = mapToBin(r, 0, 170, 600);
            index = flattenIndex(bin_a, bin_b, bin_r, 61, 61, 600);
	    //FtToThresholdPipe::write(index); 
	    //idx = glob_index * 60 * 60 + j * 60 + k;
	    //bin_set[idx] = flattenIndex(bin_a, bin_b, bin_r, 61, 61, 600);	
	    //accumaltor[index]++; 
        }
    }

}

std::vector<std::vector<float>> CircularHough::back_transform(std::vector<std::vector<float>> data, const std::vector<float>& track,
                                                               int size) const
{
    std::vector<std::vector<float>> track_recon;
    float r_squared, r, a_centre, b_centre, r_centre;
    int bin_a, bin_b, bin_r;

    for(int i = 0; i < size; i++)
    {
        bool assigned = false;
        for(int j = 0; j < 180; j++)
        {
            if(assigned)
            {
                break;
            }
            for(int k = 0; k < 180; k++)
            {
                r_squared = r_ab(data[i][0], data[i][1], a_values[j], b_values[k]);
                r = std::sqrt(r_squared);
                bin_a = mapToBin(a_values[j], -90, 90, 450);
                bin_b = mapToBin(b_values[k], -90, 90, 450);
                bin_r = mapToBin(r, 0, 170, 450);
                a_centre = getBinCenter(bin_a, -90, 90, 450);
                b_centre = getBinCenter(bin_b, -90, 90, 450);
                r_centre = getBinCenter(bin_r, 0, 170, 450);
			

                if(a_centre == track[0] and b_centre == track[1] and r_centre == track[2])
                {
                    std::vector<float> hit = {data[i][0], data[i][1]};
                    track_recon.emplace_back(hit);
                    assigned = true;
                    break;
                }
            }
        }
    }
    return track_recon;
}

[[nodiscard]] float CircularHough::get_max_r() const
{
    return max_r;
}

float CircularHough::get_num_rows() const
{
    return size_a * size_b * hits;
}
