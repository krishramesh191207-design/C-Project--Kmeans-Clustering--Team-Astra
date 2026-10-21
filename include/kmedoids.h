#pragma once

#include "initializers.h"
#include "kmeans.h"

#include <cstddef>
#include <random>
#include <vector>

// K-Medoids keeps each cluster representative as an actual input point.
// The Cluster::centroid member is used to expose that representative to the
// existing analytics functions.
class KMedoids {
public:
    KMedoids(size_t clusterCount,
             const CentroidInitializer& initializer,
             size_t maxIterations = 100,
             unsigned int seed = std::random_device{}());

    void fit(const std::vector<DataPoint>& data);

    const std::vector<Cluster>& clusters() const noexcept;
    size_t iterations() const noexcept;
    bool converged() const noexcept;
    double cost() const noexcept;
    void printResults() const;

private:
    void assignPoints(const std::vector<DataPoint>& data);
    bool updateMedoids(const std::vector<DataPoint>& data);
    size_t farthestPointFromMedoids(const std::vector<DataPoint>& data) const;

    size_t clusterCount_;
    const CentroidInitializer& initializer_;
    size_t maxIterations_;
    std::mt19937 generator_;
    std::vector<Cluster> clusters_;
    std::vector<size_t> medoidIndices_;
    size_t iterations_{};
    bool converged_{};
    double cost_{};
};
