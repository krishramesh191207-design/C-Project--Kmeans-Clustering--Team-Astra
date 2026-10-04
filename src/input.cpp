#include "input.h"

#include <algorithm>
#include <cctype>
#include <iostream>
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
