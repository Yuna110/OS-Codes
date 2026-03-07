#include <iostream>
#include <fstream>
using namespace std;
 
int main() {
    // Sequential Write
    ofstream out("records.bin", ios::binary);
    int values[] = {100, 200, 300, 400, 500};
    for (int i = 0; i < 5; i++) {
        out.write(reinterpret_cast<char*>(&values[i]), sizeof(int));
    }
    out.close();
 
    // Sequential Read
    ifstream in("records.bin", ios::binary);
    int val;
    int pos = 0;
    cout << "Sequential Read:" << endl;
    while (in.read(reinterpret_cast<char*>(&val), sizeof(int))) {
        cout << "Record[" << pos++ << "] = " << val << endl;
    }
    in.close();
    return 0;
}
// Output:
// Sequential Read:
// Record[0] = 100
// Record[1] = 200
// Record[2] = 300
// Record[3] = 400
// Record[4] = 500

