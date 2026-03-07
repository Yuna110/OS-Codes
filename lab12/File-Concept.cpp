#include <iostream>
#include <fstream>
#include <string>
using namespace std;
 
int main() {
    // Create and write to file
    ofstream outFile("data.txt");
    if (!outFile.is_open()) {
        cerr << "Error: Could not create data.txt" << endl;
        return 1;
    }
    outFile << "Line 1: File Concept" << endl;
    outFile << "Line 2: Access Methods" << endl;
    outFile << "Line 3: Protection" << endl;
    outFile.close();
 
    // Read back from file
    ifstream inFile("data.txt");
    if (!inFile.is_open()) {
        cerr << "Error: Could not open data.txt" << endl;
        return 1;
    }
    string line;
    cout << "File contents:" << endl;
    while (getline(inFile, line)) {
        cout << line << endl;
    }
    inFile.close();
    return 0;
}
// Output:
// File contents:
// Line 1: File Concept
// Line 2: Access Methods
// Line 3: Protection

