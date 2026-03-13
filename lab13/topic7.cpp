#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>

using namespace std;

// Packet representation
struct Packet {
    string srcIP, dstIP;
    int srcPort, dstPort;
    string proto;      
    string direction;  
    string payload;
    bool isSyn=false;
};

// Connection tracking
struct ConnKey {
    string srcIP; int srcPort;
    string dstIP; int dstPort;
    string proto;

    bool operator<(const ConnKey& o) const {
        ostringstream a,b;
        a<<srcIP<<srcPort<<dstIP<<dstPort<<proto;
        b<<o.srcIP<<o.srcPort<<o.dstIP<<o.dstPort<<o.proto;
        return a.str()<b.str();
    }
};

enum ConnState {SYN_SENT,ESTABLISHED,CLOSED};

map<ConnKey,ConnState> conntrack;


// Firewall rule
struct Rule{
    string chain;
    string srcNet;
    string dstNet;
    int dstPort;
    string proto;
    string target;
    string comment;
    bool stateful;
};


// Firewall Engine
class Firewall{

vector<Rule> rules_;
int accepted_=0;
int dropped_=0;

bool matchNet(const string& ip,const string& net){
    if(net=="ANY") return true;

    string prefix=net.substr(0,net.find('/'));
    return ip.find(prefix.substr(0,prefix.rfind('.')))==0;
}

bool isEstablished(const Packet& p){

    ConnKey k{p.srcIP,p.srcPort,p.dstIP,p.dstPort,p.proto};
    ConnKey r{p.dstIP,p.dstPort,p.srcIP,p.srcPort,p.proto};

    return (conntrack.count(k)&&conntrack[k]==ESTABLISHED) ||
           (conntrack.count(r)&&conntrack[r]==ESTABLISHED);
}

void trackConn(const Packet& p){

    ConnKey k{p.srcIP,p.srcPort,p.dstIP,p.dstPort,p.proto};

    if(p.isSyn)
        conntrack[k]=SYN_SENT;

    else if(conntrack.count(k)&&conntrack[k]==SYN_SENT)
        conntrack[k]=ESTABLISHED;
}

public:

void addRule(const Rule& r){
    rules_.push_back(r);
}

string process(const Packet& p){

    if(isEstablished(p)){
        trackConn(p);
        accepted_++;
        return "ACCEPT (ESTABLISHED)";
    }

    for(auto &r:rules_){

        bool matchProto=(r.proto=="ANY"||r.proto==p.proto);
        bool matchSrc=matchNet(p.srcIP,r.srcNet);
        bool matchDst=(r.dstPort==-1||r.dstPort==p.dstPort);
        bool matchDir=(r.chain==p.direction||r.chain=="BOTH");

        if(matchProto && matchSrc && matchDst && matchDir){

            if(!p.payload.empty() &&
               p.payload.find("DROP_KEYWORD")!=string::npos){

                dropped_++;
                return "DROP (DPI blocked)";
            }

            trackConn(p);

            if(r.target=="ACCEPT"||r.target=="LOG"){
                accepted_++;
                return r.target+" ["+r.comment+"]";
            }

            dropped_++;
            return r.target+" ["+r.comment+"]";
        }
    }

    dropped_++;
    return "DROP (default)";
}

void stats(){
    cout<<"\nFirewall Stats\n";
    cout<<"Accepted: "<<accepted_<<endl;
    cout<<"Dropped : "<<dropped_<<endl;
}

};


// MAIN PROGRAM
int main(){

    Firewall fw;

    // Add rules
    fw.addRule({"INBOUND","192.168.1.0/24","ANY",80,"TCP","ACCEPT","Allow HTTP",true});
    fw.addRule({"INBOUND","ANY","ANY",22,"TCP","DROP","Block SSH",false});
    fw.addRule({"BOTH","ANY","ANY",-1,"ANY","ACCEPT","Allow others",false});


    // Test packets
    Packet p1={"192.168.1.10","10.0.0.1",1234,80,"TCP","INBOUND","Hello",true};
    Packet p2={"5.5.5.5","10.0.0.1",3456,22,"TCP","INBOUND","SSH login",true};
    Packet p3={"192.168.1.20","10.0.0.1",5000,443,"TCP","INBOUND","DROP_KEYWORD",false};

    cout<<"Packet1: "<<fw.process(p1)<<endl;
    cout<<"Packet2: "<<fw.process(p2)<<endl;
    cout<<"Packet3: "<<fw.process(p3)<<endl;

    fw.stats();

    return 0;
}