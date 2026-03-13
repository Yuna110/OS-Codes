#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <iomanip>
#include <cstring>
#include <cstdint>
#include <ctime>
#include <cstdlib>

using namespace std;

// Permission bitmask
enum Perm : uint8_t {
    P_NONE=0, P_R=1, P_W=2, P_X=4, P_D=8, P_A=16
};

string ps(uint8_t p){
    string s;
    s+=(p&P_R)?"r":"-";
    s+=(p&P_W)?"w":"-";
    s+=(p&P_X)?"x":"-";
    s+=(p&P_D)?"d":"-";
    s+=(p&P_A)?"a":"-";
    return s;
}

// Security label ranking
int rankLabel(const string& l){
    if(l=="top_secret") return 3;
    if(l=="secret") return 2;
    if(l=="confidential") return 1;
    return 0;
}

// Resource and Subject
struct Resource{
    string owner;
    string label;
    map<string,uint8_t> acl;
};

struct Subject{
    set<string> roles;
    string clearance;
    bool disabled;
};

// Access Control Engine
class ACE{

    map<string,Subject> S;
    map<string,Resource> R;

    set<string> expand(const set<string>& base){

        map<string,vector<string> > H;
        H["admin"]={"manager","analyst","user"};
        H["manager"]={"analyst","user"};
        H["analyst"]={"user"};
        H["user"]={};

        set<string> out=base;

        for(auto r:base){
            if(H.count(r)){
                for(auto x:H[r])
                    out.insert(x);
            }
        }

        return out;
    }

public:

    void addSubject(string n, vector<string> roles, string clr){

        Subject s;
        s.clearance=clr;
        s.disabled=false;

        for(auto r:roles)
            s.roles.insert(r);

        S[n]=s;
    }

    void addResource(string n,string owner,string label,map<string,uint8_t> acl){

        Resource r;
        r.owner=owner;
        r.label=label;
        r.acl=acl;

        R[n]=r;
    }

    bool check(string subj,string res,uint8_t req){

        cout<<"[CHECK] "<<subj<<" ["<<ps(req)<<"] "<<res<<" -> ";

        if(!S.count(subj)||!R.count(res)){
            cout<<"DENY(not found)\n";
            return false;
        }

        Subject &s=S[subj];
        Resource &r=R[res];

        if(s.disabled){
            cout<<"DENY(disabled)\n";
            return false;
        }

        if((req&P_R)&&rankLabel(s.clearance)<rankLabel(r.label)){
            cout<<"DENY(no-read-up)\n";
            return false;
        }

        if((req&P_W)&&rankLabel(s.clearance)>rankLabel(r.label)){
            cout<<"DENY(no-write-down)\n";
            return false;
        }

        uint8_t eff=P_NONE;

        if(r.owner==subj)
            eff|=(P_R|P_W|P_X|P_D);

        for(auto role:expand(s.roles))
            if(r.acl.count(role))
                eff|=r.acl[role];

        if((eff&req)==req){
            cout<<"ALLOW\n";
            return true;
        }

        cout<<"DENY\n";
        return false;
    }
};

// Stack Canary demo
class CanaryGuard{

    static const uint64_t CANARY=0xDEADBEEFCAFEBABEULL;

public:

    void safeProcess(const char* input){

        volatile uint64_t guard1=CANARY;

        char buf[32];

        volatile uint64_t guard2=CANARY;

        size_t len=strlen(input);

        if(len>=sizeof(buf)){
            cout<<"[CANARY] input too long\n";
            return;
        }

        memcpy(buf,input,len);
        buf[len]=0;

        if(guard1!=CANARY||guard2!=CANARY){
            cout<<"STACK CORRUPTION DETECTED\n";
            abort();
        }

        cout<<"Safe: "<<buf<<"\n";
    }
};

// Audit log
class AuditLog{

    vector<string> log;

public:

    void write(string entry){

        time_t t=time(NULL);

        char buf[32];
        strftime(buf,32,"%Y-%m-%d %H:%M:%S",localtime(&t));

        log.push_back(string(buf)+" | "+entry);
    }

    void dump(){

        cout<<"\nAUDIT LOG\n";

        for(auto e:log)
            cout<<e<<"\n";
    }
};

int main(){

    cout<<"Security Demo\n\n";

    ACE ace;

    ace.addSubject("root",{"admin"},"top_secret");
    ace.addSubject("alice",{"analyst"},"secret");
    ace.addSubject("bob",{"user"},"unclassified");

    ace.addResource("/etc/shadow","root","secret",{{"admin",P_R|P_W}});
    ace.addResource("/var/log/auth","root","confidential",{{"analyst",P_R}});
    ace.addResource("/home/bob/notes","bob","unclassified",{{"user",P_R|P_W}});

    ace.check("root","/etc/shadow",P_R|P_W);
    ace.check("alice","/etc/shadow",P_R);
    ace.check("alice","/var/log/auth",P_R);
    ace.check("bob","/var/log/auth",P_R);
    ace.check("bob","/home/bob/notes",P_R|P_W);

    cout<<"\nStack Canary\n";

    CanaryGuard g;
    g.safeProcess("HelloWorld");

    AuditLog log;
    log.write("root ALLOW /etc/shadow");
    log.write("alice DENY /etc/shadow");

    log.dump();

    return 0;
}