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
                string in_arg;
                string out_arg = var;

                if (type == "string") {
                    in_arg = "const char *" + var + "[]";

                    string maxLength = get_field(val, "maxLength", message, path);
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
                    throw DataException("unsupported type " + type + " for " + gen_path(path));
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
                string in_arg;
                string out_arg = var;
                if (type == "string") {
                    in_arg = "const char *" + var;

                    string maxLength = get_field(val, "maxLength", message, path);

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
                    throw DataException("unsupported type " + type + " for " + gen_path(path));
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

void GenEgress::gen_xdcc(Message *message)
{
   string msg_name = message->getName();

   std::ifstream schemaStream(message->getSchemaFile());
   json schemaJson;
   schemaStream >> schemaJson;

   try {
       vector<string> path;
       vector<string> assignments;
       vector<string> in_args;
       vector<string> out_args;

       string type = get_field(schemaJson, "type", message, path);
       if (type == "array") {
           gen_xdcc_array(message, schemaJson["items"]["properties"], path, assignments, in_args, out_args);
       }
       else if (type == "object") {
           gen_xdcc_obj(message, schemaJson["properties"], path, assignments, in_args, out_args);
       }
       else {
           throw DataException("unsupported type: " + type + " for " + message->getName());
       }

       string msg_name_u = msg_name;
       boost::to_upper(msg_name_u);
       genfile << "#pragma cle begin XDLINKAGE_ECHO_" << msg_name_u << endl
               << "int echo_" << msg_name << "(";

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
               << "#pragma cle end XDLINKAGE_ECHO_" << msg_name_u << endl
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
        catch (DataException &e) {
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
        catch (DataException &e) {
            e.print();
        }
        path.pop_back();
    }
}

void GenEgress::gen_egress(Message *message)
{
   string msg_name = message->getName();

   std::ifstream schemaStream(message->getSchemaFile());
   json schemaJson;
   schemaStream >> schemaJson;

   try {
       vector<string> path;
       vector<string> assignments;
       vector<string> in_args;
       vector<string> out_args;

       string type = get_field(schemaJson, "type", message, path);
       if (type == "array") {
           gen_egress_array(message, schemaJson["items"]["properties"], path, assignments, in_args, out_args);
       }
       else if (type == "object") {
           gen_egress_obj(message, schemaJson["properties"], path, assignments, in_args, out_args);
       }
       else {
           throw DataException("Unsupported type: " + type);
       }

       genfile << "int egress_" + msg_name + "(const char *jstr)" << endl
               << "{" << endl
               << "    int fromRemote;" << endl;

       map<string, string>::iterator share_it = shares.find(msg_name);
       if (share_it != shares.end()) {
           genfile << "#pragma cle def begin " + share_it->second + "_SHAREABLE" << endl;
       }
       for (std::vector<string>::iterator it = assignments.begin(); it != assignments.end(); ++it) {
           genfile << *it << endl;
       }
       if (share_it != shares.end()) {
           genfile << "#pragma cle def end " + share_it->second + "_SHAREABLE" << endl;
       }

       genfile << endl
               << "    if (_local_" + msg_name + ")" << endl
               << "        return;" << endl
               << endl;

       genfile << "    unmarshal_" + msg_name + "(" << endl
               << "        jstr," << endl
               << "        &fromRemote";
       for (std::vector<string>::iterator it = in_args.begin(); it != in_args.end(); ++it) {
           genfile << ",\n" << *it;
       }
       genfile << endl
               << "    );" << endl;

       genfile << "    if (fromRemote == 0)" << endl
               << "        echo_" + msg_name + "(" << endl;
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
   catch (DataException &e) {
       e.print();
   }
}

int GenEgress::gen(XdccFlow& xdccFlow)
{
    string enclave = config.getEnclave();
    for (auto const& message : myMessages) {
        gen_xdcc((Message *)message);
        endOfFunc();

        gen_egress((Message *)message);
        endOfFunc();
    }
    return 0;
}

void GenEgress::annotations(const XdccFlow &xdccFlow)
{
    shares.clear();

    string my_enclave = config.getEnclave();

    string my_enclave_u = my_enclave;
    boost::to_upper(my_enclave_u);

    genfile << "#pragma cle def " + my_enclave_u + " {\"level\":\"" + my_enclave + "\"}" << endl << endl;

    // generate shareables
//    map<string, string> components = xdccFlow.getComponents();
//    for (auto const& message : myMessages) {
//        // find remote enclaves the message is to be shared, to prepare to generate the SHAREABLEs
//        for (auto &flow : message->getFlows()) {
//            map<string, string>::iterator it = components.find(flow.getDestination());
//            if (it != components.end() && it->second.compare(my_enclave_u)) {
//                remoteEnclaves.insert(it->second);
//                shares[message->getName()] = it->second;  // e.g. ORANGE or PURPLE
//            }
//        }
//    }

    boost::ptr_map<string, Cle *> cles = xdccFlow.getCles();
    for (auto const& x : remoteEnclaves) {
        Cdf *cdf = xdccFlow.find_cle(x, my_enclave);
        if (cdf == NULL) {
            cout << "SHAREABLE: Could not find CDF for level/remote : " << x << "/" << my_enclave << endl;
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
/*
    // generate xdlinkages
    set<string> gened;
    for (auto const& message : myMessages) {
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
            if (gened.find(remote) != gened.end() || !remote.compare(my_enclave)) {
                continue;
            }
            gened.insert(remote);

            Cdf *cdf = cle->find_cdf(my_enclave, remote, true);
            if (cdf == NULL) {
                cout << "XDLINKAGE: Could not find CDF for level/remote: " << my_enclave << "/" << remote << endl;
                continue;
            }

            Cle tmpCle(my_enclave, cdf);
            json clejson;
            to_json(clejson, tmpCle);

            string upper_name = message->getName();
            boost::to_upper(upper_name);

            string clestr = clejson.dump(4);
            findAndReplaceAll(clestr, "\n", " \\\n");
            genfile << "#pragma cle def XDLINKAGE_ECHO_" + upper_name << " " << clestr << endl << endl;
        }
    }
    */
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
    for (auto const& message : myMessages) {
        string msgName = message->getName();

        genfile << ", \\\n    " << msgName;
    }
    genfile << endl << endl;

    genfile << "/* _local_X is 1 if X is local, else 0 */\n";
//    for (auto const& message : myMessages) {
//        genfile << "#define _local_" + message->name + " " + (message->isLocal() ? "1" : "0") << endl;
//    }
    genfile << endl << endl;

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
