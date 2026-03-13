#include <iostream>
#include <string>
#include <map>
#include <random>
#include <sstream>
#include <iomanip>
#include <functional>
#include <ctime>

using namespace std;

// ── Salted password hasher ─────────────────────────────
class ShadowHash {
public:

    static string salt(int len = 8) {

        static const string C =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

        random_device rd;
        mt19937 g(rd());
        uniform_int_distribution<> d(0, C.size() - 1);

        string s;
        for (int i = 0; i < len; i++)
            s += C[d(g)];

        return s;
    }

    // renamed to avoid conflict with std::hash
    static string makeHash(const string& pw, const string& s) {

        size_t h = std::hash<string>{}(s + pw + s);

        ostringstream o;
        o << "$6$" << s << "$" << hex << h;

        return o.str();
    }

    static bool verify(const string& pw, const string& stored) {

        size_t p1 = stored.find('$', 3);

        string s = stored.substr(3, p1 - 3);

        return makeHash(pw, s) == stored;
    }
};

// ── TOTP ───────────────────────────────────────────────
class TOTP {

    string secret_;
    static const int STEP = 30;

public:
    explicit TOTP(const string& s) : secret_(s) {}

    int token() const {

        long long step = (long long)time(nullptr) / STEP;

        size_t h = std::hash<string>{}(secret_) ^ (size_t)step;

        return (int)(h % 1000000);
    }

    bool valid(int t) const { return t == token(); }
};

// ── User store ─────────────────────────────────────────
struct Account {

    string name, pwHash, totpSecret;

    int fails = 0;

    bool locked = false;

    time_t lockTs = 0;

    bool mfa = false;
};

// ── Authentication system ─────────────────────────────
class AuthSystem {

    map<string, Account> db_;

    const int MAX_FAIL = 3;
    const int LOCK_SECS = 30;

    bool unlockIfExpired(Account& a) {

        if (!a.locked) return false;

        if (time(nullptr) - a.lockTs >= LOCK_SECS) {

            a.locked = false;
            a.fails = 0;

            cout << " [UNLOCK] Account unlocked\n";
        }

        return a.locked;
    }

    void fail(Account& a) {

        if (++a.fails >= MAX_FAIL) {

            a.locked = true;
            a.lockTs = time(nullptr);

            cout << " [LOCKOUT] Account locked\n";
        }
        else {

            cout << " [FAIL] " << a.fails << "/" << MAX_FAIL << "\n";
        }
    }

public:

    void addUser(const string& name, const string& pw, bool mfa=false) {

        Account a;

        a.name = name;

        string s = ShadowHash::salt();

        a.pwHash = ShadowHash::makeHash(pw, s);

        a.mfa = mfa;

        a.totpSecret = mfa ? "TOTP_" + name : "";

        db_[name] = a;

        cout << "[ADDUSER] " << name << "\n";
    }

    bool login(const string& name, const string& pw, int otp=-1) {

        cout << "\n[LOGIN] " << name << "\n";

        auto it = db_.find(name);

        if (it == db_.end()) {

            cout << " [DENY] Unknown user\n";
            return false;
        }

        Account& a = it->second;

        if (unlockIfExpired(a)) {

            cout << " [DENY] Account locked\n";
            return false;
        }

        if (!ShadowHash::verify(pw, a.pwHash)) {

            fail(a);

            cout << " [DENY] Bad password\n";

            return false;
        }

        if (a.mfa) {

            TOTP totp(a.totpSecret);

            if (!totp.valid(otp)) {

                fail(a);

                cout << " [DENY] Bad OTP\n";

                return false;
            }

            cout << " [MFA OK]\n";
        }

        a.fails = 0;

        cout << " [GRANT] Access granted\n";

        return true;
    }
};

int main() {

    cout << "=== PAM-style Authentication System ===\n\n";

    AuthSystem auth;

    auth.addUser("alice", "Pa$$w0rd!", false);

    auth.addUser("bob", "Linux2024", true);

    cout << "\n--- Scenario 1 ---";
    auth.login("alice", "Pa$$w0rd!");

    cout << "\n--- Scenario 2 ---";
    auth.login("alice", "wrong1");
    auth.login("alice", "wrong2");
    auth.login("alice", "wrong3");

    cout << "\n--- Scenario 3 ---";

    TOTP totp("TOTP_bob");
    int t = totp.token();

    cout << " OTP: " << setw(6) << setfill('0') << t << "\n";

    auth.login("bob", "Linux2024", t);

    return 0;
}