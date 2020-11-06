#include "gen.h"

void make_header_echo();
void gen_unmarshal(string topic, string filename);
void gen_echo(string topic, string filename);

class GenEcho : public Gen
{
public:
    GenEcho(const string& path, const string& filename, const string& header) : Gen(path, filename, header) {
    }

    int generate(XdccFlow& xdccFlow);
    int open(const XdccFlow &xdccFlow);
    int close();

private:
    void gen_echo(Message *message);
    void gen_echo_obj(json j, vector<string> path, vector<string> &assignments,
                        vector<string> &in_args, vector<string> &out_args);
    void gen_echo_array(string arrayName, json j, vector<string> path, vector<string> &assignments,
                          vector<string> &in_args, vector<string> &out_args);

    void gen_unmarshal_array(string arrayName, json j, vector<string> path,
            vector<string> &assignments, vector<string> &in_args, vector<string> &out_args);
    void gen_unmarshal_obj(json j, vector<string> path, vector<string> &assignments,
            vector<string> &in_args, vector<string> &out_args);
    void gen_unmarshal(Message *message);
};
