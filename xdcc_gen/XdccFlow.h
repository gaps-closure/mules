#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

class Flow {
public:
    string source;
    string destination;

    Flow(nlohmann::basic_json<> value) {
        for (auto& el : value.items()) {
            string key = el.key();

            if (!key.compare("source")) {
                source = el.value().get<string>();
            }
            else if (!key.compare("destination")) {
                destination = el.value().get<string>();
            }
        }
    }

    string getDestination() const {
        return destination;
    }

    string getSource() const {
        return source;
    }
};

class Message {
public:

    string name;
    string comment;
    vector<Flow> flows;
    string schema;
    string cle;
    bool local;
    bool topic;

    Message(nlohmann::basic_json<> value) {
        for (auto& el : value.items()) {
            string key = el.key();

            if (!key.compare("name")) {
                name = el.value().get<string>();
            }
            else if (!key.compare("comment")) {
                comment = el.value().get<string>();
            }
            else if (!key.compare("schema")) {
                schema = el.value().get<string>();
            }
            else if (!key.compare("cle")) {
                cle = el.value().get<string>();
            }
            else if (!key.compare("local")) {
                local = el.value().get<bool>();
            }
            else if (!key.compare("topic")) {
                topic = el.value().get<bool>();
            }
            else if (!key.compare("flows")) {
                for (auto& el2 : el.value().items()) {
                    Flow flow(el2.value());
                    flows.push_back(flow);
                }
            }
        }
    }

    string getCle() const {
        return cle;
    }

    string getComment() const {
        return comment;
    }

    vector<Flow> getFlows() const {
        return flows;
    }

    string getName() const {
        return name;
    }

    string getSchema() const {
        return schema;
    }

    bool isLocal() const {
        return local;
    }

    bool isTopic() const {
        return topic;
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

    GuardDirective(nlohmann::basic_json<> value) {

        for (auto& el : value.items()) {
            string key = el.key();

            if (!key.compare("operation")) {
                operation = el.value().get<string>();
            }
            else if (!key.compare("gapstag")) {
                for (auto& el2 : el.value().items()) {
                    gapstag.push_back(el2.value());
                }
            }
            else if (!key.compare("oneway")) {
                oneway = el.value().get<bool>();
            }
        }
    };

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

    Cdf(nlohmann::basic_json<> value) {
        for (auto& el : value.items()) {
            string key = el.key();

            if (!key.compare("remotelevel")) {
                remotelevel = el.value().get<string>();
            }
            else if (!key.compare("direction")) {
                direction = el.value().get<string>();
            }
            else if (!key.compare("guarddirective")) {
                GuardDirective g(el.value());
                guarddirective = g;
            }
            else if (!key.compare("codtaints")) {
                for (auto& el2 : el.value().items()) {
                    codtaints.push_back(el2.value().get<string>());
                }
            }
            else if (!key.compare("rettaints")) {
                for (auto& el2 : el.value().items()) {
                    rettaints.push_back(el2.value().get<string>());
                }
            }
            else if (!key.compare("argtaints")) {
                for (auto& el2 : el.value().items()) {
                    vector<string> vs;
                    for (auto& el3 : el2.value().items()) {
                        vs.push_back(el3.value().get<string>());
                    }
                    argtaints.push_back(vs);
                }
            }
        }
    };

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

class Cle
{
public:
    string level;
    vector<Cdf> cdf;

  public:
    Cle() {
    }

    Cle(string level, Cdf *cdf) {
        this->level = level;
        this->cdf.push_back(*cdf);
    }

    ~Cle() {};

    Cle(nlohmann::basic_json<> value) {
        for (auto& el : value.items()) {
            string key = el.key();

            if (!key.compare("level")) {
                level = el.value().get<string>();
            }
            else if (!key.compare("cdf")) {
                for (auto& el2 : el.value().items()) {
                    Cdf c(el2.value());
                    cdf.push_back(c);
                }
            }
        }
    };

    Cdf* find_cdf(string level, string remote, bool note) {
        string level_l = level;
        boost::to_lower(level_l);

        for (int i = 0; i < cdf.size(); i++) {
            if (!this->level.compare(level_l) &&
                !cdf[i].getRemoteLevel().compare(remote)) {
                return &cdf[i];
            }
        }
        if (note)
            cout << "No CDF for level/remote: " << level << "/" << remote << endl;
        return NULL;
    }

    string getDirection() {
        return level + " -> " + cdf[0].getRemoteLevel();
    }

    string getLevel() {
        return level;
    }

    vector<Cdf>& getCdf() {
        return cdf;
    }

    void setCdf(vector<Cdf> cdf) {
        this->cdf = cdf;
    }

    void setLevel(string level) {
        this->level = level;
    }
};

class XdccFlow {
public:

    map<string, string> components;
    map<string, Message *> messages;
    map<string, Cle *> cles;

    XdccFlow(const string &filename) {
        std::ifstream jStream(filename);
        json js;
        jStream >> js;

        for (auto& el : js.items()) {
            put(el.key(), el.value());
        }
    }

    /**
     * Find the Cdf of the remote enclave that is the destination of a message.
     */
    Cdf *find_cle(string level, string remote) const {
        string level_l = level;
        boost::to_lower(level_l);

        Cle *cle = NULL;
        for (auto const& y : cles) {
            Cle *cle = (Cle *) y.second;
            Cdf *cdf = cle->find_cdf(level, remote, false);
            if (cdf != NULL)
                return cdf;
        }
        cout << "No CLE for level/remote " << level << "/" << remote << endl;
        return NULL;
    }

    Cle *find_cle(const Message *message) const {
        std::map<string, Cle *>::const_iterator it = cles.find(message->getCle());
        if (it == cles.end()) {
            return NULL;
        }
        return it->second;
    }

    void put(const string &key, nlohmann::basic_json<> value) {
        if (!key.compare("components")) {
            for (auto& el : value.items()) {
                components[el.key()] = el.value().get<string>();
            }
        }
        else if (!key.compare("messages")) {
            for (auto& el : value.items()) {
                messages[el.key()] = new Message(el.value());
            }
        }
        else if (!key.compare("cles")) {
            for (auto& el : value.items()) {
                cles[el.key()] = new Cle(el.value());
            }
        }
    }

    map<string, Cle*> getCles() const {
        return cles;
    }

    map<string, string> getComponents() const {
        return components;
    }

    map<string, Message*> getMessages() const {
        return messages;
    }
};
