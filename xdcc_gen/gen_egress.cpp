#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <exception>
#include <unordered_map>
#include <set>
#include <unistd.h>
#include <dirent.h>
#include <getopt.h>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using json = nlohmann::json;

#include "util.h"
#include "gen_egress.h"
#include "Config.h"
#include "XdccException.h"

extern Config config;
Cdf badCdf;

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
    std::vector<Cdf> x = p.getCdf();
    json j2 = x;

    j = json{
            {"level", p.getLevel()},
            {"cdf", j2}
    };
}

/******************************
 * XDCC
 */
void GenEgress::gen_xdcc_array(Message *message, json j, vector<string> path, vector<string> &assignments,
                      vector<string> &in_args, vector<string> &out_args)
{
    string countVar = "count";
    gen_var(countVar);
    assignments.push_back("    int " + countVar + " = 1; // TODO: get json array length");

    int i = 0;
    for (auto& el : j.items()) {
        string key = el.key();
        json val = el.value();

        string var(key);
        int inc = gen_var(var);

        path.push_back(key);

        try {
            string type = get_field(val, "type", message, path);
            if (type == "array") {
                gen_xdcc_array(message, val["items"]["properties"], path, assignments, in_args, out_args);
            }
            else if (type == "object") {
                gen_xdcc_obj(message, j["properties"], path, assignments, in_args, out_args);
            }
            else {
                string stmt;
                string arg;
                string out_arg = var;

                if (type == "string") {
                    arg = "const char *" + var + "[]";

                    string maxLength = get_field(val, "maxLength", message, path);
                    assignments.push_back("    char " + var + "_cpp[" + countVar + "][" + maxLength + "];");
                    assignments.push_back("    for (int j = 0; j < " + countVar + "; j++)");
                    assignments.push_back("        memcpy(" + var + "_cpp[j], " + var + "[j], " + maxLength + ");\n");

                    out_arg = var + "_cpp";
                }
                else if (type == "integer") {
                    arg = "int " + var + "[]";
                }
                else if (type == "number") {
                    arg = "double " + var + "[]";
                }
                else {
                    throw DataException("unsupported type " + type + " for " + gen_path(path));
                }
                in_args.push_back(arg);
                out_args.push_back(out_arg);
            }
        }
        catch (DataException e) {
            e.print();
        }

        path.pop_back();
        var_count -= inc;
        varSet.erase(key);
    }
}

void GenEgress::gen_xdcc_obj(Message *message, json j, vector<string> path, vector<string> &assignments,
                    vector<string> &in_args, vector<string> &out_args)
{
    for (auto& el : j.items()) {
        string key = el.key();
        json val = el.value();

        path.push_back(key);

        try {
            string var(key);
            gen_var(var);

            string type = get_field(val, "type", message, path);
            if (type == "array") {
                gen_xdcc_array(message, val["items"]["properties"], path, assignments, in_args, out_args);
            }
            else if (type == "object") {
                gen_xdcc_obj(message, val["properties"], path, assignments, in_args, out_args);
            }
            else {
                string arg;
                string out_arg = var;
                string stmt;
                if (type == "string") {
                    arg = "const char *" + var;

                    string maxLength = get_field(val, "maxLength", message, path);

                    stmt = "    char " + var + "_cpp[" + maxLength + "];";
                    assignments.push_back(stmt);

                    stmt = "    memcpy(" + var + "_cpp, " + var + ", " + maxLength + ");\n";
                    assignments.push_back(stmt);

                    out_arg = var + "_cpp";
                }
                else if (type == "integer") {
                    arg = "int " + var;
                }
                else if (type == "number") {
                    arg = "double " + var;
                }
                else {
                    throw DataException("unsupported type " + type + " for " + gen_path(path));
                }
                in_args.push_back(arg);
                out_args.push_back(out_arg);
            }
        }
        catch (DataException e) {
            e.print();
        }
        path.pop_back();
    }
}

void GenEgress::gen_xdcc(Message *message)
{
   string topic = message->getName();

   std::ifstream schemaStream(message->getSchema());
   json schemaJson;
   schemaStream >> schemaJson;

   try {
       vector<string> path;
       string type = get_field(schemaJson, "type", message, path);
       vector<string> assignments;
       vector<string> in_args;
       vector<string> out_args;

       if (type == "array") {
           gen_xdcc_array(message, schemaJson["items"]["properties"], path, assignments, in_args, out_args);
       }
       else if (type == "object") {
           gen_xdcc_obj(message, schemaJson["properties"], path, assignments, in_args, out_args);
       }
       else {
           throw DataException("unsupported type: " + type + " for " + message->getName());
       }

       string upper_topic = topic;
       boost::to_upper(upper_topic);
       genfile << "#pragma cle begin XDLINKAGE_ECHO_" << upper_topic << endl
               << "int echo_" << topic << "(";

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
               << "#pragma cle end XDLINKAGE_ECHO_" << upper_topic << endl
               << "{" << endl;

       for (std::vector<string>::iterator it = assignments.begin(); it != assignments.end(); ++it) {
           genfile << *it << endl;
       }

       genfile << "    echo_" + topic + "_cpp(\n"
               << "        amq(),\n"
               << "        _topic_" + topic;
       for (std::vector<string>::iterator it = out_args.begin(); it != out_args.end(); ++it) {
           genfile << ",";
           genfile << "\n        " << *it;
       }
       genfile << "\n    );\n";

       genfile << "    return 0;" << endl
               << "}" << endl
               << endl;
   }
   catch (DataException e) {
       e.print();
   }
}

/******************************
 * egress
 */
void GenEgress::gen_egress_array(Message *message, json j, vector<string> path,
        vector<string> &assignments, vector<string> &in_args, vector<string> &out_args)
{
    string countVar = "count";
    gen_var(countVar);
    in_args.push_back("        " + countVar);

    assignments.push_back("    int " + countVar + " = 1; // TODO: get json array length");

    int i = 0;
    for (auto& el : j.items()) {
        string key = el.key();
        json val = el.value();

        string var(key);
        gen_var(var);

        path.push_back(key);
        try {
            string type = get_field(val, "type", message, path);
            if (type == "array") {
                gen_egress_array(message, val["items"]["properties"], path, assignments, in_args, out_args);
            }
            else if (type == "object") {
                gen_egress_obj(message, val["properties"], path, assignments, in_args, out_args);
            }
            else {
                string in_arg;
                string stmt;
                string out_arg;

                if (type == "string") {
                    string maxLength = get_field(val, "maxLength", message, path);

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
        catch (DataException e) {
            e.print();
        }
        path.pop_back();
    }
}

void GenEgress::gen_egress_obj(Message *message, json j, vector<string> path, vector<string> &assignments,
        vector<string> &in_args, vector<string> &out_args)
{
    for (auto& el : j.items()) {
        string key = el.key();
        json val = el.value();

        path.push_back(key);
        try {
            string var(key);
            gen_var(var);

            string type = get_field(val, "type", message, path);
            if (type == "array") {
                gen_egress_array(message, val["items"]["properties"], path,
                        assignments, in_args, out_args);
            }
            else if (type == "object") {
                gen_egress_obj(message, val["properties"], path, assignments,
                        in_args, out_args);
            }
            else {
                string in_arg;
                string stmt;
                string out_arg;

                if (type == "string") {
                    string maxLength = get_field(val, "maxLength", message, path);
                    stmt = "    char " + var + "[" + maxLength + "];";
                    in_arg = "        " + var;
                    out_arg = "            " + var;
                }
                else if (type == "integer") {
                    stmt = "    int " + var + ";";
                    in_arg = "        &" + var;
                    out_arg = "            " + var;
                }
                else if (type == "number") {
                    stmt = "    double " + var + ";";
                    in_arg = "        &" + var;
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
        catch (DataException e) {
            e.print();
        }
        path.pop_back();
    }
}

void GenEgress::gen_egress(Message *message)
{
   string topic = message->getName();

   std::ifstream schemaStream(message->getSchema());
   json schemaJson;
   schemaStream >> schemaJson;

   try {
       vector<string> path;
       string type = get_field(schemaJson, "type", message, path);
       vector<string> assignments;
       vector<string> in_args;
       vector<string> out_args;

       if (type == "array") {
           gen_egress_array(message, schemaJson["items"]["properties"], path, assignments, in_args, out_args);
       }
       else if (type == "object") {
           gen_egress_obj(message, schemaJson["properties"], path, assignments, in_args, out_args);
       }
       else {
           throw DataException("Unsupported type: " + type);
       }

       genfile << "int egress_" + topic + "(const char *jstr)" << endl
               << "{" << endl
               << "    int fromRemote;" << endl;

       string share = "";
       map<string, string>::iterator it = shares.find(topic);
       if (it != shares.end()) {
           share = it->second;
       }
       if (!share.empty()) {
           genfile << "#pragma cle def begin " + share + "_SHAREABLE" << endl;
       }
       for (std::vector<string>::iterator it = assignments.begin(); it != assignments.end(); ++it) {
           genfile << *it << endl;
       }
       if (!share.empty()) {
           genfile << "#pragma cle def end " + share + "_SHAREABLE" << endl;
       }

       genfile << endl
               << "    if (_local_" + topic + ")" << endl
               << "        return;" << endl
               << endl;

       genfile << "    unmarshal_" + topic + "(" << endl
               << "        jstr," << endl
               << "        &fromRemote";
       for (std::vector<string>::iterator it = in_args.begin(); it != in_args.end(); ++it) {
           genfile << ",\n" << *it;
       }
       genfile << endl
               << "    );" << endl;

       genfile << "    if (fromRemote == 0)" << endl
               << "        echo_" + topic + "(" << endl;
       bool first = true;
       for (std::vector<string>::iterator it = out_args.begin(); it != out_args.end(); ++it) {
           if (!first)
               genfile << ",\n";
           genfile << *it;
           first = false;
       }
       genfile << endl
               << "        );" << endl;

       genfile << "    return 0;" << endl
               << "}" << endl
               << endl;
   }
   catch (DataException e) {
       e.print();
   }
}

int GenEgress::generate(XdccFlow& xdccFlow)
{
    string enclave = config.getEnclave();
    for (auto const& x : xdccFlow.getMessages()) {
        if (!is_interested(x.first, enclave))
            continue;

        Message *message = (Message *)x.second;

        gen_xdcc(message);
        endOfFunc();

        gen_egress(message);
        endOfFunc();
    }
    return 0;
}

void GenEgress::annotations(const XdccFlow &xdccFlow)
{
    shares.clear();

    string myEnclave = config.getEnclave();

    string upper_my_enclave = myEnclave;
    boost::to_upper(upper_my_enclave);

    genfile << "#pragma cle def " + upper_my_enclave + " {\"level\":\"" + myEnclave + "\"}" << endl << endl;

    // generate shareables
    map<string, string> components = xdccFlow.getComponents();
    set<string> remote_enclaves;
    for (auto const& x : xdccFlow.getMessages()) {
        // interested only in messages flows from this enclave
        if (!is_interested(x.first, myEnclave))
            continue;

        Message *message = (Message *) x.second;
        // find remote enclaves the message is to be shared, to prepare to generate the SHAREABLEs
        for (auto &flow : message->getFlows()) {
            map<string, string>::iterator it = components.find(flow.getDestination());
            if (it != components.end() && it->second.compare(upper_my_enclave)) {
                remote_enclaves.insert(it->second);
                shares[message->getName()] = it->second;
            }
        }
    }

    map<string, Cle *> cles = xdccFlow.getCles();
    for (auto const& x : remote_enclaves) {
        Cdf *cdf = xdccFlow.find_cle(x, myEnclave);
        if (cdf == NULL) {
            cout << "SHAREABLE: Could not find CDF for level/remote : " << x << "/" << myEnclave << endl;
            continue;
        }

        string lower_x = x;
        boost::to_lower(lower_x);
        Cle tmpCle(lower_x, cdf);
        json clejson;
        to_json(clejson, tmpCle);
               
        string clestr = clejson.dump(4);
        findAndReplaceAll(clestr, "\n", " \\\n");
        genfile << "#pragma cle def " + x + "_SHAREABLE " << clestr << endl << endl;
    }

    // generate xdlinkages
    set<string> gened;
    for (auto const& x : xdccFlow.getMessages()) {
        if (!is_interested(x.first, myEnclave))
            continue;

        Message *message = (Message *) x.second;
        std::map<string, Cle *>::iterator it = cles.find(message->getCle());
        if (it == cles.end()) {
            cout << "CLE " + message->getCle() << " not found for " << message->getName() << endl;
            continue;
        }
        Cle *cle = it->second;

        for (auto &flow : message->getFlows()) {
            string dst = flow.getDestination();

            map<string, string>::iterator it = components.find(dst);
            if (it == components.end()) {
                cout << "ERROR: no such destination in components: " + dst;
                continue;
            }
            string remote = it->second;
            boost::to_lower(remote);
            if (gened.find(remote) != gened.end() || !remote.compare(myEnclave)) {
                continue;
            }
            gened.insert(remote);

            Cdf *cdf = cle->find_cdf(myEnclave, remote, true);
            if (cdf == NULL) {
                cout << "XDLINKAGE: Could not find CDF for level/remote: " << myEnclave << "/" << remote << endl;
                continue;
            }

            Cle tmpCle(myEnclave, cdf);
            json clejson;
            to_json(clejson, tmpCle);

            string upper_name = message->getName();
            boost::to_upper(upper_name);

            string clestr = clejson.dump(4);
            findAndReplaceAll(clestr, "\n", " \\\n");
            genfile << "#pragma cle def XDLINKAGE_ECHO_" + upper_name << " " << clestr << endl << endl;
        }
    }
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

    std::vector<string> msgs;
    std::vector<string> locals;
    std::vector<string> destinations;

    string enclave = config.getEnclave();
    for (auto const& x : xdccFlow.getMessages()) {
        if (!is_interested(x.first, enclave))
            continue;

        Message *message = (Message *) x.second;
        string msgName = message->name;

        msgs.push_back(msgName);
        locals.push_back("#define _local_" + msgName + " " + (message->isLocal() ? "1" : "0"));
        destinations.push_back("#define _topic_" + msgName + " " + (message->isTopic() ? "1" : "0"));
    }

    genfile << "/* Messages in system */" << endl;

    genfile << "#define ALL_MSGS_LIST";
    for (std::vector<string>::iterator it = msgs.begin(); it != msgs.end(); ++it) {
        genfile << ", \\\n    " << *it;
    }
    genfile << endl << endl;

    genfile << "/* _local_X is 1 if X is local, else 0 */\n";
    for (std::vector<string>::iterator it = locals.begin(); it != locals.end(); ++it) {
        genfile << *it << endl;
    }
    genfile << endl << endl;

    genfile << "/* _topic_X is 1 if X is a topic, else 0 */\n";
    for (std::vector<string>::iterator it = destinations.begin(); it != destinations.end(); ++it) {
        genfile << *it << endl;
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

   for (auto const& x : shares) {
       genfile << "#pragma cle def begin " + x.second + "_SHAREABLE" << endl;
   }

   genfile << "    int i = 100;" << endl;
   for (auto const& x : shares) {
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
