#pragma once

#include "Gen.h"
#include "XdccFlow.h"
#include "main.h"

class GenEgress : public Gen
{
private:
    set<string> remoteEnclaves;
    map<string, set<string>> msgToEnclaves;  // message name to list of dst enclaves
    map<string, string> combo;   // msg_remote to cle def

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
    void genCombo(const XdccFlow &xdccFlow);
    void populateRemoteEnclaves(const XdccFlow &xdccFlow);

    void annotations(const XdccFlow &xdccFlow);

    void traverseEgress(json &schemaJson, Message *message);
    void traverseArrayEgress(Message *message, string arrayName, json j, vector<string> path, string numElements);
    void traverseObjEgress(Message *message, json j, vector<string> path);

    void genFlowToRemote(string msg_name, string remote);
    void genCoerce(string &my_enclave, string &msg_name_u);
    void genEgress(Message *message);

    void traverseEcho(json &schemaJson, Message *message);
    void traverseArrayEcho(Message *message, string arrayName, json j, vector<string> path, string numElements);
    void travereObjEcho(Message *message, json j, vector<string> path);

    void genEchoCommon(Message *message, bool singleRemote);
    void genEcho(Message *message, string component);
};
