#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

#include "Config.h"
#include "XdccFlow.h"
#include "util.h"
#include "XdccException.h"

namespace fs = boost::filesystem;

using json = nlohmann::json;
using namespace std;

extern Config config;

class Gen {
protected:
    ofstream genfile;
    ofstream headerfile;
    std::set<string> varSet;
    int var_count = 1;
    set<const Message *> myMessages;

public:
    virtual int gen(XdccFlow& xdccFlow) = 0;
    virtual int open(const XdccFlow &xdccFlow) = 0;
    virtual int close() = 0;

    Gen(const string& path, const string& filename, const string& header);

    virtual ~Gen() {}

    void generate(XdccFlow& xdccFlow);

    void endOfFunc();

    int gen_var(string &key);

    string gen_path(vector<string> &path);

    void gen_leaf(vector<string> path, string leaf, vector<string> &assignments, bool isString);

    string get_field(json js, string field, Message *message, vector<string> path);

private:

    void get_my_messages(XdccFlow& xdccFlow);
};
