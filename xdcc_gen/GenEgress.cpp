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
const string CPP = "_cpp";
const int INDENT = 4;

#define TAB_1 string(INDENT, ' ')
#define TAB_2 string(2 * INDENT, ' ')
#define TAB_3 string(3 * INDENT, ' ')

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

void to_json(json& j, const CleJson& p) {
    vector<Cdf> cdf = p.getCdf();
    json jcdf = cdf;

    string level = p.getLevel();

    j = json{
            {"level", level },
            {"cdf", jcdf },
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
void GenEgress::traverseArrayEcho(Message *message, json j, vector<string> path, string numElements)
{
    string countVar = "count";
    genVar(countVar);
    stmts.push_back("int " + countVar + " = " + numElements + "; // numElements from schema");

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
                string numElements = getField(val, "numElements", message, path);
                traverseArrayEcho(message, val["items"]["properties"], path, numElements);
            }
            else if (type == "object") {
                travereObjEcho(message, j["properties"], path);
            }
            else {
                string in_arg;
                string out_arg = var;

                if (type == "string") {
                    in_arg = "const char *" + var + "[]";

                    string maxLength = getField(val, "maxLength", message, path);
                    stmts.push_back("char " + var + CPP + "[" + countVar + "][" + maxLength + "];");
                    stmts.push_back("for (int j = 0; j < " + countVar + "; j++)");
                    stmts.push_back("    memcpy(" + var + CPP + "[j], " + var + "[j], " + maxLength + ");\n");

                    out_arg = var + CPP;
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
//        var_count -= inc;
//        varSet.erase(key);
    }
}

void GenEgress::travereObjEcho(Message *message, json j, vector<string> path)
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
                string numElements = getField(val, "numElements", message, path);
                traverseArrayEcho(message, val["items"]["properties"], path, numElements);
            }
            else if (type == "object") {
                travereObjEcho(message, val["properties"], path);
            }
            else {
                string in_arg;
                string out_arg = var;
                if (type == "string") {
                    in_arg = "const char *" + var;

                    string maxLength = getField(val, "maxLength", message, path);

                    string stmt = "char " + var + CPP + "[" + maxLength + "];";
                    stmts.push_back(stmt);

                    stmt = "memcpy(" + var + CPP + ", " + var + ", " + maxLength + ");\n";
                    stmts.push_back(stmt);

                    out_arg = var + CPP;
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

void GenEgress::traverseEcho(Message *message)
{
    try {
        json schemaJson;
        beginFunc(message, schemaJson);

        vector<string> path;

        string type = getField(schemaJson, "type", message, path);
        if (type == "array") {
            string numElements = getField(schemaJson, "numElements", message, path);
            traverseArrayEcho(message, schemaJson["items"]["properties"], path, numElements);
        }
        else if (type == "object") {
            travereObjEcho(message, schemaJson["properties"], path);
        }
        else {
            throw DataException("unsupported type: " + type + " for " + message->getName());
        }
    }
    catch (DataException &e) {
    }
}

void GenEgress::genEchoCommon(Message *message, bool singleRemote)
{
    string msg_name = message->getName();

    try {
        string msg_name_u = msg_name;

        if (singleRemote) {
            boost::to_upper(msg_name_u);
            genfile << "#pragma cle begin XDLINKAGE_ECHO_" << msg_name_u << endl;
        }

        string suffix = singleRemote ? "(" : "_common(";
        genfile << "int echo_" << msg_name << suffix;

        bool first = true;
        for (std::vector<string>::iterator it = in_args.begin(); it != in_args.end(); ++it) {
            if (!first) {
                genfile << ",";
            }
            else {
                first = false;
            }
            genfile << "\n" << TAB_1 << *it;
        }
        genfile << endl
                << ")" << endl
                << "{" << endl;
        if (singleRemote) {
            genfile << "#pragma cle end XDLINKAGE_ECHO_" << msg_name_u << endl;
        }

        for (std::vector<string>::iterator it = stmts.begin(); it != stmts.end(); ++it) {
            genfile << TAB_1 << *it << endl;
        }

        genfile << TAB_1 << "echo_" + msg_name + CPP + "(\n"
                << TAB_2 << "amq(),\n"
                << TAB_2 << "_topic_" + msg_name;
        for (std::vector<string>::iterator it = out_args.begin(); it != out_args.end(); ++it) {
            genfile << ",";
            genfile << "\n" << TAB_2 << *it;
        }
        genfile << "\n" << TAB_1 << ");\n";

        genfile << TAB_1 << "return 0;" << endl
                << "}" << endl
                << endl;
    }
    catch (DataException &e) {
        e.print();
    }
}

void GenEgress::genEcho(Message *message, string remote)
{
    try {
        string msg_name = message->getName();

        string combo = msg_name + "_" + remote;
        string combo_u = combo;
        boost::to_upper(combo_u);
        genfile << "#pragma cle begin XDLINKAGE_ECHO_" << combo_u << endl
                << "int echo_" << combo << "(";

        bool first = true;
        for (std::vector<string>::iterator it = in_args.begin(); it != in_args.end(); ++it) {
            if (!first) {
                genfile << ",";
            }
            else {
                first = false;
            }
            genfile << "\n" << TAB_1 << *it;
        }
        genfile << endl
                << ")" << endl
                << "#pragma cle end XDLINKAGE_ECHO_" << combo_u << endl
                << "{" << endl;

        genfile << TAB_1 << "echo_" + msg_name + "_common(";
        first = true;
        for (std::vector<string>::iterator it = out_args.begin(); it != out_args.end(); ++it) {
            string stmt = *it;
            findAndReplaceAll(stmt, CPP, "");

            if (!first)
                genfile << ",";
            genfile << "\n" << TAB_2 << stmt;
            first = false;
        }
        genfile << "\n" << TAB_1 << ");\n";

        genfile << TAB_1 << "return 0;" << endl
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
void GenEgress::traverseArrayEgress(Message *message, json j, vector<string> path, string numElements)
{
    string countVar = "count";
    genVar(countVar);
    in_args.push_back(countVar);

    stmts.push_back("int " + countVar + " = " + numElements + "; // numElements from schema");

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
                string numElements = getField(val, "numElements", message, path);
                traverseArrayEgress(message, val["items"]["properties"], path, numElements);
            }
            else if (type == "object") {
                traverseObjEgress(message, val["properties"], path);
            }
            else {
                string in_arg = var;
                string stmt;
                string out_arg = var;

                if (type == "string") {
                    string maxLength = getField(val, "maxLength", message, path);

                    stmt    = "char " + var + "[" + countVar + "][" + maxLength + "];";
                }
                else if (type == "integer") {
                    stmt    = "int " + var + "[" + countVar + "];";
                }
                else if (type == "number") {
                    stmt    = "double " + var + "[" + countVar + "];";
                }
                else {
                    cout << "unsupported type: " << type << endl;
                    exit(1);
                }
                in_args.push_back(in_arg);
                stmts.push_back(stmt);
                out_args.push_back(out_arg);
            }
        }
        catch (DataException &e) {
            e.print();
        }
        path.pop_back();
    }
}

void GenEgress::traverseObjEgress(Message *message, json j, vector<string> path)
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
                string numElements = getField(val, "numElements", message, path);
                traverseArrayEgress(message, val["items"]["properties"], path, numElements);
            }
            else if (type == "object") {
                traverseObjEgress(message, val["properties"], path);
            }
            else {
                string in_arg;
                string stmt;
                string out_arg;
                string copy;

                if (type == "string") {
                    string maxLength = getField(val, "maxLength", message, path);
                    stmt = "char " + var + WCARD + "[" + maxLength + "];";
                    in_arg = var;
                    out_arg = var + WCARD;

                    copy = "strncpy(" + var + WCARD + ", " + var + ", " + maxLength + ");";
                }
                else if (type == "integer") {
                    stmt = "int " + var + WCARD + ";";
                    in_arg = "&" + var;
                    out_arg = var + WCARD;

                    copy = var + WCARD + " = " + var + ";";
                }
                else if (type == "number") {
                    stmt = "double " + var + WCARD + ";";
                    in_arg = "&" + var;
                    out_arg = var + WCARD;

                    copy = var + WCARD + "= " + var + ";";
                }
                else {
                    cout << "unsupported type: " << type << endl;
                    exit(1);
                }
                in_args.push_back(in_arg);
                stmts.push_back(stmt);
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

void GenEgress::traverseEgress(Message *message)
{
   try {
       json schemaJson;
       beginFunc(message, schemaJson);

       vector<string> path;

       string type = getField(schemaJson, "type", message, path);
       if (type == "array") {
           string numElements = getField(schemaJson, "numElements", message, path);
           traverseArrayEgress(message, schemaJson["items"]["properties"], path, numElements);
       }
       else if (type == "object") {
           traverseObjEgress(message, schemaJson["properties"], path);
       }
       else {
           throw DataException("Unsupported type: " + type);
       }
   }
   catch (DataException &e) {
   }
}

void GenEgress::beginFunc(Message *message, json& schemaJson)
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

void GenEgress::genFlowToRemote(string msg_name, string remote)
{
    genfile << TAB_1 << "{" << endl;

    string key = remote + "_SHAREABLE";
    boost::to_upper(key);
    string suffix = "_" + remote;

    genfile << "#pragma cle def begin " <<  key << endl;
    for (std::vector<string>::iterator it = stmts.begin(); it != stmts.end(); ++it) {
        string stmt = *it;
        findAndReplaceAll(stmt, WCARD, suffix);
        genfile << TAB_2 << stmt << endl;
    }
    genfile << "#pragma cle def end " << key << endl
            << endl;
    // copies
    for (std::vector<string>::iterator it = copies.begin(); it != copies.end(); ++it) {
        string stmt = *it;
        findAndReplaceAll(stmt, WCARD, suffix);
        genfile << TAB_2 << stmt << endl;
    }
    genfile << endl;

    genfile << TAB_2 << "echo_" << msg_name << "_" << remote << "(" << endl;
    bool first = true;
    for (std::vector<string>::iterator it = out_args.begin(); it != out_args.end(); ++it) {
        if (!first)
            genfile << ",\n";
        string stmt = *it;
        findAndReplaceAll(stmt, WCARD, suffix);
        genfile << TAB_3 << stmt;
        first = false;
    }
    genfile << endl
            << TAB_2 << ");" << endl;
    genfile << TAB_1 << "}" << endl;
}

void GenEgress::genEgress(Message *message)
{
   try {
       string msg_name = message->getName();
       bool singleRemote = false;
       set<string> remotes;

       map<string, set<string>>::iterator it = msgToEnclaves.find(msg_name);
       if (it == msgToEnclaves.end()) {
           singleRemote = true;
       }
       else {
           remotes = it->second;
           singleRemote = (remotes.size() == 1);
       }
       // if the message flows to only one remote enclave, then
       // there is no need to fan out (echo_*_common() and related are not generated

       genfile << "int egress_" + msg_name + "(const char *jstr)" << endl
               << "{" << endl;

       if (message->isLocal()) {
           genfile << TAB_1 << "return 0;" << endl
                   << "}" << endl
                   << endl;
           return;
       }

       genfile << TAB_1 << "int fromRemote;" << endl
               << endl;

       string key;
       if (singleRemote) {
           string theRemote;
           for (auto const remote : remotes) {
               key = remote + "_SHAREABLE";
               boost::to_upper(key);
           }
           genfile << "#pragma cle def begin " <<  key << endl;
       }
       for (std::vector<string>::iterator it = stmts.begin(); it != stmts.end(); ++it) {
           string stmt = *it;
           findAndReplaceAll(stmt, WCARD, "");
           genfile << TAB_1 << stmt << endl;
       }
       if (singleRemote)
           genfile << "#pragma cle def end " <<  key << endl;


       genfile << endl
               << TAB_1 << "if (_local_" + msg_name + ")" << endl
               << TAB_2 << "return 0;" << endl
               << endl;

       genfile << TAB_1 << "unmarshal_" + msg_name + "(" << endl
               << TAB_2 << "jstr," << endl
               << TAB_2 << "&fromRemote";
       for (std::vector<string>::iterator it = in_args.begin(); it != in_args.end(); ++it) {
           genfile << ",\n" << TAB_2 << *it;
       }
       genfile << endl
               << TAB_1 << ");" << endl;


       if (singleRemote) {
           genfile << TAB_1 << "if (fromRemote == 0) {" << endl;
           genfile << TAB_2 << "echo_" << msg_name << "(" << endl;
           bool first = true;
           for (std::vector<string>::iterator it = out_args.begin(); it != out_args.end(); ++it) {
               if (!first)
                   genfile << ",\n";
               string stmt = *it;
               findAndReplaceAll(stmt, WCARD, "");
               genfile << TAB_3 << stmt;
               first = false;
           }
           genfile << endl << TAB_2 << ");" << endl;
           genfile << TAB_1 << "}" << endl;
       }
       else { // need separate annotations
           genfile << TAB_1 << "if (fromRemote != 0)" << endl
                   << TAB_2 << "return 0;" << endl
                   << endl;

           string my_enclave = config.getEnclave();
           for (auto const remote : remotes) {
               if (my_enclave.compare(remote))
                   genFlowToRemote(msg_name, remote);
           }
       }
       genfile << TAB_1 << "return 0;" << endl
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

    for (auto const &msg_map : xdccFlow.getMessages()) {
        Message *message = (Message*) msg_map.second;
        traverseEcho(message);

        string msg_name = message->getName();

        map<string, set<string>>::iterator it = msgToEnclaves.find(msg_name);
        if (it != msgToEnclaves.end()) {
            int remotesSize = it->second.size();
            if (remotesSize > 0) {
                bool singleRemote = (remotesSize == 1);

                genEchoCommon(message, singleRemote);
                endOfFunc();

                for (auto const remote : it->second) {
                    if (!enclave.compare(remote))
                        continue;
                    if (!singleRemote) {
                        genEcho(message, remote);
                        endOfFunc();
                    }
                }
            }
        }
        traverseEgress(message);
        genEgress(message);
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
            eprintf("no such message in the \"messages\" section: %s", msgName.c_str());
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

            const map<string, set<string>>::const_iterator& it = msgToEnclaves.find(msgName);
             if (it == msgToEnclaves.end()) {
                 msgToEnclaves.insert(make_pair(msgName, set<string>()));
             }
             if (remote.compare(enclave)) {
                 msgToEnclaves[msgName].insert(remote);
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

void GenEgress::genCombo(const XdccFlow& xdccFlow)
{
    string enclave = config.getEnclave();

    set<string> genedLabels;
    for (auto const flow_map : xdccFlow.getFlows()) {
        Flow *flow = (Flow*) flow_map.second;
        if (genedLabels.find(flow->getLabel()) != genedLabels.end()) // already generated
            continue;
        genedLabels.insert(flow->getLabel());

        string fromComponent = flow->getFromComponent();
        string msgName = flow->getMessage();


        map<string, set<string>>::iterator it = msgToEnclaves.find(msgName);
        if (it == msgToEnclaves.end()) {
            continue;
        }
        set<string> remotes = it->second;
        bool singleRemote = (remotes.size() == 1);

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
        if (cdf.size() > 1) {
            eprintf("more than one CDFs %s", cle->getLabel().c_str());
        }

        for (int i = 0; i < cdf.size(); i++) {
            string remote = cdf[i].getRemoteLevel();

            json clejson;
            to_json(clejson, cdf[i]);

            string clestr = clejson.dump(2);
            findAndReplaceAll(clestr, "\n", " \\\n");

            if (!remote.compare(enclave)) {  // same enclave
                continue;
            }
            json cjs;
            to_json(cjs, cleJson);
            string cdfstr = cjs.dump(2);
            //string cdfstr = "{\"level\": \"" + enclave + "\", \"cdf\": [ " + clestr + "]}";
            findAndReplaceAll(cdfstr, "\n", " \\\n");

            string key = msgName + (singleRemote ? "" : ("_" + remote));
            combo[key] = cdfstr;
        }
    }
}

void GenEgress::annotations(const XdccFlow &xdccFlow)
{
    string my_enclave = config.getEnclave();

    string my_enclave_u = my_enclave;
    boost::to_upper(my_enclave_u);

    genfile << "#pragma cle def " + my_enclave_u + " {\"level\":\"" + my_enclave + "\"}" << endl << endl;

    for (auto remote : remoteEnclaves) {
        string remote_u = remote;
        boost::to_upper(remote_u);

        genfile << "#pragma cle def " << remote_u << "_SHAREABLE {" << endl
                << "  \"level\": \"" << remote << "\",\\" << endl
                << "  \"cdf\": [\\" << endl
                << "    {\"remotelevel\":\"" << my_enclave << "\", \\" << endl
                << "     \"direction\": \"egress\", \\" << endl
                << "     \"guarddirective\": { \"operation\": \"allow\"}}\\" << endl
                << "] }" << endl;
    }
    genfile << endl;

    for (auto const c : combo) {
        string key = c.first;
        boost::to_upper(key);

        genfile << "#pragma cle def XDLINKAGE_ECHO_"
                << key << " "
                << c.second << endl << endl;
    }
}

int GenEgress::open(const XdccFlow &xdccFlow)
{
    remoteEnclaves.clear();

    setXdccFlow(xdccFlow);
    populateRemoteEnclaves(xdccFlow);
    genCombo(xdccFlow);

    genfile
      << "#include <stdlib.h>" << endl
      << "#include <string.h>" << endl
      << "#include <unistd.h>" << endl << endl
      << "#include \"amqlib.h\"" << endl
      << "#include \"xdcc_echo.h\"" << endl
      << "#include \"map.h\"" << endl << endl
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
    for (auto message : myMessages) {
        bool local = true;
        string msg_name = message->getName();
        map<string, set<string>>::iterator it = msgToEnclaves.find(msg_name);
        if (it != msgToEnclaves.end() && it->second.size() > 0)
            local = false;

        ((Message *)message)->setLocal(local);
        genfile << "#define _local_" + msg_name + " " + (local ? "1" : "0") << endl;
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

   for (auto remote : remoteEnclaves) {
       string remote_u = remote;
       boost::to_upper(remote_u);

       genfile << "#pragma cle begin " << remote_u << "_SHAREABLE" << endl;
       genfile << "    int i = 100;" << endl;
       genfile << "#pragma cle end " << remote_u << "_SHAREABLE" << endl;
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
