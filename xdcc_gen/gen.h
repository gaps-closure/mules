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
    int var_count;

public:
    virtual int generate(XdccFlow& xdccFlow) = 0;
    virtual int open(const XdccFlow &xdccFlow) = 0;
    virtual int close() = 0;

    Gen(const string& path, const string& filename, const string& header) {
        string fname = filename;
        string hname = header;
        if (path.compare(".") != 0) {
            fs::path dir(path);

            if (!fs::exists(dir)) {
                if (!boost::filesystem::create_directories(dir)) {
                    cout << "failed to create output directory: " << path << endl;
                    exit(1);
                }
            }
            fname = path + "/" + filename;
            hname = path + "/" + header;
        }
        genfile.open(fname);

        if (!header.empty()) {
            headerfile.open(hname);
        }
    }

    bool is_interested(string message_key, string enclave) {
        string upper_enclave = enclave;
        boost::to_upper(upper_enclave);

        return endsWith(message_key, upper_enclave);
    }

    void endOfFunc() {
        varSet.clear();
        var_count = 1;
    }

    int gen_var(string &key)  {
        std::set<std::string>::iterator it = varSet.find(key);
        if (it != varSet.end()) {
            key += to_string(var_count);
            var_count++;
            return 1;
        }
        else {
            varSet.insert(key);
            return 0;
        }
    }

    string gen_path(vector<string> &path) {
        string pathStr;
        for (std::vector<string>::iterator it = path.begin(); it != path.end(); ++it) {
            pathStr += "[\"" + *it + "\"]";
        }

        return pathStr;
    }

    void gen_leaf(vector<string> path, string leaf, vector<string> &assignments, bool isString) {
       string left = isString ? "string(" : "";
       string right = isString ? ")" : "";

       ;

       string assign = "    js" + gen_path(path) + " = " + left + leaf + right + ";";

       assignments.push_back(assign);
    }

    string get_field(json js, string field, Message *message, vector<string> path) {
        if (js.find(field) == js.end()) {
            string err = "missing '" + field + "' field in schema of " + message->getName() + gen_path(path);
            throw DataException(err);
        }
        string val = to_string(js[field]);
        findAndReplaceAll(val, "\"", "");

        return val;
    }
};
