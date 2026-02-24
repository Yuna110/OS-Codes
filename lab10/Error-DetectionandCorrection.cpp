#include <iostream>
#include <vector>
using namespace std;

int computeParity(vector<int>& bits) {
    int parity = 0;
    for (int b : bits)
        parity ^= b;
    return parity;
}

bool checkParity(vector<int>& data, int storedParity) {
    return computeParity(data) == storedParity;
}

int main() {
    vector<int> data = {1, 0, 1, 1, 0, 0, 1, 0};

    int parity = computeParity(data);
    cout << "Parity bit: " << parity << endl;

    // Simulate a 1-bit error
    data[3] = 1 - data[3];

    cout << "Error detected: "
         << !checkParity(data, parity) << endl;

    return 0;
}