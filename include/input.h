#pragma once

#include "data_point.h"

#include <string>
#include <vector>

// Asks the user how many points they want, then asks for "x y" for each point.
// Bad input is rejected and the user is asked again.
vector<DataPoint> readPointsFromUser();

// Reads a CSV file whose first two columns are numeric x and y values. A
// single header row is allowed and blank lines are ignored.
vector<DataPoint> readPointsFromCsv(const string& filename);

// Shows the accepted CSV structure before asking for a CSV file path.
void printCsvRequirements();

// Lets the user choose either keyboard input or a CSV file.
vector<DataPoint> readPoints();

// Asks for a whole number between minimum and maximum (inclusive).
// Keeps asking until the user types a valid number.
long long readInteger(const string& prompt, long long minimum, long long maximum);

// Asks the user to type one of the allowed words (for example "random" or "kmeans++").
// Keeps asking until the user types one of them.
string readChoice(const string& prompt, const vector<string>& allowedChoices);
