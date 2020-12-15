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

        if (!key.compare("flowId")) {
            flowId = el.value().get<int>();
        }
        else if (!key.compare("message")) {
            message = el.value().get<string>();
        }
        else if (!key.compare("label")) {
            label = el.value().get<string>();
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
        else if (!key.compare("inFlows")) {
            for (auto &el2 : el.value().items())
                inFlows.push_back(el2.value());
        }
        else if (!key.compare("outFlows")) {
            for (auto &el2 : el.value().items())
                outFlows.push_back(el2.value());
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
                string comp_name = component->getComponent();
                map<string, Component *>::iterator it = topology.find(comp_name);
                if (it != topology.end()) {
                    eprintf("duplicated component: %s", comp_name.c_str());
                    continue;
                }
                topology[comp_name] = component;
            }
        }
        else if (!key.compare("messages")) {
            for (auto &el : value.items()) {
                Message *message = new Message(el.value());

                string msg_name = message->getName();
                map<string, Message *>::iterator it = messages.find(msg_name);
                if (it != messages.end()) {
                    eprintf("duplicated message: %s", msg_name.c_str());
                    continue;
                }
                messages[msg_name] = message;
            }
        }
        else if (!key.compare("flows")) {
            for (auto &el2 : value.items()) {
                Flow *flow = new Flow(el2.value());

                int flowId = flow->getFlowId();
                map<int, Flow *>::iterator it = flows.find(flowId);
                if (it != flows.end()) {
                    eprintf("duplicated flow: %d", flowId);
                    continue;
                }
                flows[flowId] = flow;
            }
        }
        else if (!key.compare("cles")) {
            for (auto &el : value.items()) {
                Cle *cle = new Cle(el.value());

                string label = cle->getLabel();
                map<string, Cle *>::iterator it = cles.find(label);
                if (it != cles.end()) {
                    eprintf("duplicated CLE: %s", label.c_str());
                    continue;
                }
                cles[label] = cle;
            }
        }
        else {
            eprintf("unrecognized key %s", key.c_str());
        }
    }
    verify();
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
    string label = flow->getLabel();
    return find_cle(label);
}

Cle* XdccFlow::find_cle(string &label) const
{
    map<string, Cle*>::const_iterator it = cles.find(label);
    if (it == cles.end()) {
        return NULL;
    }
    return (Cle*) it->second;
}

void verifyComponent(const XdccFlow& xdccFlow)
{
    map<string, Cle*> cles = xdccFlow.getCles();
    map<int, Flow *> flows = xdccFlow.getFlows();

    for (auto const topoMap : xdccFlow.getTopology()) {
        Component *component = (Component *) topoMap.second;
        string comp_name = component->getComponent();

        vector<vector<string>> argtaints;
        map<string, Cle *>::const_iterator itCle = cles.find(component->getLabel());
        if (itCle == cles.end()) {
            eprintf("undefined CLE %s for component %s", component->getLabel().c_str(), comp_name.c_str());
        }
        else {
            CleJson cleJson = itCle->second->getCleJson();
            vector<Cdf> cdfs = cleJson.getCdf();
            Cdf cdf = cdfs[0];
            argtaints = cdf.getArgtaints();
        }

        // check that all referenced flowId's do exist in the flows block
        // and collect the CLE label of the flows.
        vector<string> mytaints;
        for (auto const flowId : component->getInFlows()) {
            map<int, Flow *>::const_iterator it = flows.find(flowId);
            if (it == flows.end()) {
                eprintf("%s inFlow %d is undefined in XDCC's the \"flows\" block",
                        comp_name.c_str(), flowId);
                continue;
            }
            mytaints.push_back(it->second->getLabel());
        }

        for (auto const flowId : component->getOutFlows()) {
            map<int, Flow *>::const_iterator it = flows.find(flowId);
            if (it == flows.end()) {
                eprintf("%s outFlow %d is undefined in XDCC's the \"flows\" block",
                        comp_name.c_str(), flowId);
                continue;
            }
            mytaints.push_back(it->second->getLabel());
        }

        // check that the implied taints of the flows are the same as those listed in the componens' CLE
        if (mytaints.size() != argtaints.size()) {
            eprintf("%s, length of argtaints do not match inFlows and outFlows: %ld v.s. %ld",
                    comp_name.c_str(), mytaints.size(), argtaints.size());
        }
        else {
            for (int i = 0; i < mytaints.size(); i++) {
                vector<string> oneArg  = argtaints[i];
                if (oneArg.size() == 0) {
                    eprintf("empty taints in the CLE for the %d argument of %s\n", i, comp_name.c_str());
                }
                else {
                    string cdfTaint = oneArg[0];
                    if (mytaints[i].compare(cdfTaint)) {
                        eprintf("taints mismatched taints for the %d argument of %s: %s v.s. %s\n",
                                i, comp_name.c_str(), cdfTaint.c_str(), mytaints[i].c_str());
                    }
                }
            }
        }
    }
}

bool XdccFlow::verify() const
{
    verifyComponent(*this);

    for (auto const flow_map : flows) {
        Flow *flow = (Flow*) flow_map.second;
        int flowId = flow->getFlowId();

        bool foundIn = false;
        bool foundOut = false;

        for (auto const topoMap : topology) {
            Component *component = (Component *) topoMap.second;
            string comp_name = component->getComponent();

            bool localFoundIn = false;
            bool localFoundOut = false;
            vector<int> list = component->getInFlows();
            if (std::find(list.begin(), list.end(), flowId) != list.end()) {
                if (foundIn) {
                    eprintf("Flow %d appear more than once in inFlow", flowId);
                }
                localFoundIn = true;
                foundIn = true;
            }

            list = component->getOutFlows();
            if (std::find(list.begin(), list.end(), flowId) != list.end()) {
                if (foundOut) {
                    eprintf("Flow %d appear more than once in outFlow", flowId);
                }
                localFoundOut = true;
                foundOut = true;
            }
            if (localFoundIn && localFoundOut)
                eprintf("Flow %d appear in both inFlows and outFlows of %s", flowId, comp_name.c_str());
        }
        if (!foundIn || !foundOut) {
            eprintf("Flow %d must appear exactly once in inFlows and outFlows of two components", flowId);
        }

        string msgName = flow->getMessage();

        map<string, Message *>::const_iterator it = messages.find(msgName);
        if (it == messages.end()) {
            eprintf("undefined message %s for flow %d", msgName.c_str(), flowId);
        }

        map<string, Cle *>::const_iterator it2 = cles.find(flow->getLabel());
        if (it2 == cles.end()) {
            eprintf("undefined CLE %s for flow %d", flow->getLabel().c_str(), flowId);
        }
    }

    return true;
}
