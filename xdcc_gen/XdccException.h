#pragma once

#include <iostream>
#include <string>

using namespace std;

class DataException : public std::logic_error {

public:
    DataException(const string& msg) : logic_error(msg) {
    }

    ~DataException() {
    }

    void print() {
        cerr << "DataException: " << what() << endl;
    }
};
