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
#include "GenEcho.h"
#include "XdccException.h"

/******************************
 * echo
 */
void GenEcho::genEchoArray(Message *message, string arrayName, json j, vector<string> path,
        vector<string> &assignments, vector<string> &in_args, vector<string> &out_args)
{
    string count = "count_" + arrayName;
    in_args.push_back("    int " + count);
    out_args.push_back(count);

    string indices = genPath(path);

    assignments.push_back("\n    for (int i = 0; i < " + count + "; i++) {");
    assignments.push_back("        json ele = js" + indices + "[i];\n");

    int i = 0;
    for (auto& el : j.items()) {
        string key = el.key();
        json val = el.value();

        string var(key);
        int inc = genVar(var);

        path.push_back(key);

        try {
            string stmt = "        ele[\"" + key + "\"] = " + var + "[i];";

            string type = getField(val, "type", message, path);
            if (type == "array") {
                genEchoArray(message, key, val["items"]["properties"], path, assignments, in_args, out_args);
            }
            else if (type == "object") {
                genEchoObj(message, j["properties"], path, assignments, in_args, out_args);
            }
            else {
                string in_arg;
                string out_arg = var;
                if (type == "string") {
                    in_arg = "    const char *" + var + "[]";
                    out_arg = var + "_";
                    stmt = "        ele[\"" + key + "\"] = string(" + var + "[i]);";
                }
                else if (type == "integer") {
                    in_arg = "    int " + var + "[]";
                }
                else if (type == "number") {
                    in_arg = "    double " + var + "[]";
                }
                else {
                    throw DataException("unsupported type " + type + " for " + genPath(path));
                }
                in_args.push_back(in_arg);
                out_args.push_back(out_arg);
                assignments.push_back(stmt);
            }
        }
        catch (DataException &e) {
            e.print();
        }
        path.pop_back();
    }
    assignments.push_back("\n        js" + indices + "[i] = ele;");
    assignments.push_back("    }\n");
}

void GenEcho::genEchoObj(Message *message, json j, vector<string> path, vector<string> &assignments,
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
                genEchoArray(message, key, val["items"]["properties"], path, assignments, in_args, out_args);
            }
            else if (type == "object") {
                genEchoObj(message, val["properties"], path, assignments, in_args, out_args);
            }
            else {
                string in_arg;
                bool isString = false;
                if (type == "string") {
                    in_arg = "    const char *" + var;
                    isString = true;
                }
                else if (type == "integer") {
                    in_arg = "    int " + var;
                }
                else if (type == "number") {
                    in_arg = "    double " + var;
                }
                else {
                    throw DataException("unsupported type " + type + " for " + genPath(path));
                }
                in_args.push_back(in_arg);

                genLeaf(path, var, assignments, isString);
            }
        }
        catch (DataException &e) {
            e.print();
        }
        path.pop_back();
    }
}

void GenEcho::genEcho(Message *message)
{
   string topic = message->getName();

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
           genEchoArray(message, "", schemaJson["items"]["properties"], path, assignments, in_args, out_args);
       }
       else if (type == "object") {
           genEchoObj(message, schemaJson["properties"], path, assignments, in_args, out_args);
       }
       else {
           throw DataException("unsupported type: " + type + " for " + message->getName());
       }

       string signature;
       signature = "void echo_" + topic + "_cpp(\n"
               + "    amqlib_t *__amqlib,\n"
               + "    bool __is_topic";

       for (std::vector<string>::iterator it = in_args.begin(); it != in_args.end(); ++it) {
           signature += ",\n" + *it;
       }

       headerfile << signature
               << ");" << endl
               << endl;

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
               << "    amq->publish(\"" + topic + "\", js, __is_topic == 0 ? false : true);" << endl
               << "}" << endl
               << endl;
   }
   catch (DataException &e) {
       e.print();
   }
}

/******************************
 * unmarshaling
 */
void GenEcho::genUnmarshalArray(Message *message, string arrayName, json j, vector<string> path,
        vector<string> &assignments, vector<string> &in_args, vector<string> &out_args)
{
    string countVar = "count";
    genVar(countVar);
    in_args.push_back("    int " + countVar);

    string indices = genPath(path);

    assignments.push_back("\n    for (int i = 0; i < " + countVar +"; i++) {");
    assignments.push_back("        json ele = js" + indices + "[i];");
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
                genUnmarshalArray(message, key, val["items"]["properties"], path, assignments, in_args, out_args);
            }
            else if (type == "object") {
                genUnmarshalObj(message, val["properties"], path, assignments, in_args, out_args);
            }
            else {
                string in_arg;
                string stmt;
                string out_arg;

                if (type == "string") {
                    string maxLength = to_string(val["maxLength"]);
                    in_arg  = "    char * " + var + "[]";
                    stmt    = "        strncpy(" + var + "[i], ele[\"" + key + "\"]"
                            + ".get<string>().c_str(), " + maxLength + ");";
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
                    throw DataException("unsupported type " + type + " for " + genPath(path));
                }
                assignments.push_back(stmt);
                in_args.push_back(in_arg);
            }
        }
        catch (DataException &e) {
            e.print();
        }
        path.pop_back();
    }
    assignments.push_back("    }");
}

void GenEcho::genUnmarshalObj(Message *message, json j, vector<string> path, vector<string> &assignments,
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
                genUnmarshalArray(message, key, val["items"]["properties"], path, assignments, in_args, out_args);
            }
            else if (type == "object") {
                genUnmarshalObj(message, val["properties"], path, assignments, in_args, out_args);
            }
            else {
                string in_arg;
                string stmt;
                string out_arg;

                string indices = genPath(path);

                if (type == "string") {
                    in_arg  = "    char *" + var;
                    stmt    = "    string " + var + "_cpp = js" +  indices + ".get<string>();";
                    string maxLength = to_string(val["maxLength"]);
                    out_arg = "    strncpy(" + var + ", " + var + "_cpp.c_str(), " + maxLength + ");";
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
                    throw DataException("unsupported type " + type + " for " + genPath(path));
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

void GenEcho::genUnmarshal(Message *message)
{
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
           genUnmarshalArray(message, "", schemaJson["items"]["properties"], path, assignments, in_args, out_args);
       }
       else if (type == "object") {
           genUnmarshalObj(message, schemaJson["properties"], path, assignments, in_args, out_args);
       }
       else {
           throw DataException("unsupported type: " + type + " for " + message->getName());
       }

       string signature;
       signature = "void unmarshal_" + message->getName() + "(\n"
               + "    const char *jstr,\n"
               + "    int *fromRemote";

       for (std::vector<string>::iterator it = in_args.begin(); it != in_args.end(); ++it) {
           signature += ",\n" + *it;
       }

       headerfile << signature
               << ");" << endl
               << endl;

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
   catch (DataException &e) {
       e.print();
   }
}

int GenEcho::gen(XdccFlow& xdccFlow)
{
    for (auto const& message : myMessages) {
        genEcho((Message *) message);
        endOfFunc();

        genUnmarshal((Message *) message);
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
