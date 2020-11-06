#include "gen.h"
#include "XdccFlow.h"

void make_header_xdcc();
void gen_egress(string topic, string filename);
void gen_xdcc(string topic, string filename);

class GenEgress : public Gen
{
private:
    map<string, string> shares;

public:
    GenEgress(const string& path, const string& filename, const string& header) : Gen(path, filename, header) {
    }

    int generate(XdccFlow& xdccFlow);
    int open(const XdccFlow &xdccFlow);
    int close();

protected:
    void annotations(const XdccFlow &xdccFlow);

    void gen_egress(Message *def);
    void gen_egress_array(string arrayName, json j, vector<string> path,
                          vector<string> &assignments, vector<string> &in_args, vector<string> &out_args);
    void gen_egress_obj(json j, vector<string> path, vector<string> &assignments,
            vector<string> &in_args, vector<string> &out_args);

    void gen_xdcc(Message *def);
    void gen_xdcc_array(string arrayName, json j, vector<string> path, vector<string> &assignments,
                        vector<string> &in_args, vector<string> &out_args);
    void gen_xdcc_obj(json j, vector<string> path, vector<string> &assignments,
                      vector<string> &in_args, vector<string> &out_args);
};
