#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/any.hpp>

using json = nlohmann::json;
using namespace std;

class Flow {
private:
    int dataId;
    string message;
    string label;
    string fromComponent;
    string toComponent;

public:
    Flow(nlohmann::basic_json<> value) {
        for (auto& el : value.items()) {
            string key = el.key();

            if (!key.compare("dataId")) {
            	dataId = el.value().get<int>();
            }
            else if (!key.compare("message")) {
            	message = el.value().get<string>();
            }
            else if (!key.compare("label")) {
            	label = el.value().get<string>();
            }
            else if (!key.compare("fromComponent")) {
            	fromComponent = el.value().get<string>();
            }
            else if (!key.compare("toComponent")) {
            	toComponent = el.value().get<string>();
            }
            else {
            	cerr << "Flow: unrecognized key: " << key << endl;
            }
        }
    }

	const int getDataId() const {
		return dataId;
	}

	const string& getFromComponent() const {
		return fromComponent;
	}

	const string& getLabel() const {
		return label;
	}

	const string& getMessage() const {
		return message;
	}

	const string& getToComponent() const {
		return toComponent;
	}
};

class Component {
private:
    string component;
    string label;
    vector<int> inMessages;
    vector<int> outMessages;

public:
    Component(nlohmann::basic_json<> value) {
        for (auto& el : value.items()) {
            string key = el.key();

            if (!key.compare("component")) {
            	component = el.value().get<string>();
            }
            else if (!key.compare("label")) {
            	label = el.value().get<string>();
            }
            else if (!key.compare("inMessages")) {
            	for (auto& el2 : el.value().items())
            		inMessages.push_back(el2.value());
            }
            else if (!key.compare("outMessages")) {
            	for (auto& el2 : el.value().items())
            		outMessages.push_back(el2.value());
            }
            else {
            	cerr << "Component: unrecognized key: " << key << endl;
            }
        }
    }

	const string& getComponent() const {
		return component;
	}

	const vector<int>& getInMessages() const {
		return inMessages;
	}

	const string& getLabel() const {
		return label;
	}

	const vector<int>& getOutMessages() const {
		return outMessages;
	}
};

class Message {
private:
    string name;
    string schemaFile;
    string schemaType;
    bool topic;
    bool local = false;

public:
    Message(nlohmann::basic_json<> value) {
        for (auto& el : value.items()) {
            string key = el.key();

            if (!key.compare("name")) {
                name = el.value().get<string>();
            }
            else if (!key.compare("schemaFile")) {
                schemaFile = el.value().get<string>();
            }
            else if (!key.compare("schemaType")) {
            	schemaType = el.value().get<string>();
            }
            else if (!key.compare("topic")) {
                topic = el.value().get<bool>();
            }
            else {
            	cerr << "Message: unrecognized key: " << key << endl;
            }
        }
    }

	const string& getName() const {
		return name;
	}

	const string& getSchemaFile() const {
		return schemaFile;
	}

	const string& getSchemaType() const {
		return schemaType;
	}

	bool isTopic() const {
		return topic;
	}

	bool isLocal() const {
		return local;
	}

	void setLocal(bool local = false) {
		this->local = local;
	}
};

class GuardDirective
{
public:
    string operation;
    vector<int> gapstag;
    bool oneway = false;

    GuardDirective() {
    }

    ~GuardDirective() {};

    GuardDirective(nlohmann::basic_json<> value) {

        for (auto& el : value.items()) {
            string key = el.key();

            if (!key.compare("operation")) {
                operation = el.value().get<string>();
            }
            else if (!key.compare("gapstag")) {
                for (auto& el2 : el.value().items()) {
                    gapstag.push_back(el2.value());
                }
            }
            else if (!key.compare("oneway")) {
                oneway = el.value().get<bool>();
            }
        }
    };

    string getOperation() const {
        return operation;
    }

    vector<int> getGapstag() const {
        return gapstag;
    }

    bool isOneway() const {
        return oneway;
    }
};

class Cdf
{
public:
    string remotelevel;
    string direction;
    GuardDirective guarddirective;

    vector<vector<string>> argtaints;
    vector<string> codtaints;
    vector<string> rettaints;

    Cdf() {
    }

    ~Cdf() {};

    Cdf(nlohmann::basic_json<> value) {
        for (auto& el : value.items()) {
            string key = el.key();

            if (!key.compare("remotelevel")) {
                remotelevel = el.value().get<string>();
            }
            else if (!key.compare("direction")) {
                direction = el.value().get<string>();
            }
            else if (!key.compare("guarddirective")) {
                GuardDirective g(el.value());
                guarddirective = g;
            }
            else if (!key.compare("codtaints")) {
                for (auto& el2 : el.value().items()) {
                    codtaints.push_back(el2.value().get<string>());
                }
            }
            else if (!key.compare("rettaints")) {
                for (auto& el2 : el.value().items()) {
                    rettaints.push_back(el2.value().get<string>());
                }
            }
            else if (!key.compare("argtaints")) {
                for (auto& el2 : el.value().items()) {
                    vector<string> vs;
                    for (auto& el3 : el2.value().items()) {
                        vs.push_back(el3.value().get<string>());
                    }
                    argtaints.push_back(vs);
                }
            }
        }
    };

    string getRemoteLevel() const {
        return remotelevel;
    }

    string getDirection() const {
        return direction;
    }

    GuardDirective getGuardDirective() const {
        return guarddirective;
    }

    vector<vector<string> > getArgtaints() const {
        return argtaints;
    }

    vector<string> getCodtaints() const {
        return codtaints;
    }

    vector<string> getRettaints() const {
        return rettaints;
    }
};

class CleJson
{
public:
    string level;
    vector<Cdf> cdf;

  public:
    CleJson() {
    }

    CleJson(string level, Cdf *cdf) {
        this->level = level;
        this->cdf.push_back(*cdf);
    }

    ~CleJson() {};

    CleJson(nlohmann::basic_json<> value) {
        for (auto& el : value.items()) {
            string key = el.key();

            if (!key.compare("level")) {
                level = el.value().get<string>();
            }
            else if (!key.compare("cdf")) {
                for (auto& el2 : el.value().items()) {
                    Cdf c(el2.value());
                    cdf.push_back(c);
                }
            }
        }
    };

    Cdf* find_cdf(string level, string remote, bool note) {
        string level_l = level;
        boost::to_lower(level_l);

        for (int i = 0; i < cdf.size(); i++) {
            if (!this->level.compare(level_l) &&
                !cdf[i].getRemoteLevel().compare(remote)) {
                return &cdf[i];
            }
        }
        if (note)
            cout << "No CDF for level/remote: " << level << "/" << remote << endl;
        return NULL;
    }

    bool isLocal(string enclave, Flow *flow) {
        for (int i = 0; i < cdf.size(); i++) {
            if (this->level.compare(enclave))  // not flowing from my enclave
            	return false;

            if (cdf[i].getRemoteLevel().compare(enclave)) // flow to a different enclave
                return false;
        }
        return true;
    }

    string getDirection() {
        return level + " -> " + cdf[0].getRemoteLevel();
    }

    string getLevel() {
        return level;
    }

    vector<Cdf>& getCdf() {
        return cdf;
    }

    void setCdf(vector<Cdf> cdf) {
        this->cdf = cdf;
    }

    void setLevel(string level) {
        this->level = level;
    }
};

class Cle
{
public:
    string label;
    CleJson cleJson;

  public:
    Cle(nlohmann::basic_json<> value) {
    	for (auto& el : value.items()) {
    		string key = el.key();

    		if (!key.compare("cle-label")) {
    			label = el.value().get<string>();
    		}
    		else if (!key.compare("cle-json")) {
                CleJson c(el.value());
                cleJson = c;
    		}
    	}
    }
    
    ~Cle() {
	}

	const CleJson& getCleJson() const {
		return cleJson;
	}

	const string& getLabel() const {
		return label;
	}
};

class XdccFlow {
public:

    boost::ptr_map<string, boost::any> topology;
    boost::ptr_map<string, boost::any> messages;
    boost::ptr_map<int, boost::any > flows;
    boost::ptr_map<string, boost::any> cles;

    XdccFlow(const string &filename) {
        std::ifstream jStream(filename);
        json js;
        jStream >> js;

        for (auto& el : js.items()) {
            string key = el.key();
            nlohmann::basic_json<> value = el.value();

            if (!key.compare("topology")) {
                for (auto& el : value.items()) {
                	Component *component = new Component(el.value());
                	topology[component->getComponent()] = component;
                	cout << component->getComponent() << endl;
                }
            }
            else if (!key.compare("messages")) {
                for (auto& el : value.items()) {
                	Message *message = new Message(el.value());
                    messages[message->getName()] = message;
                	cout << message->getName() << endl;
                }
            }
            else if (!key.compare("flows")) {
                for (auto& el2 : value.items()) {
                	Flow *flow = new Flow(el.value());
                	flows[flow->getDataId()] = flow;
                	cout << flow->getDataId() << endl;
                }
            }
            else if (!key.compare("cles")) {
                for (auto& el : value.items()) {
                	Cle *cle = new Cle(el.value());
                	cles[cle->getLabel()] = cle;
                	cout << cle->getLabel() << endl;

                    cles[el.key()] = new Cle(el.value());
                }
            }
        }
    }

    /**
     * Find the Cdf of the remote enclave that is the destination of a message.
     */
    Cdf *find_cle(string level, string remote) const {
        string level_l = level;
        boost::to_lower(level_l);

        Cle *cle = NULL;
        for (auto const& y : cles) {
            Cle *cle = (Cle *) y.second;
            CleJson clejson = cle->getCleJson();
            Cdf *cdf = clejson.find_cdf(level, remote, false);
            if (cdf != NULL)
                return cdf;
        }
        cout << "No CLE for level/remote " << level << "/" << remote << endl;
        return NULL;
    }

    Cle *find_cle(const Flow *flow) const {
        boost::ptr_map<string, boost::any>::const_iterator it = cles.find(flow->getLabel());
        if (it == cles.end()) {
            return NULL;
        }
        return (Cle *)it->second;
    }

    boost::ptr_map<string, boost::any> getCles() const {
        return cles;
    }

    boost::ptr_map<string, boost::any> getMessages() const {
        return messages;
    }

	const boost::ptr_map<string, boost::any>& getTopology() const {
		return topology;
	}

	boost::ptr_map<int, boost::any>& getFlows() {
		return flows;
	}
};
