#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

class Config {
private:
    string egressDir;
    string ingressDir;
    string echoDir;
    string enclave;
    string xdccFlow;

public:
    Config() {
    }

    Config(const std::string& egressDir_, const std::string& ingressDir_,
           const std::string& echoDir_, const std::string& xdcc_flow_  ) {
        egressDir = egressDir_;
        ingressDir = ingressDir_;
        echoDir = echoDir_;
        xdccFlow = xdcc_flow_;
    }

    void print() const {
        cout << "egressDir: "  << egressDir << endl
             << "IngressDir: " << ingressDir << endl
             << "echoDir: "    << echoDir << endl
             ;
    }

    string& getEchoDir() {
        return echoDir;
    }

    void setEchoDir(string echoDir) {
        this->echoDir = echoDir;
    }

    string& getEgressDir() {
        return egressDir;
    }

    void setEgressDir(string egressDir) {
        this->egressDir = egressDir;
    }

    string& getIngressDir() {
        return ingressDir;
    }

    void setIngressDir(string ingressDir) {
        this->ingressDir = ingressDir;
    }

    string& getXdccFlow() {
        return xdccFlow;
    }

    void setXdccFlow(string xdccFlow) {
        this->xdccFlow = xdccFlow;
    }

    string& getEnclave() {
        return enclave;
    }

    void setEnclave(string enclave) {
        this->enclave = enclave;
    }
};
