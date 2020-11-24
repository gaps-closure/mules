#include "Gen.h"
#include "XdccFlow.h"

void make_header_xdcc();
void gen_egress(string topic, string filename);
void gen_xdcc(string topic, string filename);

class GenEgress : public Gen
{
private:
    map<string, string> shares;   // from message names to enclave names
    set<string> remoteEnclaves;

public:
    GenEgress(const string& path, const string& filename, const string& header) : Gen(path, filename, header) {
    }

    int gen(XdccFlow& xdccFlow);
    int open(const XdccFlow &xdccFlow);
    int close();

    set<string> & getRemoteEnclaves() {
        return remoteEnclaves;
    }

protected:
    void annotations(const XdccFlow &xdccFlow);

    void gen_egress(Message *message);
    void gen_egress_array(Message *message, json j, vector<string> path,
                          vector<string> &assignments, vector<string> &in_args, vector<string> &out_args);
    void gen_egress_obj(Message *message, json j, vector<string> path, vector<string> &assignments,
            vector<string> &in_args, vector<string> &out_args);

    void gen_xdcc(Message *message);
    void gen_xdcc_array(Message *message, json j, vector<string> path, vector<string> &assignments,
                        vector<string> &in_args, vector<string> &out_args);
    void gen_xdcc_obj(Message *message, json j, vector<string> path, vector<string> &assignments,
                      vector<string> &in_args, vector<string> &out_args);
};
