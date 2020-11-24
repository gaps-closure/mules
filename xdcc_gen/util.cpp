#include <iostream>
#include <vector>
#include <string>
#include <exception>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unistd.h>
#include <dirent.h>
#include <algorithm>
#include <set>

#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

#include "util.h"

extern std::set<string> varSet;
extern int var_count;

// trim from start (in place)
static inline void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
void trim(std::string &s)
{
    ltrim(s);
    rtrim(s);
}

string toString(vector<int> v)
{
    string rtn;
    for (int i : v)
        rtn += to_string(i) + " ";

    return rtn;
}

bool endsWith(std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    }
    else {
        return false;
    }
}

void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr)
{
    size_t pos = data.find(toSearch);

    while (pos != std::string::npos) {
        data.replace(pos, toSearch.size(), replaceStr);
        pos = data.find(toSearch, pos + replaceStr.size());
    }
}
