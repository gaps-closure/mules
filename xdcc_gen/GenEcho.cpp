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

const int INDENT = 4;

#define TAB_1 string(INDENT, ' ')
#define TAB_2 string(2 * INDENT, ' ')
#define TAB_3 string(3 * INDENT, ' ')

/******************************
 * echo
 */

void GenEcho::traverseArrayEcho(Message *message, string arrayName, json j,
        vector<string> path, string numElements)
{
    string activeCnt = arrayName + "ActiveCnt";
    genVar(activeCnt);
    in_args.push_back("    int " + activeCnt);
    out_args.push_back(activeCnt);

    string maxCnt = arrayName + "MaxCnt";
    genVar(maxCnt);
    in_args.push_back("    int " + maxCnt);
    out_args.push_back(maxCnt);

    string indices = genPath(path);

    stmts.push_back("\n    for (int i = 0; i < " + activeCnt + "; i++) {");
    stmts.push_back("        json ele = js" + indices + "[i];\n");

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
                traverseArrayEcho(message, key, val["items"]["properties"], path, numElements);
            }
            else if (type == "object") {
                traverseObjEcho(message, j["properties"], path);
            }
            else {
                string in_arg;
                if (type == "string") {
                    string maxLength = getField(val, "maxLength", message, path);
                    for (int i = 0; i < stoi(numElements); i++) {
                        string idx = to_string(i);
                        string varidx = var + idx;

                        in_args.push_back("    char *" + varidx);

                        string stmt = "    if (" + activeCnt + " > " + idx + ")";
                        copies.push_back(stmt);

                        stmt = "        js" + indices + "[" + idx + "][\"" + key + "\"] = string(" + varidx + ");";
                        copies.push_back(stmt);
                    }
                }
                else if (type == "integer") {
                    in_args.push_back("    int " + var + "[]");
                    stmts.push_back("        ele[\"" + key + "\"] = " + var + "[i];");
                }
                else if (type == "number") {
                    in_args.push_back("    double " + var + "[]");
                    stmts.push_back("        ele[\"" + key + "\"] = " + var + "[i];");
                }
                else {
                    throw DataException("unsupported type " + type + " for " + genPath(path));
                }
            }
        }
        catch (DataException &e) {
            e.print();
        }
        path.pop_back();
    }
    stmts.push_back("\n        js" + indices + "[i] = ele;");
    stmts.push_back("    }");
}

void GenEcho::traverseObjEcho(Message *message, json j, vector<string> path)
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
                traverseArrayEcho(message, key, val["items"]["properties"], path, numElements);
            }
            else if (type == "object") {
                traverseObjEcho(message, val["properties"], path);
            }
            else {
                string in_arg;
                bool isString = false;
                if (type == "string") {
                    in_arg = "    char *" + var;
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

                string left = isString ? "string(" : "";
                string right = isString ? ")" : "";
                string assign = "    js" + genPath(path) + " = " + left + var + right + ";";
                stmts.push_back(assign);
            }
        }
        catch (DataException &e) {
            e.print();
        }
        path.pop_back();
    }
}

void GenEcho::traverseEcho(Message *message)
{
    json schemaJson;
    beginFunc(message, schemaJson);

    try {
        vector<string> path;

        string type = getField(schemaJson, "type", message, path);
        if (type == "array") {
            string numElements = getField(schemaJson, "numElements", message, path);
            traverseArrayEcho(message, "", schemaJson["items"]["properties"], path, numElements);
        }
        else if (type == "object") {
            traverseObjEcho(message, schemaJson["properties"], path);
        }
        else {
            throw DataException("unsupported type: " + type + " for " + message->getName());
        }
    }
    catch (DataException &e) {
        e.print();
    }
}

void GenEcho::genEcho(Message *message)
{
   string msgName = message->getName();

   try {
       string signature =
               "void echo_" + msgName + "_cpp(\n"
               + "    amqlib_t *__amqlib,\n"
               + "    int __is_topic";

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

       for (std::vector<string>::iterator it = copies.begin(); it != copies.end(); ++it) {
           genfile << *it << endl;
       }

       for (std::vector<string>::iterator it = stmts.begin(); it != stmts.end(); ++it) {
           genfile << *it << endl;
       }

       genfile << endl
               << "    AMQManager *amq = static_cast<AMQManager *>(__amqlib->obj);" << endl
               << "    amq->publish(\"" + msgName + "\", js, __is_topic == 0 ? false : true);" << endl
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
void GenEcho::traverseArrayUnmarshal(Message *message, string arrayName, json j, vector<string> path, string numElements)
{
    string countVar = "*" + arrayName + "ActiveCnt";
    genVar(countVar);
    in_args.push_back("    int " + countVar);

    string countMaxVar = arrayName + "MaxCnt";
    genVar(countMaxVar);
    in_args.push_back("    int " + countMaxVar);

    string indices = genPath(path);

    copies.push_back("    " + countVar + " = js" + indices + ".size();");
//    copies.push_back("    if (" + countVar + " > " + countMaxVar + ")");
//    copies.push_back("        return;");

    stmts.push_back("    for (int i = 0; i < " + countVar +"; i++) {");
    stmts.push_back("        json ele = js" + indices + "[i];");
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
                traverseArrayUnmarshal(message, key, val["items"]["properties"], path, numElements);
            }
            else if (type == "object") {
                traverseObjUnmarshal(message, val["properties"], path);
            }
            else {
                if (type == "string") {
                    string maxLength = getField(val, "maxLength", message, path);
                    for (int i = 0; i < stoi(numElements); i++) {
                        string idx = to_string(i);
                        string varidx = var + idx;

                        in_args.push_back("    char *" + varidx);

                        string stmt = "    if (" + countVar + " > " + to_string(i) + ")";
                        copies.push_back(stmt);

                        stmt = "        strncpy(" + varidx + ", js" + indices + "[" + idx + "][\"" + key + "\"]"
                                + ".get<string>().c_str(), " + maxLength + ");";
                        copies.push_back(stmt);
                    }
                }
                else if (type == "integer") {
                    in_args.push_back("    int " + var + "[]");
                    stmts.push_back("        " + var + "[i] = ele[\"" + key + "\"]" + ".get<int>();");
                }
                else if (type == "number") {
                    in_args.push_back("    double " + var + "[]");
                    stmts.push_back("        " + var + "[i] = ele[\"" + key + "\"]" + ".get<double>();");
                }
                else {
                    throw DataException("unsupported type " + type + " for " + genPath(path));
                }
            }
        }
        catch (DataException &e) {
            e.print();
        }
        path.pop_back();
    }
    stmts.push_back("    }");
}

void GenEcho::traverseObjUnmarshal(Message *message, json j, vector<string> path)
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
                traverseArrayUnmarshal(message, key, val["items"]["properties"], path, numElements);
            }
            else if (type == "object") {
                traverseObjUnmarshal(message, val["properties"], path);
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

void GenEcho::traverseUnmarshal(Message *message)
{
   json schemaJson;
   beginFunc(message, schemaJson);

   try {
       vector<string> path;

       string type = getField(schemaJson, "type", message, path);
       if (type == "array") {
           string numElements = getField(schemaJson, "numElements", message, path);
           traverseArrayUnmarshal(message, "", schemaJson["items"]["properties"], path, numElements);
       }
       else if (type == "object") {
           traverseObjUnmarshal(message, schemaJson["properties"], path);
       }
       else {
           throw DataException("unsupported type: " + type + " for " + message->getName());
       }
   }
   catch (DataException &e) {
       e.print();
   }
}

void GenEcho::genUnmarshal(Message *message)
{
   try {
       vector<string> path;

       string signature;
       signature = "void unmarshal_" + message->getName() + "(\n"
               + "    char *jstr,\n"
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

       for (std::vector<string>::iterator it = copies.begin(); it != copies.end(); ++it) {
           genfile << *it << endl;
       }

       for (std::vector<string>::iterator it = stmts.begin(); it != stmts.end(); ++it) {
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
        traverseEcho((Message *) message);
        genEcho((Message *) message);
        endOfFunc();

        traverseUnmarshal((Message *) message);
        genUnmarshal((Message *) message);
        endOfFunc();
    }
    return 0;
}

void GenEcho::beginFunc(Message *message, json& schemaJson)
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
