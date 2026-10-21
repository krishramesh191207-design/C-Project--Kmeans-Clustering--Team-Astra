#include "kmedoids.h"
#include "cluster_style.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace std;

namespace {
bool samePoint(const DataPoint& first, const DataPoint& second) {
    return first.x == second.x && first.y == second.y;
}

size_t findUnusedMatchingPoint(const vector<DataPoint>& data,
                               const vector<size_t>& usedIndices,
                               const DataPoint& target) {
    for (size_t index = 0; index < data.size(); ++index) {
        if (samePoint(data[index], target) &&
            find(usedIndices.begin(), usedIndices.end(), index) == usedIndices.end()) {
            return index;
        }
    }
    throw logic_error("initializer returned a medoid outside the input data");
}

double totalDistanceToMembers(size_t candidate,
                              const vector<size_t>& members,
                              const vector<DataPoint>& data) {
    double total = 0.0;
    for (size_t member : members) total += distance(data[candidate], data[member]);
    return total;
}
}  // namespace

KMedoids::KMedoids(size_t clusterCount,
                   const CentroidInitializer& initializer,
                   size_t maxIterations,
                   unsigned int seed)
    : clusterCount_(clusterCount), initializer_(initializer), maxIterations_(maxIterations),
      generator_(seed) {
    if (clusterCount == 0 || maxIterations == 0) {
        throw invalid_argument("invalid K-Medoids configuration");
    }
}

void KMedoids::fit(const vector<DataPoint>& data) {
    if (data.empty() || clusterCount_ > data.size()) {
        throw invalid_argument("data must contain at least as many points as clusters");
    }

    clusters_.clear();
    medoidIndices_.clear();
    const vector<DataPoint> startingMedoids = initializer_.initialize(data, clusterCount_, generator_);
    for (const DataPoint& medoid : startingMedoids) {
        const size_t index = findUnusedMatchingPoint(data, medoidIndices_, medoid);
        medoidIndices_.push_back(index);
        clusters_.push_back({data[index], {}});
    }

    converged_ = false;
    iterations_ = 0;
    for (; iterations_ < maxIterations_; ++iterations_) {
        assignPoints(data);
        if (updateMedoids(data)) {
            converged_ = true;
            ++iterations_;
            break;
        }
    }
    assignPoints(data);
    cost_ = 0.0;
    for (const Cluster& cluster : clusters_) {
        for (size_t pointIndex : cluster.memberIndices) {
            cost_ += distance(data[pointIndex], cluster.centroid);
        }
    }
}

void KMedoids::assignPoints(const vector<DataPoint>& data) {
    for (Cluster& cluster : clusters_) cluster.memberIndices.clear();
    for (size_t pointIndex = 0; pointIndex < data.size(); ++pointIndex) {
        size_t nearestCluster = 0;
        double nearestDistance = distance(data[pointIndex], clusters_.front().centroid);
        for (size_t clusterIndex = 1; clusterIndex < clusters_.size(); ++clusterIndex) {
            const double candidate = distance(data[pointIndex], clusters_[clusterIndex].centroid);
            if (candidate < nearestDistance) {
                nearestDistance = candidate;
                nearestCluster = clusterIndex;
            }
        }
        clusters_[nearestCluster].memberIndices.push_back(pointIndex);
    }
}

bool KMedoids::updateMedoids(const vector<DataPoint>& data) {
    bool unchanged = true;
    for (size_t clusterIndex = 0; clusterIndex < clusters_.size(); ++clusterIndex) {
        const vector<size_t>& members = clusters_[clusterIndex].memberIndices;
        size_t bestMedoid = medoidIndices_[clusterIndex];
        if (members.empty()) {
            bestMedoid = farthestPointFromMedoids(data);
        } else {
            double bestCost = numeric_limits<double>::infinity();
            for (size_t candidate : members) {
                const double candidateCost = totalDistanceToMembers(candidate, members, data);
                if (candidateCost < bestCost) {
                    bestCost = candidateCost;
                    bestMedoid = candidate;
                }
            }
        }
        if (bestMedoid != medoidIndices_[clusterIndex]) unchanged = false;
        medoidIndices_[clusterIndex] = bestMedoid;
        clusters_[clusterIndex].centroid = data[bestMedoid];
    }
    return unchanged;
}

size_t KMedoids::farthestPointFromMedoids(const vector<DataPoint>& data) const {
    size_t farthestPoint = 0;
    double farthestDistance = -1.0;
    for (size_t pointIndex = 0; pointIndex < data.size(); ++pointIndex) {
        double nearestDistance = numeric_limits<double>::infinity();
        for (const Cluster& cluster : clusters_) {
            nearestDistance = min(nearestDistance, distance(data[pointIndex], cluster.centroid));
        }
        if (nearestDistance > farthestDistance) {
            farthestDistance = nearestDistance;
            farthestPoint = pointIndex;
        }
    }
    return farthestPoint;
}

const vector<Cluster>& KMedoids::clusters() const noexcept { return clusters_; }
size_t KMedoids::iterations() const noexcept { return iterations_; }
bool KMedoids::converged() const noexcept { return converged_; }
double KMedoids::cost() const noexcept { return cost_; }

void KMedoids::printResults() const {
    cout << fixed << setprecision(3);
    for (size_t index = 0; index < clusters_.size(); ++index) {
        const Cluster& cluster = clusters_[index];
        cout << "Cluster " << index + 1 << " [" << clusterColorName(index) << "]: medoid (" << cluster.centroid.x << ", "
             << cluster.centroid.y << "), members: " << cluster.memberIndices.size() << '\n';
    }
}
