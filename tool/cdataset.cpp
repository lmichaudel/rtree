#include "json.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using json = nlohmann::json;
using namespace std;

void writeBinaryFile(const string& filename,
                     const vector<pair<double, double>>& coordinates) {
  ofstream outfile(filename, ios::binary);
  size_t size = coordinates.size();
  outfile.write(reinterpret_cast<const char*>(&size), sizeof(size));
  outfile.write(reinterpret_cast<const char*>(coordinates.data()),
                size * sizeof(pair<double, double>));
}

static double MIN_LON = -125.0, MAX_LON = -66.0;
static double MIN_LAT = 24.0, MAX_LAT = 50.0;

bool is_valid(pair<double, double> coordinate) {
  if (coordinate.first < MIN_LON || coordinate.first > MAX_LON ||
      coordinate.second < MIN_LAT || coordinate.second > MAX_LAT)
    return false;

  return true;
}

int main(int argc, char* argv[]) {
  assert(argc == 3);
  std::ifstream infile(argv[1]);

  if (!infile) {
    std::cerr << "Failed to open input or output file.\n";
    return 1;
  }

  std::vector<std::pair<double, double>> coordinates;
  std::string line;

  int c = 0;
  while (std::getline(infile, line)) {
    try {
      nlohmann::json j = nlohmann::json::parse(line);
      const auto& coords = j["geometry"]["coordinates"];
      double lon = coords[0];
      double lat = coords[1];

      std::pair<double, double> coord = {lon, lat};
      if (is_valid(coord)) {
        c++;
        coordinates.push_back(coord);
      }
    } catch (const std::exception& e) {
      std::cerr << "Failed to parse line: " << e.what() << "\n";
    }
  }
  infile.close();

  // Shuffle the coordinates
  std::random_device rd;
  std::mt19937 gen(rd());
  std::shuffle(coordinates.begin(), coordinates.end(), gen);

  // Write to output binary file
  writeBinaryFile(argv[2], coordinates);

  std::cout << "Finished writing " << c << " coordinates to binary file.\n";
  return 0;
}