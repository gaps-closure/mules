#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/algorithm/string.hpp>

using json = nlohmann::json;
using namespace std;

extern int verbose;

class Flow {
private:
    int dataId;
    string message;
    string label;
    string fromComponent;
    string toComponent;

public:
    Flow(nlohmann::basic_json<> value);

    const int getDataId() const {
        return dataId;
    }

    const string& getFromComponent() const {
        return fromComponent;
    }

    const string& getLabel() const {
        return label;
    }

    const string& getMessage() const {
        return message;
    }

    const string& getToComponent() const {
        return toComponent;
    }
};

class Component {
private:
    string component;
    string label;
    vector<int> inFlows;
    vector<int> outFlows;

public:
    Component(nlohmann::basic_json<> value);

    const string& getComponent() const {
        return component;
    }

    const vector<int>& getInMessages() const {
        return inFlows;
    }

    const string& getLabel() const {
        return label;
    }

    const vector<int>& getOutMessages() const {
        return outFlows;
    }
};

class Message {
private:
    string name;
    string schemaFile;
    string schemaType;
    bool topic;
    bool local = false;
    map<string, vector<Flow *>> outFlows;  // component to flows from that component

public:
    Message(nlohmann::basic_json<> value);

    void addOutFlow(Flow* flow);
    void clearOutFlow();

    const string& getName() const {
        return name;
    }

    const string& getSchemaFile() const {
        return schemaFile;
    }

    const string& getSchemaType() const {
        return schemaType;
    }

    bool isTopic() const {
        return topic;
    }

    bool isLocal() const {
        return local;
    }

    void setLocal(bool local = false) {
        this->local = local;
    }

    const map<string, vector<Flow *>>& getOutFlows() const {
        return outFlows;
    }
};

class GuardDirective
{
public:
    string operation;
    vector<int> gapstag;
    bool oneway = false;

    GuardDirective() {
    }

    ~GuardDirective() {};

    GuardDirective(nlohmann::basic_json<> value);

    string getOperation() const {
        return operation;
    }

    vector<int> getGapstag() const {
        return gapstag;
    }

    bool isOneway() const {
        return oneway;
    }
};

class Cdf
{
public:
    string remotelevel;
    string direction;
    GuardDirective guarddirective;

    vector<vector<string>> argtaints;
    vector<string> codtaints;
    vector<string> rettaints;

    Cdf() {
    }

    ~Cdf() {};

    Cdf(nlohmann::basic_json<> value);

    string getRemoteLevel() const {
        return remotelevel;
    }

    string getDirection() const {
        return direction;
    }

    GuardDirective getGuardDirective() const {
        return guarddirective;
    }

    vector<vector<string> > getArgtaints() const {
        return argtaints;
    }

    vector<string> getCodtaints() const {
        return codtaints;
    }

    vector<string> getRettaints() const {
        return rettaints;
    }
};

class CleJson
{
public:
    string level;
    vector<Cdf> cdf;

  public:
    CleJson() {
    }

    CleJson(string level, Cdf *cdf);

    ~CleJson() {};

    CleJson(nlohmann::basic_json<> value);

    Cdf* find_cdf(string level, string remote, bool note);

    bool isLocal(string enclave, Flow *flow);

    string getDirection() {
        return level + " -> " + cdf[0].getRemoteLevel();
    }

    string getLevel() const {
        return level;
    }

    vector<Cdf> getCdf() const {
        return cdf;
    }

    void setCdf(vector<Cdf> cdf) {
        this->cdf = cdf;
    }

    void setLevel(string level) {
        this->level = level;
    }
};

class Cle
{
public:
    string label;
    CleJson cleJson;

  public:
    Cle(nlohmann::basic_json<> value);
    
    ~Cle() {
    }

    const CleJson& getCleJson() const {
        return cleJson;
    }

    const string& getLabel() const {
        return label;
    }
};

class XdccFlow {
public:

    map<string, Component *> topology;
    map<string, Message *> messages;
    map<int, Flow * > flows;
    map<string, Cle*> cles;

    XdccFlow() {
    }

    XdccFlow(const string &filename);

    /**
     * Find the Cdf of the remote enclave that is the destination of a message.
     */
    Cdf *find_cle(string level, string remote) const;

    Cle *find_cle(const Flow *flow) const;
    Cle* find_cle(string &label) const;

    const map<string, Cle*>& getCles() const {
        return cles;
    }

    const map<int, Flow*>& getFlows() const {
        return flows;
    }

    const map<string, Message*>& getMessages() const {
        return messages;
    }

    const map<string, Component*>& getTopology() const {
        return topology;
    }
};
