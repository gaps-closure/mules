#ifndef _INPUT_H
#define _INPUT_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

class MessageDef {
public:

    string message;
    string schema;
    bool local;
    bool topic;

    MessageDef() {
    };

    bool isLocal() const {
        return local;
    }

    string getMessage() const {
        return message;
    }

    string getSchema() const {
        return schema;
    }

    bool isTopic() const {
        return topic;
    }
};

#endif
