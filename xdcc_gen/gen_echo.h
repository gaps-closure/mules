#include "gen.h"

void make_header_echo();
void gen_unmarshal(string topic, string filename);
void gen_echo(string topic, string filename);

class GenEcho : public Gen
{
public:
    GenEcho(const string& path, const string& filename, const string& header) : Gen(path, filename, header) {
    }

    int gen(XdccFlow& xdccFlow);
    int open(const XdccFlow &xdccFlow);
    int close();

private:
    void gen_echo(Message *message);
    void gen_echo_obj(Message *message, json j, vector<string> path, vector<string> &assignments,
                        vector<string> &in_args, vector<string> &out_args);
    void gen_echo_array(Message *message, string arrayName, json j, vector<string> path, vector<string> &assignments,
                          vector<string> &in_args, vector<string> &out_args);

    void gen_unmarshal_array(Message *message, string arrayName, json j, vector<string> path,
            vector<string> &assignments, vector<string> &in_args, vector<string> &out_args);
    void gen_unmarshal_obj(Message *message, json j, vector<string> path, vector<string> &assignments,
            vector<string> &in_args, vector<string> &out_args);
    void gen_unmarshal(Message *message);
};
