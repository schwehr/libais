// This file is deprecated.  I plan to make a new C++ decoder based on vdm.cc
// and to make C++ based TAG Block and USCG metadata handlers.
//
// Do not expect this file to build or really work.

#include "ais.h"

#include <fstream>
#include <iostream>
#include <cstring>
#include <sstream>
#include <string>

using std::stringstream;

// http://stackoverflow.com/questions/236129/c-how-to-split-a-string
vector<string> &split(const string &s, char delim, vector<string> &elems) {
  stringstream ss(s);
  string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

vector<string> split(const string &s, char delim) {
  vector<string> elems;
  return split(s, delim, elems);
}


int main(int argc,  char* argv[]) {
  assert(2 <= argc);
  std::ifstream infile(argv[1]);
  if (!infile.is_open()) {
    std::cerr << "Unable to open file: " << argv[1] << std::endl;
    exit(1);
  }

  int i = 0;
  string line;
  while (!infile.eof()) {
    i++;
    getline(infile, line);  // G++ problem with this and a string
    if (line.size() < 20) {continue;}
    if ('!' != line[0]  ||  'A' != line[1] ) {continue;}
    string line_str(line);
    vector<string> fields = split(line_str, ',');
    {
      if (fields.size() < 7) continue;
      if (fields[5].size() < 5) continue;
      if (fields[5][0] != '5') continue;
      Ais5 m5(fields[5].c_str(), 2);
      if (m5.had_error()) continue;
      std::cout << m5.mmsi << "," << m5.name << "," << m5.callsign << ","
                << m5.type_and_cargo << std::endl;
    }
  }
  return 0;
}
