#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
using namespace std;
 
string permString(mode_t mode) {
    string perm = "---------";
    // Owner
    if (mode & S_IRUSR) perm[0] = 'r';
    if (mode & S_IWUSR) perm[1] = 'w';
    if (mode & S_IXUSR) perm[2] = 'x';
    // Group
    if (mode & S_IRGRP) perm[3] = 'r';
    if (mode & S_IWGRP) perm[4] = 'w';
    if (mode & S_IXGRP) perm[5] = 'x';
    // Others
    if (mode & S_IROTH) perm[6] = 'r';
    if (mode & S_IWOTH) perm[7] = 'w';
    if (mode & S_IXOTH) perm[8] = 'x';
    return perm;
}
 
int main() {
    const char* fname = "protected.txt";
 
    // (a) Create file
    ofstream f(fname);
    f << "Sensitive data";
    f.close();
 
    // (b) Set permissions: 0600 = rw------- (owner read+write only)
    if (chmod(fname, S_IRUSR | S_IWUSR) == -1) {
        perror("chmod"); return 1;
    }
    // (c) Read and display permissions
    struct stat info;
    stat(fname, &info);
    cout << "Permissions for '" << fname << "': "
         << permString(info.st_mode & 0777) << endl;
    cout << "Octal: " << oct << (info.st_mode & 0777) << dec << endl;
    // Output: Permissions: rw-------   Octal: 600
    return 0;
}

