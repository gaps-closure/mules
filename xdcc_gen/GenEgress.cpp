#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <exception>
#include <unordered_map>
#include <set>
#include <utility>
#include <unistd.h>
#include <dirent.h>
#include <getopt.h>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <boost/algorithm/string.hpp>

#include "util.h"
#include "GenEgress.h"
#include "Config.h"
#include "XdccException.h"
#include "main.h"

using namespace std;
using json = nlohmann::json;

const string WCARD = "###";

void to_json(json& j, const GuardDirective& p) {
    string x = p.getOperation();

    std::vector<int> g = p.getGapstag();
    json j2 = g;

    j = json{
            {"operation", x},
            {"gapstag", j2},
            {"oneway", p.isOneway()}
    };
}

void to_json(json& j, const Cdf& p) {
    string r = p.getRemoteLevel();
    string d = p.getDirection();
    GuardDirective g = p.getGuardDirective();
    json j2;
    to_json(j2, g);

    vector<vector<string>> argtaints = p.getArgtaints();
    json jarg = argtaints;

    vector<string> codtaints = p.getCodtaints();
    json jcod = codtaints;

    json jret = p.getRettaints();

    j = json{
            {"remotelevel", r },
            {"direction", d },
            {"guarddirective", j2 },
            {"argtaints", jarg },
            {"codtaints", jcod },
            {"rettaints", jret },
    };
}

void to_json(json& j, Cle& p)
{
//    std::vector<Cdf> x = p.getCdf();
//    json j2 = x;
//
//    j = json{
//            {"level", p.getLevel()},
//            {"cdf", j2}
//    };
}

/******************************
 * XDCC
 */
void GenEgress::genXdccArray(Message *message, json j, vector<string> path, vector<string> &assignments,
                      vector<string> &in_args, vector<string> &out_args)
{
    string countVar = "count";
    genVar(countVar);
    assignments.push_back("    int " + countVar + " = 1; // TODO: get json array length");

    int i = 0;
    for (auto& el : j.items()) {
        string key = el.key();
        json val = el.value();

        string var(key);
        int inc = genVar(var);

        path.push_back(key);

        try {
            string type = getField(val, "type", message, path);
            if (type == "array") {
                genXdccArray(message, val["items"]["properties"], path, assignments, in_args, out_args);
            }
            else if (type == "object") {
                genXdccObj(message, j["properties"], path, assignments, in_args, out_args);
            }
            else {
                string in_arg;
                string out_arg = var;

                if (type == "string") {
                    in_arg = "const char *" + var + "[]";

                    string maxLength = getField(val, "maxLength", message, path);
                    assignments.push_back("    char " + var + "_cpp[" + countVar + "][" + maxLength + "];");
                    assignments.push_back("    for (int j = 0; j < " + countVar + "; j++)");
                    assignments.push_back("        memcpy(" + var + "_cpp[j], " + var + "[j], " + maxLength + ");\n");

                    out_arg = var + "_cpp";
                }
                else if (type == "integer") {
                    in_arg = "int " + var + "[]";
                }
                else if (type == "number") {
                    in_arg = "double " + var + "[]";
                }
                else {
                    throw DataException("unsupported type " + type + " for " + genPath(path));
                }
                in_args.push_back(in_arg);
                out_args.push_back(out_arg);
            }
        }
        catch (DataException &e) {
            e.print();
        }

        path.pop_back();
        var_count -= inc;
        varSet.erase(key);
    }
}

void GenEgress::genXdccObj(Message *message, json j, vector<string> path, vector<string> &assignments,
                    vector<string> &in_args, vector<string> &out_args)
{
    for (auto& el : j.items()) {
        string key = el.key();
        json val = el.value();

        path.push_back(key);

        try {
            string var(key);
            genVar(var);

            string type = getField(val, "type", message, path);
            if (type == "array") {
                genXdccArray(message, val["items"]["properties"], path, assignments, in_args, out_args);
            }
            else if (type == "object") {
                genXdccObj(message, val["properties"], path, assignments, in_args, out_args);
            }
            else {
                string in_arg;
                string out_arg = var;
                if (type == "string") {
                    in_arg = "const char *" + var;

                    string maxLength = getField(val, "maxLength", message, path);

                    string stmt = "    char " + var + "_cpp[" + maxLength + "];";
                    assignments.push_back(stmt);

                    stmt = "    memcpy(" + var + "_cpp, " + var + ", " + maxLength + ");\n";
                    assignments.push_back(stmt);

                    out_arg = var + "_cpp";
                }
                else if (type == "integer") {
                    in_arg = "int " + var;
                }
                else if (type == "number") {
                    in_arg = "double " + var;
                }
                else {
                    throw DataException("unsupported type " + type + " for " + genPath(path));
                }
                in_args.push_back(in_arg);
                out_args.push_back(out_arg);
            }
        }
        catch (DataException &e) {
            e.print();
        }
        path.pop_back();
    }
}

void GenEgress::genCommon(Message *message)
{
    string msg_name = message->getName();

    std::ifstream schemaStream(message->getSchemaFile());
    if (schemaStream.fail()) {
        eprintf("%s does not exist", message->getSchemaFile().c_str());
        return;
    }
    json schemaJson;
    schemaStream >> schemaJson;
    schemaStream.close();

    try {
        vector<string> path;
        vector<string> assignments;
        vector<string> in_args;
        vector<string> out_args;

        string type = getField(schemaJson, "type", message, path);
        if (type == "array") {
            genXdccArray(message, schemaJson["items"]["properties"], path, assignments, in_args, out_args);
        }
        else if (type == "object") {
            genXdccObj(message, schemaJson["properties"], path, assignments, in_args, out_args);
        }
        else {
            throw DataException("unsupported type: " + type + " for " + message->getName());
        }

        genfile << "int echo_" << msg_name << "_common(";

        bool first = true;
        for (std::vector<string>::iterator it = in_args.begin(); it != in_args.end(); ++it) {
            if (!first) {
                genfile << ",";
            }
            else {
                first = false;
            }
            genfile << "\n    " << *it;
        }
        genfile << endl
                << ")" << endl
                << "{" << endl;

        for (std::vector<string>::iterator it = assignments.begin(); it != assignments.end(); ++it) {
            genfile << *it << endl;
        }

        genfile << "    echo_" + msg_name + "_cpp(\n"
                << "        amq(),\n"
                << "        _topic_" + msg_name;
        for (std::vector<string>::iterator it = out_args.begin(); it != out_args.end(); ++it) {
            genfile << ",";
            genfile << "\n        " << *it;
        }
        genfile << "\n    );\n";

        genfile << "    return 0;" << endl
                << "}" << endl
                << endl;
    }
    catch (DataException &e) {
        e.print();
    }
}

void GenEgress::genXdcc(Message *message, string component)
{
    string msg_name = message->getName();

    std::ifstream schemaStream(message->getSchemaFile());
    if (schemaStream.fail()) {
        eprintf("%s does not exist", message->getSchemaFile().c_str());
        return;
    }
    json schemaJson;
    schemaStream >> schemaJson;
    schemaStream.close();

    try {
        vector<string> path;
        vector<string> assignments;
        vector<string> in_args;
        vector<string> out_args;

        string type = getField(schemaJson, "type", message, path);
        if (type == "array") {
            genXdccArray(message, schemaJson["items"]["properties"], path, assignments, in_args, out_args);
        }
        else if (type == "object") {
            genXdccObj(message, schemaJson["properties"], path, assignments, in_args, out_args);
        }
        else {
            throw DataException("unsupported type: " + type + " for " + message->getName());
        }

        string msg_name_u = msg_name;
        boost::to_upper(msg_name_u);
        genfile << "#pragma cle begin XDLINKAGE_ECHO_" << msg_name_u << "_" << component << endl
                << "int echo_" << msg_name << "_" << component << "(";

        bool first = true;
        for (std::vector<string>::iterator it = in_args.begin(); it != in_args.end(); ++it) {
            if (!first) {
                genfile << ",";
            }
            else {
                first = false;
            }
            genfile << "\n    " << *it;
        }
        genfile << endl
                << ")" << endl
                << "#pragma cle end XDLINKAGE_ECHO_" << msg_name_u << "_" << component<< endl
                << "{" << endl;

        genfile << "    echo_" + msg_name + "_common(";
        first = true;
        for (std::vector<string>::iterator it = out_args.begin(); it != out_args.end(); ++it) {
            if (!first)
                genfile << ",";
            genfile << "\n        " << *it;
            first = false;
        }
        genfile << "\n    );\n";

        genfile << "    return 0;" << endl
                << "}" << endl
                << endl;
    }
    catch (DataException &e) {
        e.print();
    }
}
/******************************
 * egress
 */
void GenEgress::genEgressArray(Message *message, json j, vector<string> path,
        vector<string> &assignments, vector<string> &in_args, vector<string> &out_args)
{
    string countVar = "count";
    genVar(countVar);
    in_args.push_back("        " + countVar);

    assignments.push_back("    int " + countVar + " = 1; // TODO: get json array length");

    int i = 0;
    for (auto& el : j.items()) {
        string key = el.key();
        json val = el.value();

        string var(key);
        genVar(var);

        path.push_back(key);
        try {
            string type = getField(val, "type", message, path);
            if (type == "array") {
                genEgressArray(message, val["items"]["properties"], path, assignments, in_args, out_args);
            }
            else if (type == "object") {
                genEgressObj(message, val["properties"], path, assignments, in_args, out_args);
            }
            else {
                string in_arg;
                string stmt;
                string out_arg;

                if (type == "string") {
                    string maxLength = getField(val, "maxLength", message, path);

                    stmt    = "    char " + var + "[" + countVar + "][" + maxLength + "];";
                    in_arg  = "        " + var;
                    out_arg = "            " + var;
                }
                else if (type == "integer") {
                    stmt    = "    int " + var + "[" + countVar + "];";
                    in_arg  = "        " + var;
                    out_arg = "            " + var;
                }
                else if (type == "number") {
                    stmt    = "    double " + var + "[" + countVar + "];";
                    in_arg  = "        " + var;
                    out_arg = "            " + var;
                }
                else {
                    cout << "unsupported type: " << type << endl;
                    exit(1);
                }
                in_args.push_back(in_arg);
                assignments.push_back(stmt);
                out_args.push_back(out_arg);
            }
        }
        catch (DataException &e) {
            e.print();
        }
        path.pop_back();
    }
}

void GenEgress::genEgressObj(Message *message, json j, vector<string> path, vector<string> &assignments,
        vector<string> &in_args, vector<string> &out_args)
{
    for (auto& el : j.items()) {
        string key = el.key();
        json val = el.value();

        path.push_back(key);
        try {
            string var(key);
            genVar(var);

            string type = getField(val, "type", message, path);
            if (type == "array") {
                genEgressArray(message, val["items"]["properties"], path,
                        assignments, in_args, out_args);
            }
            else if (type == "object") {
                genEgressObj(message, val["properties"], path, assignments,
                        in_args, out_args);
            }
            else {
                string in_arg;
                string stmt;
                string out_arg;
                string copy;

                if (type == "string") {
                    string maxLength = getField(val, "maxLength", message, path);
                    stmt = "char " + var + WCARD + "[" + maxLength + "];";
                    in_arg = "        " + var;
                    out_arg = var + "###";

                    copy = "strncpy(" + var + WCARD + ", " + var + ", " + maxLength + ");";
                }
                else if (type == "integer") {
                    stmt = "int " + var + WCARD + ";";
                    in_arg = "        &" + var;
                    out_arg = var + WCARD;

                    copy = var + WCARD + " = " + var + ";";
                }
                else if (type == "number") {
                    stmt = "double " + var + WCARD + ";";
                    in_arg = "        &" + var;
                    out_arg = var + WCARD;

                    copy = var + WCARD + "= " + var + ";";
                }
                else {
                    cout << "unsupported type: " << type << endl;
                    exit(1);
                }
                in_args.push_back(in_arg);
                assignments.push_back(stmt);
                out_args.push_back(out_arg);

                copies.push_back(copy);
            }
        }
        catch (DataException &e) {
            e.print();
        }
        path.pop_back();
    }
}

void GenEgress::genOneBranch(bool isElse, string msg_name, string component, vector<Flow *> flows, vector<string> assignments, vector<string> out_args)
{
    string msg_name_u = msg_name;
    boost::to_upper(msg_name_u);

    string component_u = component;
    boost::to_upper(component_u);

    if (isElse)
        genfile << "        else if (";
    else
        genfile << "        if (";

    bool first = true;
    for (auto flow : flows) {
        if (!first)
            genfile << " || ";
        genfile << "dataId == " << flow->getDataId();
        first = false;
    }
    genfile << ") {" << endl;

    genfile << "#pragma cle def begin " << msg_name_u << "_" << component_u + "_SHAREABLE" << endl;
    for (std::vector<string>::iterator it = assignments.begin(); it != assignments.end(); ++it) {
        string stmt = *it;
        findAndReplaceAll(stmt, WCARD, "_" + component);
        genfile << "            " << stmt << endl;
    }
    genfile << "#pragma cle def end " << msg_name_u << "_" << component_u + "_SHAREABLE" << endl;

    // copies
    for (std::vector<string>::iterator it = copies.begin(); it != copies.end(); ++it) {
        string stmt = *it;
        findAndReplaceAll(stmt, WCARD, "_" + component);
        genfile << "            " << stmt << endl;
    }

    genfile << "            echo_" << msg_name << "_" << component << "(" << endl;
    first = true;
    for (std::vector<string>::iterator it = out_args.begin(); it != out_args.end(); ++it) {
        if (!first)
            genfile << ",\n";
        string stmt = *it;
        findAndReplaceAll(stmt, WCARD, "_" + component);
        genfile << "                " << stmt;
        first = false;
    }
    genfile << endl
            << "            );" << endl;
    genfile << "        }" << endl;
}

void GenEgress::genEgress(Message *message)
{
   string msg_name = message->getName();

   std::ifstream schemaStream(message->getSchemaFile());
   if (schemaStream.fail()) {
       eprintf("%s does not exist", message->getSchemaFile().c_str());
       return;
   }
   json schemaJson;
   schemaStream >> schemaJson;
   schemaStream.close();

   try {
       copies.clear();

       vector<string> path;
       vector<string> assignments;
       vector<string> in_args;
       vector<string> out_args;

       string type = getField(schemaJson, "type", message, path);
       if (type == "array") {
           genEgressArray(message, schemaJson["items"]["properties"], path, assignments, in_args, out_args);
       }
       else if (type == "object") {
           genEgressObj(message, schemaJson["properties"], path, assignments, in_args, out_args);
       }
       else {
           throw DataException("Unsupported type: " + type);
       }

       genfile << "int egress_" + msg_name + "(const char *jstr)" << endl
               << "{" << endl
               << "    int fromRemote;" << endl
               << "    int dataId;"
               << endl;

       for (std::vector<string>::iterator it = assignments.begin(); it != assignments.end(); ++it) {
           string stmt = *it;
           findAndReplaceAll(stmt, WCARD, "");
           genfile << "    " << stmt << endl;
       }

       genfile << endl
               << "    if (_local_" + msg_name + ")" << endl
               << "        return;" << endl
               << endl;

       genfile << "    unmarshal_" + msg_name + "(" << endl
               << "        jstr," << endl
               << "        &fromRemote," << endl
               << "        &dataId";
       for (std::vector<string>::iterator it = in_args.begin(); it != in_args.end(); ++it) {
           genfile << ",\n" << *it;
       }
       genfile << endl
               << "    );" << endl;

       genfile << "    if (fromRemote == 0) {" << endl;

       bool isElse = false;
       map<string, vector<Flow *>> flows = message->getOutFlows();
       for (auto component : flows) {
           genOneBranch(isElse, msg_name, component.first, component.second, assignments, out_args);
           isElse = true;
       }
       genfile << "    }" << endl; // if (fromRemote...

       genfile << "    return 0;" << endl
               << "}" << endl
               << endl;
   }
   catch (DataException &e) {
       e.print();
   }
}

int GenEgress::gen(XdccFlow& xdccFlow)
{
    string enclave = config.getEnclave();
    for (auto const& message : myMessages) {
        genCommon((Message *) message);
        endOfFunc();

        map<string, vector<Flow *>> flows = message->getOutFlows();
        for (auto component : flows) {
            genXdcc((Message *)message, component.first);
            endOfFunc();
        }

        genEgress((Message *)message);
        endOfFunc();
    }
    return 0;
}

void GenEgress::populateRemoteEnclaves(const XdccFlow &xdccFlow)
{
    string enclave = config.getEnclave();
    remoteEnclaves.clear();

    for (auto const &msg_map : xdccFlow.getMessages()) {
        Message *message = (Message*) msg_map.second;
        message->clearOutFlow();
    }

    const map<string, Message*>& msg_map = xdccFlow.getMessages();
    for (auto const flow_map : xdccFlow.getFlows()) {
        Flow *flow = (Flow*) flow_map.second;
        string msgName = flow->getMessage();

        // add fromComponent to the message's senders
        map<string, Message *>::const_iterator it = msg_map.find(msgName);
        if (it == msg_map.end()) {
            eprintf("no such message: %s", msgName.c_str());
            continue;
        }
        Message *msg = (Message *) it->second;
        msg->addOutFlow(flow);

        // add remoteLevel to remoteEnclaves
        Cle *cle = xdccFlow.find_cle(flow);
        if (cle == NULL) {
            eprintf("no CLE for %s", msgName.c_str());
            continue;
        }

        CleJson cleJson = cle->getCleJson();
        string level = cleJson.getLevel();
        vector<Cdf> cdf = cleJson.getCdf();
        for (int i = 0; i < cdf.size(); i++) {
            if (level.compare(enclave))  // not flowing from my enclave
                continue;

            string remote = cdf[i].getRemoteLevel();
            if (remote.compare(enclave)) { // flow to a different enclave
                remoteEnclaves.insert(remote);
            }
        }
    }

    if (debug) {
        for (auto const &msg_map : xdccFlow.getMessages()) {
            Message *message = (Message*) msg_map.second;
            string msgName = message->getName();

            cout << "message " << msgName << ": " << endl;
            map<string, vector<Flow *>> outFlows = message->getOutFlows();
            for (auto component : outFlows) {
                cout << "\tfrom " << component.first << ": ";
                for (auto& id : component.second) {
                    cout << id->getDataId() << ", ";
                }
                cout << endl;
            }
        }

        cout << "local encalve: " << enclave << ", remote enclaves: ";
        for (auto e : remoteEnclaves) {
            cout << e << ", ";
        }
        cout << endl;
    }
}

static void genShareables(const XdccFlow &xdccFlow, ofstream& genFile)
{
    string enclave = config.getEnclave();

    for (auto const &msg_map : xdccFlow.getMessages()) {
        Message *message = (Message*) msg_map.second;
        string msgName = message->getName();

        map<string, vector<Flow *>> flows = message->getOutFlows();
        for (auto component : flows) {
            string sender = component.first;
        }

        set<string> remoteEnclaves;
        for (auto const flow_map : xdccFlow.getFlows()) {
            Flow *flow = (Flow*) flow_map.second;
            if (flow->getMessage().compare(msgName))
                continue;

            Cle *cle = xdccFlow.find_cle(flow);
            if (cle == NULL) {
                cerr << __FUNCTION__ << ": no CLE for " << msgName << endl;
                continue;
            }

            CleJson cleJson = cle->getCleJson();
            string level = cleJson.getLevel();
            vector<Cdf> cdf = cleJson.getCdf();
            for (int i = 0; i < cdf.size(); i++) {
                if (level.compare(enclave))  // not flowing from my enclave
                    continue;

                string remote = cdf[i].getRemoteLevel();
                if (remote.compare(enclave)) { // flow to a different enclave
                    if (remoteEnclaves.find(remote) == remoteEnclaves.end()) { // not generated
                        remoteEnclaves.insert(remote);

                        string remote_u = remote;
                        boost::to_upper(remote_u);
                        string msgName_u = msgName;
                        boost::to_upper(msgName_u);
                        json clejson;
                        to_json(clejson, cdf[i]);

                        string clestr = clejson.dump(4);
                        findAndReplaceAll(clestr, "\n", " \\\n");
                        genFile << "#pragma cle def "
                                << msgName_u << "_" << remote_u
                                << "_SHAREABLE " << clestr << endl << endl;
                    }
                }
            }
        }
    }
}

static void genXDLinkages(const XdccFlow &xdccFlow, ofstream& genFile)
{
    string enclave = config.getEnclave();

    set<string> remoteEnclaves;
    for (auto const flow_map : xdccFlow.getFlows()) {
        Flow *flow = (Flow*) flow_map.second;
        if (remoteEnclaves.find(flow->getLabel()) != remoteEnclaves.end()) // already generated
            continue;
        remoteEnclaves.insert(flow->getLabel());

        string fromComponent = flow->getFromComponent();
        string msgName = flow->getMessage();
        Cle *cle = xdccFlow.find_cle(flow);
        if (cle == NULL) {
            eprintf("no CLE for %s", msgName.c_str());
            continue;
        }

        CleJson cleJson = cle->getCleJson();
        string level = cleJson.getLevel();
        if (level.compare(enclave))  // not flowing from my enclave
            continue;

        vector<Cdf> cdf = cleJson.getCdf();
        for (int i = 0; i < cdf.size(); i++) {
            string remote = cdf[i].getRemoteLevel();

            string remote_u = remote;
            boost::to_upper(remote_u);
            string msgName_u = msgName;
            boost::to_upper(msgName_u);
            json clejson;
            to_json(clejson, cdf[i]);

            string clestr = clejson.dump(4);
            findAndReplaceAll(clestr, "\n", " \\\n");

            genFile << "#pragma cle def XDLINKAGE_ECHO_"
                    << msgName_u << "_" << fromComponent << "_" << remote_u << " "
                    << clestr << endl << endl;
        }
    }
}

void GenEgress::annotations(const XdccFlow &xdccFlow)
{
    shares.clear();

    string my_enclave = config.getEnclave();

    string my_enclave_u = my_enclave;
    boost::to_upper(my_enclave_u);

    genfile << "#pragma cle def " + my_enclave_u + " {\"level\":\"" + my_enclave + "\"}" << endl << endl;

    populateRemoteEnclaves(xdccFlow);
    genShareables(xdccFlow, genfile);
    genXDLinkages(xdccFlow, genfile);
}

int GenEgress::open(const XdccFlow &xdccFlow)
{
    genfile
      << "#include <stdlib.h>" << endl
      << "#include <string.h>" << endl
      << "#include <unistd.h>" << endl << endl
      << "#include \"amqlib.h\"" << endl
      << "#include \"xdcc_echo.h\"" << endl
      << "#include \"map.h\"" << endl << endl
//      << "/* XXX: Need to create CLE-JSON definitions here */" << endl << endl
//      << "amqlib_t *amq() { static amqlib_t *a = NULL; if (a == NULL) { a = amqlib_create(); } return a; }\n" << endl
      ;

    annotations(xdccFlow);

    genfile << "/* Messages in system */" << endl;
    genfile << "#define ALL_MSGS_LIST";
    bool first = true;
    for (auto const& message : myMessages) {
        string msgName = message->getName();
        if (!first)
            genfile << ",";
        genfile << " \\\n    " << msgName;
        first = false;
    }
    genfile << endl << endl;

    genfile << "/* _local_X is 1 if X is local, else 0 */\n";
    for (auto const& message : myMessages) {
        genfile << "#define _local_" + message->getName() + " " + (message->isLocal() ? "1" : "0") << endl;
    }
    genfile << endl;

    genfile << "/* _topic_X is 1 if X is a msg_name, else 0 */\n";
    for (auto const& message : myMessages) {
        genfile << "#define _topic_" + message->getName() + " " + (message->isTopic() ? "1" : "0") << endl;
    }
    genfile << endl;

    return 0;
}

int GenEgress::close()
{
   genfile
      << "#define XDCCLISTEN(X) amqlib_listen(amq(), #X, egress_##X, _topic_##X);" << endl
      << "int main()" << endl
      << "{" << endl
      ;

   set<string> gened;
   for (auto const& x : shares) {
       if (gened.find(x.second) != gened.end()) {
           continue;
       }
       gened.insert(x.second);
       genfile << "#pragma cle def begin " + x.second + "_SHAREABLE" << endl;
   }
   genfile << "    int i = 100;" << endl;
   gened.clear();
   for (auto const& x : shares) {
       if (gened.find(x.second) != gened.end()) {
           continue;
       }
       gened.insert(x.second);
       genfile << "#pragma cle end begin " + x.second + "_SHAREABLE" << endl;
   }

   genfile
      << "    amq();" << endl
      << "    MAP(XDCCLISTEN, ALL_MSGS_LIST)" << endl << endl
      << "    while(1) {" << endl
      << "        sleep(i);" << endl
      << "    }" << endl << endl
      << "    amqlib_destroy(amq());" << endl
      << "    return 0; " << endl
      << "}" << endl
      ;
    return 0;
}
