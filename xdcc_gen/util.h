#pragma once

#include <vector>

#define eprintf(fmt, ...)  fprintf(stderr, "ERROR in %s:%d, " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

using namespace std;

void trim(std::string &s);
string toString(vector<int> v);
bool endsWith(std::string const &fullString, std::string const &ending);
void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr);
