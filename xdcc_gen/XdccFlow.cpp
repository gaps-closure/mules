#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>

#include "XdccFlow.h"
#include "main.h"
#include "util.h"

using json = nlohmann::json;
using namespace std;

Flow::Flow(nlohmann::basic_json<> value)
{
    for (auto &el : value.items()) {
        string key = el.key();

        if (!key.compare("dataId")) {
            dataId = el.value().get<int>();
        }
        else if (!key.compare("message")) {
            message = el.value().get<string>();
        }
        else if (!key.compare("label")) {
            label = el.value().get<string>();
        }
        else if (!key.compare("fromComponent")) {
            fromComponent = el.value().get<string>();
        }
        else if (!key.compare("toComponent")) {
            toComponent = el.value().get<string>();
        }
        else {
            eprintf("unrecognized key %s", key.c_str());
        }
    }
}

Component::Component(nlohmann::basic_json<> value)
{
    for (auto &el : value.items()) {
        string key = el.key();

        if (!key.compare("component")) {
            component = el.value().get<string>();
        }
        else if (!key.compare("label")) {
            label = el.value().get<string>();
        }
        else if (!key.compare("inMessages")) {
            for (auto &el2 : el.value().items())
                inMessages.push_back(el2.value());
        }
        else if (!key.compare("outMessages")) {
            for (auto &el2 : el.value().items())
                outMessages.push_back(el2.value());
        }
        else {
            eprintf("unrecognized key %s", key.c_str());
        }
    }
}

Message::Message(nlohmann::basic_json<> value)
{
    for (auto &el : value.items()) {
        string key = el.key();

        if (!key.compare("name")) {
            name = el.value().get<string>();
        }
        else if (!key.compare("schemaFile")) {
            schemaFile = el.value().get<string>();
        }
        else if (!key.compare("schemaType")) {
            schemaType = el.value().get<string>();
        }
        else if (!key.compare("topic")) {
            topic = el.value().get<bool>();
        }
        else {
            eprintf("unrecognized key %s", key.c_str());
        }
    }
    if (schemaFile.empty()) {
        eprintf("schemaFile not specified for message %s", name.c_str());
    }
}

GuardDirective::GuardDirective(nlohmann::basic_json<> value)
{

    for (auto &el : value.items()) {
        string key = el.key();

        if (!key.compare("operation")) {
            operation = el.value().get<string>();
        }
        else if (!key.compare("gapstag")) {
            for (auto &el2 : el.value().items()) {
                gapstag.push_back(el2.value());
            }
        }
        else if (!key.compare("oneway")) {
            oneway = el.value().get<bool>();
        }
        else {
            eprintf("unrecognized key %s", key.c_str());
        }
    }
}

Cdf::Cdf(nlohmann::basic_json<> value)
{
    for (auto &el : value.items()) {
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
            for (auto &el2 : el.value().items()) {
                codtaints.push_back(el2.value().get<string>());
            }
        }
        else if (!key.compare("rettaints")) {
            for (auto &el2 : el.value().items()) {
                rettaints.push_back(el2.value().get<string>());
            }
        }
        else if (!key.compare("argtaints")) {
            for (auto &el2 : el.value().items()) {
                vector<string> vs;
                for (auto &el3 : el2.value().items()) {
                    vs.push_back(el3.value().get<string>());
                }
                argtaints.push_back(vs);
            }
        }
        else {
            eprintf("unrecognized key %s", key.c_str());
        }
    }
}
;

CleJson::CleJson(string level, Cdf *cdf)
{
    this->level = level;
    this->cdf.push_back(*cdf);
}

CleJson::CleJson(nlohmann::basic_json<> value)
{
    for (auto &el : value.items()) {
        string key = el.key();

        if (!key.compare("level")) {
            level = el.value().get<string>();
        }
        else if (!key.compare("cdf")) {
            for (auto &el2 : el.value().items()) {
                Cdf c(el2.value());
                cdf.push_back(c);
            }
        }
        else {
            eprintf("unrecognized key %s", key.c_str());
        }
    }
}

Cdf* CleJson::find_cdf(string level, string remote, bool note)
{
    string level_l = level;
    boost::to_lower(level_l);

    for (int i = 0; i < cdf.size(); i++) {
        if (!this->level.compare(level_l)
                && !cdf[i].getRemoteLevel().compare(remote)) {
            return &cdf[i];
        }
    }
    if (note)
        cout << "No CDF for level/remote: " << level << "/" << remote << endl;
    return NULL;
}

bool CleJson::isLocal(string enclave, Flow *flow)
{
    for (int i = 0; i < cdf.size(); i++) {
        if (debug) {
            cout << "my enclave: " << enclave << ", level:" << level
                    << ", remoteLevel: " << cdf[i].getRemoteLevel() << endl;
        }
        if (this->level.compare(enclave))  // not flowing from my enclave
            return false;

        if (cdf[i].getRemoteLevel().compare(enclave)) // flow to a different enclave
            return false;
    }
    return true;
}

Cle::Cle(nlohmann::basic_json<> value)
{
    for (auto &el : value.items()) {
        string key = el.key();

        if (!key.compare("cle-label")) {
            label = el.value().get<string>();
        }
        else if (!key.compare("cle-json")) {
            CleJson c(el.value());
            cleJson = c;
        }
        else {
            eprintf("unrecognized key %s", key.c_str());
        }
    }
}

XdccFlow::XdccFlow(const string &filename)
{
    std::ifstream jStream(filename);
    json js;
    jStream >> js;

    for (auto &el : js.items()) {
        string key = el.key();
        nlohmann::basic_json<> value = el.value();

        if (!key.compare("topology")) {
            for (auto &el : value.items()) {
                Component *component = new Component(el.value());
                topology[component->getComponent()] = component;
                if (debug)
                    cout << "component: " << component->getComponent() << endl;
            }
        }
        else if (!key.compare("messages")) {
            for (auto &el : value.items()) {
                Message *message = new Message(el.value());
                messages[message->getName()] = message;
                if (debug)
                    cout << "message: " << message->getName() << endl;
            }
        }
        else if (!key.compare("flows")) {
            for (auto &el2 : value.items()) {
                Flow *flow = new Flow(el2.value());
                flows[flow->getDataId()] = flow;
                if (debug)
                    cout << "flow: " << flow->getDataId() << endl;
            }
        }
        else if (!key.compare("cles")) {
            for (auto &el : value.items()) {
                Cle *cle = new Cle(el.value());
                cles[cle->getLabel()] = cle;
                if (debug)
                    cout << "cle: " << cle->getLabel() << endl;
            }
        }
        else {
            eprintf("unrecognized key %s", key.c_str());
        }
    }
}

/**
 * Find the Cdf of the remote enclave that is the destination of a message.
 */
Cdf* XdccFlow::find_cle(string level, string remote) const
{
    string level_l = level;
    boost::to_lower(level_l);

    Cle *cle = NULL;
    for (auto const &y : cles) {
        Cle *cle = (Cle*) y.second;
        CleJson clejson = cle->getCleJson();
        Cdf *cdf = clejson.find_cdf(level, remote, false);
        if (cdf != NULL)
            return cdf;
    }
    cout << "No CLE for level/remote " << level << "/" << remote << endl;
    return NULL;
}

Cle* XdccFlow::find_cle(const Flow *flow) const
{
    map<string, Cle*>::const_iterator it = cles.find(flow->getLabel());
    if (it == cles.end()) {
        return NULL;
    }
    return (Cle*) it->second;
}
