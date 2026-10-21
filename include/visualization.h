#pragma once

#include "kmeans.h"

#include <string>
#include <vector>

// Writes a self-contained SVG scatter plot. K-Means representatives are drawn
// as crosses; K-Medoids representatives are drawn as diamonds.
void writeClusterSvg(const std::string& filename,
                     const std::vector<DataPoint>& data,
                     const std::vector<Cluster>& clusters,
                     bool usesMedoids);
