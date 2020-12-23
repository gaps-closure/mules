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

class Gen {
protected:
    ofstream genfile;
    ofstream headerfile;
    std::set<string> varSet;
    int var_count = 1;

    vector<string> copies;
    vector<string> stmts;
    vector<string> in_args;
    vector<string> out_args;

public:
    virtual int gen(XdccFlow &xdccFlow) = 0;
    virtual int open(const XdccFlow &xdccFlow) = 0;
    virtual int close() = 0;

    Gen(const string &path, const string &filename, const string &header);

    virtual ~Gen() {
    }

    void generate(XdccFlow &xdccFlow);

    void beginFunc(json& schemaJson, Message *message);
    void endOfFunc();

    int genVar(string &key);

    string genPath(vector<string> &path);

    void genLeaf(vector<string> path, string leaf, vector<string> &assignments,
            bool isString);

    string getField(json js, string field, Message *message,
            vector<string> path);

private:
    void setMessageLocal(XdccFlow &xdccFlow);
};
