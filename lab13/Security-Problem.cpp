#include <iostream> 
#include <fstream> 
#include <sstream> 
#include <string> 
#include <map> 
#include <vector> 
#include <functional> 
class IntegrityMonitor { 
private: 
 std::map<std::string, size_t> baseline; 
 size_t computeHash(const std::string& filename) { 
 std::ifstream file(filename); 
 if (!file.is_open()) return 0; 
 std::stringstream buf; 
 buf << file.rdbuf(); 
 return std::hash<std::string>{}(buf.str()); 
 } 
public: 
 void createBaseline(const std::vector<std::string>& files) {  std::cout << "[BASELINE] Creating trusted baseline...\n";
 for (const auto& f : files) { 
 size_t h = computeHash(f); 
 if (h != 0) { 
 baseline[f] = h; 
 std::cout << " [OK] " << f << " | hash=" << h << "\n";  } else { 
 std::cout << " [WARN] Cannot open: " << f << "\n";  } 
 } 
 } 
 void checkIntegrity() { 
 std::cout << "\n[CHECK] Running integrity verification...\n";  bool clean = true; 
 for (const auto& [file, base] : baseline) { 
 size_t curr = computeHash(file); 
 if (curr == 0) { 
 std::cout << " [AVAILABILITY] MISSING: " << file << "\n";  clean = false; 
 } else if (curr != base) { 
 std::cout << " [INTEGRITY] MODIFIED: " << file << "\n"  << " Expected: " << base << "\n" 
 << " Found: " << curr << "\n"; 
 clean = false; 
 } else { 
 std::cout << " [OK] Unchanged: " << file << "\n";  } 
 } 
 if (clean) std::cout << "[SECURE] All files intact.\n";  } 
}; 
void simulateAttack(const std::string& file) { 
 // Attacker injects a backdoor root account 
 std::ofstream f(file, std::ios::app); 
 f << "\nhacker:x:0:0:root:/root:/bin/bash\n";
 std::cout << "[ATTACKER] Injected rogue entry into " << file << "\n"; } 
int main() { 
 std::cout << "=== CIA Triad Monitor (Tripwire/AIDE Simulation) ===\n\n"; 
 // Create simulated system files 
 { std::ofstream f("sim_passwd.txt"); 
 f << "root:x:0:0:root:/root:/bin/bash\n" 
 << "alice:x:1000:1000::/home/alice:/bin/bash\n"; } 
 { std::ofstream f("sim_sudoers.txt"); 
 f << "root ALL=(ALL:ALL) ALL\nalice ALL=(ALL) NOPASSWD: ALL\n"; } 
 IntegrityMonitor monitor; 
 monitor.createBaseline({"sim_passwd.txt", "sim_sudoers.txt"});  simulateAttack("sim_passwd.txt"); 
 monitor.checkIntegrity(); 
 return 0; 
} 
