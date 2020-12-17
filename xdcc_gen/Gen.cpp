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

#include "Gen.h"
#include "XdccFlow.h"
#include "util.h"
#include "XdccException.h"
#include "main.h"

Gen::Gen(const string &path, const string &filename, const string &header)
{
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

void Gen::generate(XdccFlow &xdccFlow)
{
    setMessageLocal(xdccFlow);

    if (xdccFlow.getMessages().empty()) {
        wprintf("empty messages block");
        return;
    }
    open(xdccFlow);
    gen(xdccFlow);
    close();
}

void Gen::beginFunc(json& schemaJson, Message *message)
{
    copies.clear();
    stmts.clear();
    in_args.clear();
    out_args.clear();

    string msg_name = message->getName();

    std::ifstream schemaStream(message->getSchemaFile());
    if (schemaStream.fail()) {
        eprintf("%s does not exist", message->getSchemaFile().c_str());
        throw DataException("");
    }
    schemaStream >> schemaJson;
    schemaStream.close();
}

void Gen::endOfFunc()
{
    varSet.clear();
    var_count = 1;
}

int Gen::genVar(string &key)
{
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

string Gen::genPath(vector<string> &path)
{
    string pathStr;
    for (std::vector<string>::iterator it = path.begin(); it != path.end();
            ++it) {
        pathStr += "[\"" + *it + "\"]";
    }

    return pathStr;
}

void Gen::genLeaf(vector<string> path, string leaf,
        vector<string> &assignments, bool isString)
{
    string left = isString ? "string(" : "";
    string right = isString ? ")" : "";

    string assign = "    js" + genPath(path) + " = " + left + leaf + right
            + ";";

    assignments.push_back(assign);
}

string Gen::getField(json js, string field, Message *message,
        vector<string> path)
{
    if (js.find(field) == js.end()) {
        string err = "missing '" + field + "' field in schema of "
                + message->getName() + genPath(path);
        throw DataException(err);
    }
    string val;
    try {
        val = js[field];
    }
    catch (nlohmann::detail::type_error &e) {
        val = to_string(js[field]);
    }

    return val;
}

void Gen::setMessageLocal(XdccFlow &xdccFlow)
{
    string enclave = config.getEnclave();

    for (auto const &msg_map : xdccFlow.getMessages()) {
        Message *message = (Message*) msg_map.second;
        string msgName = message->getName();

        bool local = true;
        for (auto const flow_map : xdccFlow.getFlows()) {
            Flow *flow = (Flow*) flow_map.second;
            if (flow->getMessage().compare(msgName))
                continue;

            Cle *cle = xdccFlow.find_cle(flow);
            if (cle == NULL) {
                eprintf("no CLE for %s/%s", msgName.c_str(), flow->getLabel().c_str());
                continue;
            }

            CleJson cleJson = cle->getCleJson();
            local = cleJson.isLocal(enclave, flow);
            if (!local)
                break;
        }
        if (debug)
            cout << "message " << msgName << " is " << (local ? "" : "not ")
                    << "local" << endl;
        message->setLocal(local);
    }
}
