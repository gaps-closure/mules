#pragma once

#include "Gen.h"

class GenEcho : public Gen
{
public:
    GenEcho(const string& path, const string& filename, const string& header) : Gen(path, filename, header) {
    }

    int gen(XdccFlow& xdccFlow);
    int open(const XdccFlow &xdccFlow);
    int close();

private:
    void genEcho(Message *message);
    void genEchoObj(Message *message, json j, vector<string> path, vector<string> &assignments,
                        vector<string> &in_args, vector<string> &out_args);
    void genEchoArray(Message *message, string arrayName, json j, vector<string> path, vector<string> &assignments,
                          vector<string> &in_args, vector<string> &out_args);

    void genUnmarshalArray(Message *message, string arrayName, json j, vector<string> path,
            vector<string> &assignments, vector<string> &in_args, vector<string> &out_args);
    void genUnmarshalObj(Message *message, json j, vector<string> path, vector<string> &assignments,
            vector<string> &in_args, vector<string> &out_args);
    void genUnmarshal(Message *message);
};
