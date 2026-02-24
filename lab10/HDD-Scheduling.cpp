#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

int sstf(int head, vector<int> requests) {
    int totalMovement = 0;

    while (!requests.empty()) {
        int closest = 0;

        for (int i = 1; i < requests.size(); i++) {
            if (abs(requests[i] - head) < abs(requests[closest] - head))
                closest = i;
        }

        totalMovement += abs(requests[closest] - head);
        head = requests[closest];
        requests.erase(requests.begin() + closest);
    }

    return totalMovement;
}

int main() {
    vector<int> req = {98, 183, 37, 122, 14, 124, 65, 67};

    cout << "SSTF Total Head Movement: "
         << sstf(53, req) << endl;

    return 0;
}