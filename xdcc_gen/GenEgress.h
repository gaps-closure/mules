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
    vector<string> stmts;
    vector<string> in_args;
    vector<string> out_args;

    map<string, string> combo;   // message_component_enclave to cle definition
    map<string, vector<string>> msgFanOuts;  // message name to list of combos for that message
    map<string, map<string, vector<int>>>  groups; // msg -> (level -> [ dataId ])

    XdccFlow xdccFlow;

public:
    GenEgress(const string& path, const string& filename, const string& header) : Gen(path, filename, header) {
    }

    int gen(XdccFlow& xdccFlow);
    int open(const XdccFlow &xdccFlow);
    int close();

    set<string> & getRemoteEnclaves() {
        return remoteEnclaves;
    }

    const XdccFlow& getXdccFlow() const
    {
        return xdccFlow;
    }

    void setXdccFlow(const XdccFlow &xdccFlow)
    {
        this->xdccFlow = xdccFlow;
    }

protected:
    void genCombo(const XdccFlow &xdccFlow);
    void groupByLevels(vector<Flow *> flows, map<string, vector<int>>& groups);
    void beginFunc(Message *message, json& schemaJson);

    void populateRemoteEnclaves(const XdccFlow &xdccFlow);
    void annotations(const XdccFlow &xdccFlow);

    void traverseEgress(Message *message);
    void traverseArrayEgress(Message *message, json j, vector<string> path);
    void traverseObjEgress(Message *message, json j, vector<string> path);

    void genFlow(bool isElse, string msg_name, string component, string remote, vector<int> ids);
    void genEgress(Message *message);

    void traverseEcho(Message *message);
    void traverseArrayEcho(Message *message, json j, vector<string> path);
    void travereObjEcho(Message *message, json j, vector<string> path);

    void genEchoCommon(Message *message);
    void genEcho(Message *message, string component);
};
