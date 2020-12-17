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
    vector<string> copies;
    vector<string> stmts;
    vector<string> in_args;
    vector<string> out_args;

    void beginFunc(Message *message, json& schemaJson);

    void traverseEcho(json &schemaJson, Message *message);
    void traverseArrayEcho(Message *message, string arrayName, json j, vector<string> path, string numElements);
    void traverseObjEcho(Message *message, json j, vector<string> path);

    void genEcho(Message *message);
    void genUnmarshal(Message *message);

    void traverseUnmarshal(json &schemaJson, Message *message);
    void traverseArrayUnmarshal(Message *message, string arrayName, json j, vector<string> path, string numElements);
    void traverseObjUnmarshal(Message *message, json j, vector<string> path);
};
