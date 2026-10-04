#include "analytics.h"
#include "initializers.h"
#include "input.h"
#include "kmeans.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

int main() {
    try {
        cout << "MiniCluster - 2D K-Means clustering\n";
        cout << "===================================\n";

        // 1. Get the data and the settings from the user.
        const vector<DataPoint> data = readPointsFromUser();
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

        // 3. Run K-Means.
        KMeans model(clusterCount, *initializer, 200, 1e-4, seed);
        model.fit(data);

        // 4. Analyse the result.
        const auto summaries = summarizeClusters(data, model.clusters());
        const auto outliers = findOutliers(data, model.clusters());

        // 5. Print the report.
        cout << fixed << setprecision(3);
        cout << "\nMiniCluster report\n"
             << "==================\n"
             << "Points: " << data.size() << " | K: " << clusterCount
             << " | initializer: " << initializerName << " | seed: " << seed << "\n"
             << "Iterations: " << model.iterations()
             << " | converged: " << (model.converged() ? "yes" : "no")
             << " | inertia: " << model.inertia()
             << " | silhouette: " << silhouetteScore(data, model.clusters()) << "\n\n";

        model.printResults();

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
