#include <iostream>
#include <fstream>
#include <sys/file.h>   // flock()
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
using namespace std;
 
// (a) WRITER: exclusive lock
void writer(const char* filename, const char* data) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) { perror("open"); return; }
 
    // LOCK_EX: exclusive lock (blocks until available)
    if (flock(fd, LOCK_EX) == -1) { perror("flock"); close(fd); return; }
 
    write(fd, data, strlen(data));   // Write data while holding lock
    write(fd, "\n", 1);
 
    flock(fd, LOCK_UN);              // Release exclusive lock
    close(fd);
    cout << "Writer: wrote and released lock" << endl;
}
 
// (b) READER: shared lock
void reader(const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) { perror("open"); return; }
 
    // LOCK_SH: shared lock (multiple readers allowed simultaneously)
    if (flock(fd, LOCK_SH) == -1) { perror("flock"); close(fd); return; }
 
    char buf[256]; ssize_t n;
    cout << "Reader output:" << endl;
    while ((n = read(fd, buf, sizeof(buf)-1)) > 0) {
        buf[n] = '\0';
        cout << buf;
    }
    flock(fd, LOCK_UN);             // Release shared lock
    close(fd);
}
 
int main() {
    writer("shared.txt", "Process A data");
    writer("shared.txt", "Process B data");
    reader("shared.txt");
    return 0;
}
// LOCK_SH: multiple readers concurrently OK
// LOCK_EX: single writer; blocks all other readers and writers

