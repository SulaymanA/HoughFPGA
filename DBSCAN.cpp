//
// Created by User on 18/01/2024.
//

#include "DBSCAN.h"
#include <cmath>
#include "iostream"

DBSCAN::DBSCAN(int minpts, float max_dist)
{
    epsilon = max_dist;
    num_pts = minpts;
}

bool DBSCAN::is_core_point(std::vector<float> point, std::vector<std::vector<float>> data) const
{
    int count = 0;
    bool is_core_point = false;
    for(int i =0; i < data.size(); i++)
    {
        //float r = pow(pow(point[0]-data[i][0], 2) + pow(point[1]-data[i][1], 2) + pow(point[2]-data[i][2], 2), 0.5);
        float r = std::sqrt(((point[0]-data[i][0]) * (point[0]-data[i][0])) +((point[1]-data[i][1]) * (point[1]-data[i][1]))
                             + ((point[2]-data[i][2]) * (point[2]-data[i][2])));
        if(r <= epsilon and r != 0)
        {
            count++;
        }
    }

    if(count >= num_pts)
    {
        is_core_point = true;
    }

    return is_core_point;
}

std::vector<std::vector<float>> DBSCAN::expand_point(std::vector<float> point, std::vector<std::vector<float>> data) const
{
    std::vector<std::vector<float>> density_reachable;
    for(int i = 0; i < data.size(); i++)
    {
        float r = std::sqrt(((point[0]-data[i][0]) * (point[0]-data[i][0])) +((point[1]-data[i][1]) * (point[1]-data[i][1]))
                + ((point[2]-data[i][2]) * (point[2]-data[i][2])));
        //float r = pow(pow(point[0]-data[i][0], 2) + pow(point[1]-data[i][1], 2) + pow(point[2]-data[i][2], 2), 0.5);
        if(r <= epsilon and r != 0)
        {
            density_reachable.emplace_back(data[i]);
        }
    }

    return density_reachable;
}

std::vector<std::vector<std::vector<float>>> DBSCAN::cluster(std::vector<std::vector<float>> data) const
{
    std::vector<std::vector<float>> core_points;
    std::vector<std::vector<std::vector<float>>> clusters;
    std::vector<std::vector<float>> undetermined;
    int cluster_number = 0;

    for(int i = 0; i < data.size(); i++)
    {
        //std::cout << i << std::endl;
        if(data[i].size() != 5 and is_core_point(data[i], data))
        {
            std::vector<std::vector<float>> new_cluster = {data[i]};
            clusters.emplace_back(new_cluster);
            data[i].emplace_back(cluster_number);

            int points_added = 1;
            //std::cout << "Here" << std::endl;
            while(points_added != 0)
            {
                points_added = 0;
                //std::cout << "Cluster size is: " << clusters[cluster_number].size() << std::endl;
                for(int j = 0; j < clusters[cluster_number].size(); j++)
                {
                    for(int k = 0; k < data.size(); k++)
                    {
//                        float r = pow(pow(clusters[cluster_number][j][0]-data[k][0], 2) +
//                                pow(clusters[cluster_number][j][1]-data[k][1], 2) + pow(clusters[cluster_number][j][2]-data[k][2], 2), 0.5);
                        float r = std::sqrt(((clusters[cluster_number][j][0]-data[k][0]) * (clusters[cluster_number][j][0]-data[k][0]))
                                + ((clusters[cluster_number][j][1]-data[k][1]) * (clusters[cluster_number][j][1]-data[k][1]))
                                + ((clusters[cluster_number][j][2]-data[k][2]) * (clusters[cluster_number][j][2]-data[k][2])));

                        if(data[k].size() != 5 and r <= epsilon and r !=0 and is_core_point(data[k], data))
                        {
                            //std::cout << "Adding density reachable point" << std::endl;
                            clusters[cluster_number].emplace_back(data[k]);
                            data[k].emplace_back(cluster_number);
                            points_added++;
                        }
                    }

                }
                //std::cout << "points added: " << points_added << std::endl;
            }
            cluster_number++;
            //std::cout << "after while" << std::endl;

        }
        else if(data[i].size() != 5 and !is_core_point(data[i], data))
        {
            undetermined.emplace_back(data[i]);
        }
    }

    //std::cout << clusters[0].size() + clusters[1].size() << std::endl;
    ///std::cout << "undetermined size: " << undetermined.size() << std::endl;
    for(int i = 0; i < undetermined.size(); i++)
    {
        for(int j = 0; j < data.size(); j++)
        {
//            float r = pow(pow(undetermined[i][0]-data[j][0], 2) +
//                           pow(undetermined[i][1]-data[j][1], 2) + pow(undetermined[i][2]-data[j][2], 2), 0.5);
            float r = std::sqrt(((undetermined[i][0]-data[j][0]) * (undetermined[i][0]-data[j][0]))
                    + ((undetermined[i][1]-data[j][1]) * (undetermined[i][1]-data[j][1]))
                    + ((undetermined[i][2]-data[j][2]) * (undetermined[i][2]-data[j][2])));
            if(r <= epsilon and data[j].size() == 5)
            {
                int cluster_id = data[j][4];
                clusters[cluster_id].emplace_back(undetermined[i]);
                break;
            }
        }
    }
    //std::cout << clusters[0].size() + clusters[1].size() << std::endl;

//    for(int i = 0; i < clusters.size(); i++)
//    {
//        std::cout << "Cluster: " << i << std::endl;
//        std::cout << "[";
//        for(int j = 0; j < clusters[i].size(); j++)
//        {
//            std::cout << "[";
//            for(int k = 0; k < 3; k++)
//            {
//                std::cout << clusters[i][j][k] << "," << " ";
//            }
//            std::cout << "]," << std::endl;
//        }
//        std::cout << "]" << std::endl;
//        std::cout << "---------------------"<< std::endl;
//    }

    //std::cout << undetermined.size() << std::endl;
    //std::cout << data.size() << std::endl;
    return clusters;

}