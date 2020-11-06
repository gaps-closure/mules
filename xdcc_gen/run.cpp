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


#include "xdcc_echo.cpp"


int main(int argc, char **argv)
{
    string line;
    ifstream infile("test.txt");
    if (!infile.is_open()) {
        cout << "Unable to open file: ";
        exit(1);
    }

    while (getline(infile, line)) {
        if (line.rfind("#", 0) == 0) {
            continue;
        }

        stringstream s_stream(line);
        string topic;
        string msg;

        if (s_stream.good()) {
            getline(s_stream, topic, ' ');
        }
        if (s_stream.good()) {
            getline(s_stream, msg, ' ');
        }
//        trim(topic);
//        trim(schema);
        printf("%s\n", topic.c_str());

        if (topic == "updateMissionPlan") {
           unmarshal_updateMissionPlan(msg.c_str());
        }
        else if (topic == "recieveISRMDetections") {
           unmarshal_recieveISRMDetections(msg.c_str());
        }
        else if (topic == "groundMovers") {
            unmarshal_groundMovers(msg.c_str());
        }
        else if (topic == "requestISRMDetections") {
            unmarshal_requestISRMDetections(msg.c_str());
        }
        else if (topic == "pnt") {
           unmarshal_pnt(msg.c_str());
        }
        else if (topic == "component_heartbeats") {
           unmarshal_component_heartbeats(msg.c_str());
        }
        printf("\n");
    }
    infile.close();
}
