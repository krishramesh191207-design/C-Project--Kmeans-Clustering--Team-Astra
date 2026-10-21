#include "input.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace {
// Shows the prompt and reads one whole line typed by the user.
// If the input stream is closed (Ctrl+D / Ctrl+Z), we stop with an error
// instead of asking forever.
string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) {
        throw runtime_error("input ended before all values were entered");
    }
    return line;
}

string toLower(string text) {
    transform(text.begin(), text.end(), text.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return text;
}

bool parseNumber(const string& text, double& value) {
    stringstream input(text);
    string leftover;
    return input >> value && !(input >> leftover);
}

vector<string> splitCsvLine(const string& line) {
    vector<string> fields;
    string field;
    bool quoted = false;
    for (size_t index = 0; index < line.size(); ++index) {
        const char character = line[index];
        if (character == '"') {
            if (quoted && index + 1 < line.size() && line[index + 1] == '"') {
                field += character;
                ++index;
            } else {
                quoted = !quoted;
            }
        } else if (character == ',' && !quoted) {
            fields.push_back(field);
            field.clear();
        } else {
            field += character;
        }
    }
    if (quoted) throw runtime_error("CSV contains an unterminated quoted field");
    fields.push_back(field);
    return fields;
}
}  // namespace

long long readInteger(const string& prompt, long long minimum, long long maximum) {
    while (true) {
        stringstream line(readLine(prompt));
        long long value = 0;
        string leftover;
        // Valid only if a whole number was read AND nothing else is left on the line.
        // This rejects words ("abc"), decimals ("3.5") and empty lines.
        if (line >> value && !(line >> leftover)) {
            if (value >= minimum && value <= maximum) {
                return value;
            }
        }
        cout << "  Please enter a whole number from " << minimum << " to " << maximum << ".\n";
    }
}

string readChoice(const string& prompt, const vector<string>& allowedChoices) {
    while (true) {
        stringstream line(readLine(prompt));
        string choice;
        string leftover;
        if (line >> choice && !(line >> leftover)) {
            choice = toLower(choice);
            if (find(allowedChoices.begin(), allowedChoices.end(), choice) != allowedChoices.end()) {
                return choice;
            }
        }
        cout << "  Please type one of:";
        for (const string& allowed : allowedChoices) {
            cout << ' ' << allowed;
        }
        cout << '\n';
    }
}

vector<DataPoint> readPointsFromUser() {
    const long long count = readInteger("How many points? (1-1000): ", 1, 1000);

    vector<DataPoint> data;
    data.reserve(static_cast<size_t>(count));
    for (long long index = 1; index <= count; ++index) {
        while (true) {
            string text = readLine("Point " + to_string(index) + " (income score): ");
            replace(text.begin(), text.end(), ',', ' ');  // allow "45,60" as well as "45 60"
            stringstream line(text);
            DataPoint point;
            string leftover;
            if (line >> point.x >> point.y && !(line >> leftover)) {
                data.push_back(point);
                break;
            }
            cout << "  Please type two numbers separated by a space, for example: 45 60\n";
        }
    }
    return data;
}

vector<DataPoint> readPointsFromCsv(const string& filename) {
    ifstream file(filename);
    if (!file) throw runtime_error("could not open CSV file: " + filename);

    vector<DataPoint> data;
    string line;
    size_t lineNumber = 0;
    bool firstDataOrHeader = true;
    while (getline(file, line)) {
        ++lineNumber;
        if (line.find_first_not_of(" \t\r\n") == string::npos) continue;
        const vector<string> fields = splitCsvLine(line);
        double x = 0.0;
        double y = 0.0;
        if (fields.size() < 2 || !parseNumber(fields[0], x) || !parseNumber(fields[1], y)) {
            if (firstDataOrHeader) {
                firstDataOrHeader = false;  // Optional column names, e.g. income,score.
                continue;
            }
            throw runtime_error("invalid numeric x,y values on CSV line " + to_string(lineNumber));
        }
        firstDataOrHeader = false;
        if (!std::isfinite(x) || !std::isfinite(y)) {
            throw runtime_error("CSV line " + to_string(lineNumber) + " contains a non-finite number");
        }
        data.push_back({x, y});
        if (data.size() > 1000) throw runtime_error("CSV files are limited to 1000 points");
    }
    if (data.empty()) throw runtime_error("CSV file contains no numeric x,y rows");
    return data;
}

void printCsvRequirements() {
    cout << "\nCSV requirements:\n"
         << "  - First two columns: numeric X,Y values (for example: 42,61)\n"
         << "  - Optional header on the first non-blank row\n"
         << "  - Blank rows and extra columns are allowed\n"
         << "  - Use finite numbers only; maximum 1000 data rows\n"
         << "  Example: annual_income_k,spending_score\n\n";
}

vector<DataPoint> readPoints() {
    const string source = readChoice("Input method (custom / csv): ", {"custom", "csv"});
    if (source == "custom") return readPointsFromUser();
    printCsvRequirements();
    return readPointsFromCsv(readLine("CSV file path (first two columns are x,y): "));
}
