//
// Created by User on 18/01/2024.
//
#include <vector>

#ifndef TEST_C___DBSCAN_H
#define TEST_C___DBSCAN_H


class DBSCAN
{
public:
    DBSCAN(int minpts, float max_dist);
    int num_pts;
    float epsilon;
    bool is_core_point(std::vector<float> point, std::vector<std::vector<float>> data) const;
    std::vector<std::vector<float>> expand_point(std::vector<float> point, std::vector<std::vector<float>> data) const;
    std::vector<std::vector<std::vector<float>>> cluster(std::vector<std::vector<float>> data) const;
};


#endif //TEST_C___DBSCAN_H
