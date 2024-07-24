#include <iostream>
#include <fstream>
#include <sstream>
#include "CircularHough.h"
#include <chrono>
#include "DBSCAN.h"
#include "string"
#include <stdio.h>
#include "sycl/ext/intel/fpga_extensions.hpp"
//#include "oneapi/mkl.hpp"
//#include <sycl/sycl.hpp>

std::vector<std::vector<float>> read_csv(const std::string& fname)
{
    std::vector<std::vector<float>> data;
    std::vector<float> row;
    std::string line, word;

    std::fstream file (fname, std::ios::in);
    if(file.is_open())
    {
        while(getline(file, line))
        {
            row.clear();

            std::stringstream str(line);

            while(getline(str, word, ','))
                row.emplace_back(std::stof(word));
            data.emplace_back(row);
        }
    }
    else
    {
        std::cout<<"Could not open the file\n";
    }
    return data;
}

void writeToCSV(const std::string& filename, const std::vector<std::vector<float>>& data) {
    std::ofstream csvFile(filename);

    if (!csvFile.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    for (const auto& row : data) {
        for (const auto& cell : row) {
            csvFile << cell << ",";
        }
        csvFile << "\n";
    }

    csvFile.close();
}

int main() {
	
	#if FPGA_SIMULATOR
		auto selector = sycl::ext::intel::fpga_simulator_selector_v;
	#elif FPGA_HARDWARE
		auto selector = sycl::ext::intel::fpga_selector_v;
	#else //#if  FPGA_EMULATOR
		auto selector = sycl::ext::intel::fpga_emulator_selector_v;
	#endif
	std::vector<std::vector<float>> momentum_dist; 
	std::vector<std::vector<float>> delta_hits;
	std::vector<std::vector<float>> recon_no_tracks;
	std::vector<std::vector<float>> counts;  	
        auto t1 = std::chrono::high_resolution_clock::now(); 
	sycl::queue q1(selector);
	auto device = q1.get_device();

	using FtToThresholdPipe = sycl::ext::intel::pipe< // Defined in the SYCL headers.
   	 class FtID,                // An identifier for the pipe.
	 int,                                         // The type of data in the pipe.
	 4>;                                 // The capacity of the pipe.


    	using ThresholdToBtPipe = sycl::ext::intel::pipe< 
	 class BtID,                
	 int,
	 4>;                            

	std::vector<std::vector<std::vector<float>>> events;  
	for(int i = 0; i < 100; i++){
		std::string fname = "data/anadata" + std::to_string(i) + ".csv";
		events.emplace_back(read_csv(fname)); 
	}

	for(int l = 0; l < 100; l++) {
		std::cout << l << std::endl;
		//std::string fname = "data/clustered_json" + std::to_string(l) + ".csv";
		std::vector<std::vector<float>> event = events[l];

		//Flatten the data structure
		std::vector<float> flattened_x;
		std::vector<float> flattened_y;
		for(int j = 0; j < event.size(); j++) {
			
			flattened_x.emplace_back(event[j][0]); 
			flattened_y.emplace_back(event[j][1]);
		}
		
		std::vector<int> truth_vect(30000*flattened_x.size(), -1);
                sycl::buffer<int, 1> truth_buffer(truth_vect.data(), sycl::range<1>(truth_vect.size()));
	
		sycl::buffer<float, 1> data_x_buffer0(flattened_x.data(), sycl::range<1>(flattened_x.size()));
		sycl::buffer<float, 1> data_y_buffer0(flattened_y.data(), sycl::range<1>(flattened_y.size()));
		int data_size = flattened_x.size();

		auto e1 = q1.submit([&](sycl::handler& h) {
			auto accessor_x_0 = data_x_buffer0.get_access(h, sycl::read_only);
			auto accessor_y_0 = data_y_buffer0.get_access(h, sycl::read_only);
			int size = data_x_buffer0.size();
			h.single_task<class foward_transform>([=]()[[intel::kernel_args_restrict]] { // Do forward transform, loop over 60 a and b values.
				//TODO Make this more efficient 
				int a_array[60];
				int b_array[60];
				int idx = 0; 
				for(int i = -90; i < 90; i+=3)
				{                       
					a_array[idx] = i;
					b_array[idx] = i;
					idx++; 
				}
				


				for(int i = 0; i < size; i++){
					//circularhough0.foward_transform(accessor_x_0[i], accessor_y_0[i], i);
					int bin_a, bin_b, bin_r, index, idx;
					float r_squared, r;
					
					for(int j = 0; j < 60; j++)
					{
						for(int k = 0; k < 60; k++)
						{
							r_squared = CircularHough::r_ab(accessor_x_0[i], accessor_y_0[i], a_array[j], b_array[k]);
							r = std::sqrt(r_squared);
							bin_a = CircularHough::mapToBin(a_array[j], -90, 90, 61);
							bin_b = CircularHough::mapToBin(b_array[k], -90, 90, 61);
							bin_r = CircularHough::mapToBin(r, 0, 170, 600);
							index = CircularHough::flattenIndex(bin_a, bin_b, bin_r, 61, 61, 600);
							FtToThresholdPipe::write(index); // Push the index in the accumulator that needs to be updated to the pipe
						}
					}
				}
			}); 

		});
		
		
		auto e2 = q1.submit([&](sycl::handler& h) {
				int iter = data_size * 60 * 60;
				//sycl::printf("here"); 

                                h.single_task<class threshold>([=]()[[intel::kernel_args_restrict]] {
					char accumulator[61 * 61 * 600];

					for(int i = 0; i < 61*61*600; i++){

                                        	accumulator[i] = 0;

                               		 }	

					for(int i = 0; i < iter+1; i++){
						int index = -1; 
						if(i < iter){
						index = FtToThresholdPipe::read();	// read from pipe - forward transfrom passes bins to update in accumulator
						accumulator[index]++; 
						}

						
						if(index == -1 || accumulator[index] == 96){ // check if bin has reached cutoff
					
							ThresholdToBtPipe::write(index); //write this bin index to next pipe 
						}

					}
                                });

                });
		
		q1.submit([&](sycl::handler& h) {
			auto accessor_x_0 = data_x_buffer0.get_access(h, sycl::read_only);
			auto accessor_y_0 = data_y_buffer0.get_access(h, sycl::read_only);
			auto accessor_tb = truth_buffer.get_access(h, sycl::write_only);
			int size = data_x_buffer0.size();

			h.single_task<class back_transform>([=]()[[intel::kernel_args_restrict]] {
				int a_array[60];
                                int b_array[60];

                                int idx = 0;
                                for(int i = -90; i < 90; i+=3)
                                {               
                                        a_array[idx] = i;
                                        b_array[idx] = i;
                                        idx++;
                                }
	
				int j = 0;
			       	int filtered_bin = -1; 
				do{
					
					float r_squared, r;
                	                int bin_a, bin_b, bin_r, index, truth;
					filtered_bin = ThresholdToBtPipe::read();
					if(filtered_bin != -1){

						for(int i = 0; i < size; i++){
							truth = 0; 	
							bin_a = filtered_bin / (61 * 600);
							bin_b = (filtered_bin % (61 * 600)) / 600;
							r_squared = CircularHough::r_ab(accessor_x_0[i], accessor_y_0[i], a_array[bin_a], b_array[bin_b]);
							r = std::sqrt(r_squared);
							bin_r = CircularHough::mapToBin(r, 0, 170, 600);
							index = CircularHough::flattenIndex(bin_a, bin_b, bin_r, 61, 61, 600);

							if(index == filtered_bin){
								truth = 1;
							}

							accessor_tb[j * size + i] = truth; 
						}
						j++; 
					}

				}while(filtered_bin != -1);
				
				
			});

		});
               

                q1.wait();
				
		sycl::host_accessor<int, 1, sycl::access::mode::read> h_acc(truth_buffer); 
		std::vector<std::vector<float>> candidate_tracks; 
		for(int i = 0; i < 30000; i++){
			if(h_acc[i * flattened_x.size()] == -1){
			
				break; 
			}
			int count = 0; 
			std::vector<std::vector<float>> track; 
			for(int j = 0; j < flattened_x.size(); j++){
				//std::cout << h_acc[i * flattened_x.size() + j] << " ";
				//count = count + truth_buffer[i * N + j];
				if(h_acc[i * flattened_x.size() + j] == 1){
					//std::cout << "here" << std::endl; 
					std::vector<float> hit = {flattened_x[j], flattened_y[j], static_cast<float>(i)};
					candidate_tracks.emplace_back(hit);
				}
			}
			std::string basestring = "data/track";
                        std::string filename = basestring + std::to_string(i) + ".csv";
		}
		std::string filename = "data/candidate_hits" + std::to_string(l) + ".csv";
		writeToCSV(filename, candidate_tracks);
		int diff = flattened_x.size() - candidate_tracks.size();
		//std::cout << diff << std::endl; 
		std::vector<float> delta = {static_cast<float>(diff)};
	        delta_hits.emplace_back(delta); 	
		
	}
	//std::cout << "Here" << std::endl;	
	//writeToCSV("delta_hits.csv", delta_hits); 
	//writeToCSV("recon_number_tracks.csv", recon_no_tracks);
	//writeToCSV("freq_dist.csv", counts);
	//writeToCSV("momentum_dist.csv", momentum_dist); 	
	auto t2 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
	std::cout << "Time for 100 Events in ms: " << duration.count() << std::endl;

	
    return 0;
}

// best: Bins = 600 bins,
// cuttoff = 30
// curvature filitering:
// float delta_final = track[track.size()-5][1] - track[track.size()-2][1];
// float delta_init = track[1][1] - track[4][1];

//DBSCAN = (6, 3)

