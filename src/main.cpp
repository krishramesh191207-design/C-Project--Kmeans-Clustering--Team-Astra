#include "analytics.h"
#include "initializers.h"
#include "input.h"
#include "kmeans.h"
#include "kmedoids.h"
#include "visualization.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

int main() {
    try {
        cout << "MiniCluster - 2D clustering\n";
        cout << "===================================\n";

        // 1. Choose the algorithm and source, then get the data and settings.
        const string algorithm = readChoice("Clustering method (kmeans / kmedoids): ",
                                            {"kmeans", "kmedoids"});
        const vector<DataPoint> data = readPoints();
        const size_t clusterCount = static_cast<size_t>(
            readInteger("Number of clusters K (1-" + to_string(data.size()) + "): ",
                        1, static_cast<long long>(data.size())));
        const string initializerName = readChoice("Initializer (random / kmeans++): ",
                                                  {"random", "kmeans++"});
        const unsigned int seed = static_cast<unsigned int>(
            readInteger("Random seed (0-1000000): ", 0, 1000000));

        // 2. Pick the initializer. We hold it through the base-class pointer,
        //    so the rest of the program does not care which one it is.
        unique_ptr<CentroidInitializer> initializer;
        if (initializerName == "random") {
            initializer.reset(new RandomInitializer());
        } else {
            initializer.reset(new KMeansPlusPlusInitializer());
        }

        // 3. Run the selected clustering algorithm.
        KMeans kmeans(clusterCount, *initializer, 200, 1e-4, seed);
        KMedoids kmedoids(clusterCount, *initializer, 200, seed);
        if (algorithm == "kmeans") kmeans.fit(data);
        else kmedoids.fit(data);
        const vector<Cluster>& clusters = algorithm == "kmeans" ? kmeans.clusters() : kmedoids.clusters();

        // 4. Analyse the result.
        const auto summaries = summarizeClusters(data, clusters);
        const auto outliers = findOutliers(data, clusters);

        // 5. Print the report.
        cout << fixed << setprecision(3);
        cout << "\nMiniCluster report\n"
             << "==================\n"
             << "Points: " << data.size() << " | K: " << clusterCount
             << " | algorithm: " << algorithm << " | initializer: " << initializerName
             << " | seed: " << seed << "\n"
             << "Iterations: " << (algorithm == "kmeans" ? kmeans.iterations() : kmedoids.iterations())
             << " | converged: " << ((algorithm == "kmeans" ? kmeans.converged() : kmedoids.converged()) ? "yes" : "no")
             << " | " << (algorithm == "kmeans" ? "inertia: " : "medoid cost: ")
             << (algorithm == "kmeans" ? kmeans.inertia() : kmedoids.cost())
             << " | silhouette: " << silhouetteScore(data, clusters) << "\n\n";

        if (algorithm == "kmeans") kmeans.printResults();
        else kmedoids.printResults();

        const string plotFilename = "clusters.svg";
        writeClusterSvg(plotFilename, data, clusters, algorithm == "kmedoids");
        cout << "\nVisualisation written to " << plotFilename << '\n';

        cout << "\nCompactness diagnostics\n";
        for (const auto& summary : summaries) {
            cout << "  C" << summary.index + 1 << ": avg radius " << summary.averageDistance
                 << ", max radius " << summary.maximumDistance << '\n';
        }

        cout << "Outlier candidates (z >= 2.5): ";
        if (outliers.empty()) {
            cout << "none\n";
        } else {
            for (size_t index : outliers) {
                cout << '#' << index << ' ';
            }
            cout << '\n';
        }
    } catch (const exception& error) {
        cerr << "\nError: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
