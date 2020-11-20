#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include <unordered_map>
#include <unistd.h>
#include <dirent.h>
#include <getopt.h>
#include <iomanip>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

#include "util.h"
#include "GenEcho.h"
#include "GenEgress.h"
#include "Config.h"
#include "XdccFlow.h"

Config config(".", ".", ".", "xdcc_flow.json");
int verbose = 0;
int debug = 0;

void from_json(const json& j, Config& config)
{
    j.at("egressDir").get_to(config.getEgressDir());
    j.at("ingressDir").get_to(config.getIngressDir());
    j.at("echoDir").get_to(config.getEchoDir());
    j.at("enclave").get_to(config.getEnclave());
    j.at("xdccFlow").get_to(config.getXdccFlow());
}

static void print_usage(char *cmd)
{
    cout << cmd << endl
         << "  -e \t egress output directory" << endl
         << "  -i \t ingress output directory" << endl
         << "  -k \t echo output directory" << endl
         << "  -f \t input JSON file" << endl
         << "  -c \t configuration file" << endl
         << "  -d \t" << endl
         << "  -v \t" << endl
         << "  -h \t print this message and exit" << endl
         ;
    exit(0);
}

static int parse_cmdline(int argc, char *argv[])
{
    int c;

    struct option long_options[] = {
        {"egress",   required_argument, 0, 'e'},
        {"ingress",  required_argument, 0, 'i'},
        {"echo",     required_argument, 0, 'k'},
        {"config",   required_argument, 0, 'c'},
        {"verbose",  no_argument,       0, 'v'},
        {"debug",    no_argument,       0, 'd'},
        {"help",     no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    vector<char *> dirs;
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "hdve:i:k:f:c:", long_options, &option_index)) != -1) {
       switch (c) {
          case 'e':
             config.setEgressDir(optarg);
             break;
          case 'i':
             config.setIngressDir(optarg);
             break;
          case 'k':
             config.setEchoDir(optarg);
             break;
          case 'c': {
              std::ifstream jStream(optarg);
              json js;
              jStream >> js;

              config = js.get<Config>();
          }
             break;
          case 'v':
             verbose = 1;
             break;
          case 'd':
             debug = 1;
             break;
          case 'h':
             print_usage(argv[0]);
             break;
          default:
             print_usage(argv[0]);
             break;
       }
    }
    
    return optind;
}

int main(int argc, char **argv)
{
    int index = parse_cmdline(argc, argv);

    XdccFlow xdccFlow(config.getXdccFlow());

    GenEgress genEgress(config.getEgressDir(), "egress_xdcc.c", "");
    genEgress.generate(xdccFlow);

    GenEcho genEcho(config.getEchoDir(), "xdcc_echo.cpp", "xdcc_echo.h");
    genEcho.generate(xdccFlow);

    GenEgress genIngress(config.getIngressDir(), "ingress_xdcc.c", "");
    set<string> remote_enclaves = genEgress.getRemoteEnclaves();
    for (auto e : remote_enclaves) { // TODO: this will only work if there is just one remote enclave
        string enclave = e;
        boost::to_lower(enclave);

        config.setEnclave(enclave);
        genIngress.generate(xdccFlow);
    }
}
