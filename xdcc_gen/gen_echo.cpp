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

using namespace std;
using json = nlohmann::json;

#include "util.h"
#include "gen_echo.h"

/******************************
 * echo
 */
void GenEcho::gen_echo_array(string arrayName, json j, vector<string> path, vector<string> &assignments,
                      vector<string> &in_args, vector<string> &out_args)
{
    string count = "count_" + arrayName;
    in_args.push_back("    int " + count);
    out_args.push_back(count);

    string indices;
    gen_path(path, indices);

    assignments.push_back("\n    for (int i = 0; i < " + count + "; i++) {");
    assignments.push_back("        json ele = js" + indices + "[i];\n");

    int i = 0;
    for (auto& el : j.items()) {
        string key = el.key();
        json val = el.value();

        string var(key);
        int inc = gen_var(var);

        path.push_back(key);

        string stmt = "        ele[\"" + key + "\"] = " + var + "[i];";

        string type = val["type"];
        if (type == "array") {
            gen_echo_array(key, val["items"]["properties"], path, assignments, in_args, out_args);
        }
        else if (type == "object") {
            gen_echo_obj(j["properties"], path, assignments, in_args, out_args);
        }
        else {
            string arg, out_arg = var;
            if (type == "string") {
                arg = "    const char *" + var + "[]";
                out_arg = var + "_";
                stmt = "        ele[\"" + key + "\"] = string(" + var + "[i]);";
            }
            else if (type == "integer") {
                arg = "    int " + var + "[]";

            }
            else if (type == "number") {
                arg = "    double " + var + "[]";
            }
            else {
                cout << "unsupported type: " << type << endl;
                exit(1);
            }
            in_args.push_back(arg);
            out_args.push_back(out_arg);
            assignments.push_back(stmt);
        }
        path.pop_back();
    }
    assignments.push_back("\n        js" + indices + "[i] = ele;");
    assignments.push_back("    }\n");
}

void GenEcho::gen_echo_obj(json j, vector<string> path, vector<string> &assignments,
                    vector<string> &in_args, vector<string> &out_args)
{
    for (auto& el : j.items()) {
        string key = el.key();
        json val = el.value();

        path.push_back(key);

        string var(key);
        gen_var(var);

        string type = val["type"];
        if (type == "array") {
            gen_echo_array(key, val["items"]["properties"], path, assignments, in_args, out_args);
        }
        else if (type == "object") {
            gen_echo_obj(val["properties"], path, assignments, in_args, out_args);
        }
        else {
            string arg;
            string out_arg;
            bool isString = false;
            if (type == "string") {
                arg = "    const char *" + var;
                isString = true;
            }
            else if (type == "integer") {
                arg = "    int " + var;
            }
            else if (type == "number") {
                arg = "    double " + var;
            }
            else {
                cout << "unsupported type: " << type << endl;
                exit(1);
            }
            in_args.push_back(arg);

            gen_leaf(path, var, assignments, isString);
        }
        path.pop_back();
    }
}

void GenEcho::gen_echo(Message *message)
{
   string topic = message->getName();

   std::ifstream schemaStream(message->getSchema());
   json schemaJson;
   schemaStream >> schemaJson;

   vector<string> path;

   string type = schemaJson["type"];
   vector<string> assignments;
   vector<string> in_args;
   vector<string> out_args;

   if (type == "array") {
      gen_echo_array("", schemaJson["items"]["properties"], path, assignments, in_args, out_args);
   }
   else if (type == "object") {
      gen_echo_obj(schemaJson["properties"], path, assignments, in_args, out_args);
   }

   string signature;
   signature = "void echo_" + topic + "_cpp(\n"
             + "    amqlib_t *__amqlib,\n"
             + "    bool __is_topic";

   for (std::vector<string>::iterator it = in_args.begin(); it != in_args.end(); ++it) {
       signature += ",\n" + *it;
   }

   headerfile << signature
              << ");" << endl;

   genfile << signature
           << "\n)" << endl
           << "{" << endl
           << "    json js;" << endl
           << "    js[\"fromRemote\"] = 1;" << endl;

   for (std::vector<string>::iterator it = assignments.begin(); it != assignments.end(); ++it) {
      genfile << *it << endl;
   }

   genfile << endl
           << "    AMQManager *amq = static_cast<AMQManager *>(__amqlib->obj);" << endl
           << "    amq->publish(\"" + topic + "\", js.dump(), __is_topic == 0 ? false : true);" << endl
           << "}" << endl
           << endl;
}

/******************************
 * unmarshaling
 */
void GenEcho::gen_unmarshal_array(string arrayName, json j, vector<string> path,
        vector<string> &assignments, vector<string> &in_args, vector<string> &out_args)
{
    string countVar = "count";
    gen_var(countVar);
    in_args.push_back("    int " + countVar);

    string indices;
    gen_path(path, indices);

    assignments.push_back("\n    for (int i = 0; i < " + countVar +"; i++) {");
    assignments.push_back("        json ele = js" + indices + "[i];");
    int i = 0;
    for (auto& el : j.items()) {
        string key = el.key();
        json val = el.value();

        string var(key);
        gen_var(var);

        //string var = vars[i++];

        path.push_back(key);

        string type = val["type"];
        if (type == "array") {
            gen_unmarshal_array(key, val["items"]["properties"], path, assignments, in_args, out_args);
        }
        else if (type == "object") {
            gen_unmarshal_obj(val["properties"], path, assignments, in_args, out_args);
        }
        else {
            string in_arg;
            string stmt;
            string out_arg;

            if (type == "string") {
                in_arg  = "    char * " + var + "[]";
                stmt    = "        strncpy(" + var + "[i], ele[\"" + key + "\"]" + ".get<string>().c_str(), MAXSTR_SIZE);";
            }
            else if (type == "integer") {
                in_arg  = "    int " + var + "[]";
                stmt = "        " + var + "[i] = ele[\"" + key + "\"]" + ".get<int>();";
            }
            else if (type == "number") {
                in_arg  = "    double " + var + "[]";
                stmt = "        " + var + "[i] = ele[\"" + key + "\"]" + ".get<double>();";
            }
            else {
                cout << "unsupported type: " << type << endl;
                exit(1);
            }
            assignments.push_back(stmt);
            in_args.push_back(in_arg);
        }
        path.pop_back();
    }
    assignments.push_back("    }");
}

void GenEcho::gen_unmarshal_obj(json j, vector<string> path, vector<string> &assignments, vector<string> &in_args, vector<string> &out_args)
{
    for (auto& el : j.items()) {
        string key = el.key();
        json val = el.value();

        path.push_back(key);

        string var(key);
        gen_var(var);

        string type = val["type"];
        if (type == "array") {
            gen_unmarshal_array(key, val["items"]["properties"], path, assignments, in_args, out_args);
        }
        else if (type == "object") {
            gen_unmarshal_obj(val["properties"], path, assignments, in_args, out_args);
        }
        else {
            string in_arg;
            string stmt;
            string out_arg;

            string indices;
            gen_path(path, indices);

            if (type == "string") {
                in_arg  = "    char *" + var;
                stmt    = "    string " + var + "_cpp = js" +  indices + ".get<string>();";
                out_arg = "    strncpy(" + var + ", " + var + "_cpp.c_str(), MAXSTR_SIZE);";
            }
            else if (type == "integer") {
                in_arg  = "    int *" + var;
                stmt    = "    int " + var + "_cpp = js" + indices + ".get<int>();";
                out_arg = "    *" + var + " = " + var + "_cpp;";
            }
            else if (type == "number") {
                in_arg  = "    double *" + var;
                stmt    = "    double " + var + "_cpp = js" + indices + ".get<double>();";
                out_arg = "    *" + var + " = " + var + "_cpp;";
            }
            else {
                cout << "unsupported type: " << type << endl;
                exit(1);
            }
            in_args.push_back(in_arg);
            assignments.push_back(stmt);
            out_args.push_back(out_arg);

        }
        path.pop_back();
    }
}

void GenEcho::gen_unmarshal(Message *message)
{
   std::ifstream schemaStream(message->getSchema());
   json schemaJson;
   schemaStream >> schemaJson;

   vector<string> path;

   string type = schemaJson["type"];
   vector<string> assignments;
   vector<string> in_args;
   vector<string> out_args;

   if (type == "array") {
      gen_unmarshal_array("", schemaJson["items"]["properties"], path, assignments, in_args, out_args);
   }
   else if (type == "object") {
      gen_unmarshal_obj(schemaJson["properties"], path, assignments, in_args, out_args);
   }

   string signature;
   signature = "void unmarshal_" + message->getName() + "(\n"
             + "    const char *jstr,\n"
             + "    int *fromRemote";

   for (std::vector<string>::iterator it = in_args.begin(); it != in_args.end(); ++it) {
       signature += ",\n" + *it;
   }

   headerfile << signature
              << ");" << endl;

   genfile << signature << endl
           << ")" << endl
           << "{" << endl
           << "    json js = json::parse(jstr);" << endl
           << "    int fromRemote_cpp = js.contains(\"fromRemote\") ? js[\"fromRemote\"].get<int>() : 0;" << endl
           << endl;

   for (std::vector<string>::iterator it = assignments.begin(); it != assignments.end(); ++it) {
       genfile << *it << endl;
   }

   genfile << endl
           << "    *fromRemote = fromRemote_cpp;" << endl;
   for (std::vector<string>::iterator it = out_args.begin(); it != out_args.end(); ++it) {
       genfile << *it << endl;
   }

   genfile << "}" << endl
           << endl;
}

int GenEcho::generate(XdccFlow& xdccFlow)
{
    string enclave = config.getEnclave();
    for (auto const& x : xdccFlow.getMessages()) {
        if (!is_interested(x.first, enclave))
            continue;

        Message *message = (Message *)x.second;

        gen_echo(message);
        endOfFunc();

        gen_unmarshal(message);
        endOfFunc();
    }
    return 0;
}

int GenEcho::open(const XdccFlow &xdccFlow)
{
    genfile << "#include <amqm/AMQManager.h>\n"
            << "#include <nlohmann/json.hpp>\n"
            << "#include <string.h>\n"
            << "#include <xdcc_echo.h>\n\n"
            << endl
            << "using namespace std;\n"
            << "using namespace amqm;\n"
            << "using json = nlohmann::json;\n\n";

    genfile << "extern \"C\" {" << endl << endl;

    headerfile << "#ifndef __XDCC_ECHO_H__\n"
               << "#define __XDCC_ECHO_H__\n"
               << endl
               << "#include \"amqlib.h\"\n"
               << endl
               << "#ifdef __cplusplus\n"
               << "extern \"C\" {\n"
               << "#endif /*__cplusplus */\n"
               << endl
               << "#define MAXSTR_SIZE 16 /* XXX: should not be hardcoded, use per-field size from schema */\n"
               << endl;
    
    return 0;
}

int GenEcho::close()
{
    genfile << "\n} // extern \"C\"" << endl << endl;

    headerfile << "#ifdef __cplusplus" << endl
               << "} /* extern \"C\" */" << endl
               << "#endif /* __cplusplus */" << endl
               << endl
               << "#endif /* __XDCC_ECHO_H__ */";
             
    return 0;
}
