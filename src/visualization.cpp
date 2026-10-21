#include "visualization.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace {
constexpr double kWidth = 900.0;
constexpr double kHeight = 650.0;
constexpr double kLeft = 85.0;
constexpr double kRight = 35.0;
constexpr double kTop = 60.0;
constexpr double kBottom = 85.0;
constexpr double kPlotWidth = kWidth - kLeft - kRight;
constexpr double kPlotHeight = kHeight - kTop - kBottom;
const char* kColors[] = {"#2563eb", "#dc2626", "#16a34a", "#9333ea", "#ea580c", "#0891b2",
                         "#db2777", "#4f46e5", "#65a30d", "#a16207"};
constexpr size_t kColorCount = sizeof(kColors) / sizeof(kColors[0]);

struct Bounds {
    double minX;
    double maxX;
    double minY;
    double maxY;
};

Bounds boundsFor(const vector<DataPoint>& data) {
    Bounds bounds{data.front().x, data.front().x, data.front().y, data.front().y};
    for (const DataPoint& point : data) {
        bounds.minX = min(bounds.minX, point.x);
        bounds.maxX = max(bounds.maxX, point.x);
        bounds.minY = min(bounds.minY, point.y);
        bounds.maxY = max(bounds.maxY, point.y);
    }
    const double xPadding = max((bounds.maxX - bounds.minX) * 0.08, 1.0);
    const double yPadding = max((bounds.maxY - bounds.minY) * 0.08, 1.0);
    bounds.minX -= xPadding;
    bounds.maxX += xPadding;
    bounds.minY -= yPadding;
    bounds.maxY += yPadding;
    return bounds;
}

double mapX(double value, const Bounds& bounds) {
    return kLeft + (value - bounds.minX) / (bounds.maxX - bounds.minX) * kPlotWidth;
}

double mapY(double value, const Bounds& bounds) {
    return kHeight - kBottom - (value - bounds.minY) / (bounds.maxY - bounds.minY) * kPlotHeight;
}

string number(double value) {
    ostringstream output;
    output << fixed << setprecision(2) << value;
    return output.str();
}

void writeGrid(ostream& output, const Bounds& bounds) {
    for (int tick = 0; tick <= 5; ++tick) {
        const double fraction = static_cast<double>(tick) / 5.0;
        const double x = kLeft + fraction * kPlotWidth;
        const double y = kHeight - kBottom - fraction * kPlotHeight;
        const double xValue = bounds.minX + fraction * (bounds.maxX - bounds.minX);
        const double yValue = bounds.minY + fraction * (bounds.maxY - bounds.minY);
        output << "<line class=\"grid\" x1=\"" << x << "\" y1=\"" << kTop << "\" x2=\"" << x
               << "\" y2=\"" << kHeight - kBottom << "\"/>\n"
               << "<line class=\"grid\" x1=\"" << kLeft << "\" y1=\"" << y << "\" x2=\""
               << kWidth - kRight << "\" y2=\"" << y << "\"/>\n"
               << "<text x=\"" << x << "\" y=\"" << kHeight - kBottom + 25
               << "\" text-anchor=\"middle\" font-size=\"12\">" << number(xValue) << "</text>\n"
               << "<text x=\"" << kLeft - 10 << "\" y=\"" << y + 4
               << "\" text-anchor=\"end\" font-size=\"12\">" << number(yValue) << "</text>\n";
    }
    output << "<line class=\"axis\" x1=\"" << kLeft << "\" y1=\"" << kHeight - kBottom << "\" x2=\""
           << kWidth - kRight << "\" y2=\"" << kHeight - kBottom << "\"/>\n"
           << "<line class=\"axis\" x1=\"" << kLeft << "\" y1=\"" << kTop << "\" x2=\"" << kLeft
           << "\" y2=\"" << kHeight - kBottom << "\"/>\n"
           << "<text x=\"" << (kLeft + kWidth - kRight) / 2 << "\" y=\"" << kHeight - 25
           << "\" text-anchor=\"middle\" font-size=\"15\">X</text>\n"
           << "<text x=\"25\" y=\"" << (kTop + kHeight - kBottom) / 2
           << "\" text-anchor=\"middle\" font-size=\"15\" transform=\"rotate(-90 25 "
           << (kTop + kHeight - kBottom) / 2 << ")\">Y</text>\n";
}

void writeRepresentative(ostream& output, const DataPoint& point, const Bounds& bounds,
                         const char* color, bool usesMedoids) {
    const double x = mapX(point.x, bounds);
    const double y = mapY(point.y, bounds);
    if (usesMedoids) {
        output << "<polygon points=\"" << x << ',' << y - 10 << ' ' << x + 10 << ',' << y << ' '
               << x << ',' << y + 10 << ' ' << x - 10 << ',' << y
               << "\" fill=\"white\" stroke=\"" << color << "\" stroke-width=\"3\"/>\n";
        return;
    }
    output << "<path d=\"M " << x - 9 << ' ' << y - 9 << " L " << x + 9 << ' ' << y + 9
           << " M " << x + 9 << ' ' << y - 9 << " L " << x - 9 << ' ' << y + 9
           << "\" stroke=\"" << color << "\" stroke-width=\"4\" stroke-linecap=\"round\"/>\n";
}

void writeCluster(ostream& output, const vector<DataPoint>& data, const Cluster& cluster,
                  size_t clusterIndex, const Bounds& bounds, bool usesMedoids) {
    const char* color = kColors[clusterIndex % kColorCount];
    for (size_t pointIndex : cluster.memberIndices) {
        const DataPoint& point = data[pointIndex];
        output << "<circle cx=\"" << mapX(point.x, bounds) << "\" cy=\"" << mapY(point.y, bounds)
               << "\" r=\"5\" fill=\"" << color
               << "\" fill-opacity=\"0.78\"><title>Point " << pointIndex + 1 << ": (" << point.x
               << ", " << point.y << ")</title></circle>\n";
    }
    writeRepresentative(output, cluster.centroid, bounds, color, usesMedoids);
    output << "<text x=\"" << kWidth - kRight - 130 << "\" y=\"" << kTop + 20 + clusterIndex * 22
           << "\" font-size=\"13\" fill=\"" << color << "\">● Cluster " << clusterIndex + 1
           << "</text>\n";
}
}  // namespace

void writeClusterSvg(const string& filename,
                     const vector<DataPoint>& data,
                     const vector<Cluster>& clusters,
                     bool usesMedoids) {
    if (data.empty()) throw invalid_argument("cannot draw an empty data set");
    ofstream output(filename);
    if (!output) throw runtime_error("could not write SVG file: " + filename);

    const Bounds bounds = boundsFor(data);
    output << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << kWidth
           << "\" height=\"" << kHeight << "\" viewBox=\"0 0 " << kWidth << ' ' << kHeight
           << "\">\n<style>text{font-family:Arial,sans-serif;fill:#1f2937}.grid{stroke:#e5e7eb;stroke-width:1}.axis{stroke:#374151;stroke-width:1.5}</style>\n"
           << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n"
           << "<text x=\"" << kLeft << "\" y=\"32\" font-size=\"22\" font-weight=\"bold\">"
           << (usesMedoids ? "K-Medoids" : "K-Means") << " clustering</text>\n";

    writeGrid(output, bounds);
    for (size_t index = 0; index < clusters.size(); ++index) {
        writeCluster(output, data, clusters[index], index, bounds, usesMedoids);
    }
    output << "<text x=\"" << kLeft << "\" y=\"" << kHeight - 8
           << "\" font-size=\"12\">Markers: points = circles; "
           << (usesMedoids ? "medoids = diamonds" : "centroids = crosses") << "</text>\n</svg>\n";
}
