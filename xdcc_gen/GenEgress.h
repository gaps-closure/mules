#pragma once

#include "Gen.h"
#include "XdccFlow.h"
#include "main.h"

class GenEgress : public Gen
{
private:
    map<string, string> shares;   // from message names to enclave names
    set<string> remoteEnclaves;
    vector<string> copies;

public:
    GenEgress(const string& path, const string& filename, const string& header) : Gen(path, filename, header) {
    }

    int gen(XdccFlow& xdccFlow);
    int open(const XdccFlow &xdccFlow);
    int close();

    set<string> & getRemoteEnclaves() {
        return remoteEnclaves;
    }

protected:
    void populateRemoteEnclaves(const XdccFlow &xdccFlow);
    void genOneBranch(bool isElse, string msg_name, string component, vector<Flow *> flows, vector<string> assignments, vector<string> out_args);
    void annotations(const XdccFlow &xdccFlow);

    void genEgress(Message *message);
    void genEgressArray(Message *message, json j, vector<string> path,
                          vector<string> &assignments, vector<string> &in_args, vector<string> &out_args);
    void genEgressObj(Message *message, json j, vector<string> path, vector<string> &assignments,
            vector<string> &in_args, vector<string> &out_args);

    void genCommon(Message *message);
    void genXdcc(Message *message, string component);
    void genXdccArray(Message *message, json j, vector<string> path, vector<string> &assignments,
                        vector<string> &in_args, vector<string> &out_args);
    void genXdccObj(Message *message, json j, vector<string> path, vector<string> &assignments,
                      vector<string> &in_args, vector<string> &out_args);
};
