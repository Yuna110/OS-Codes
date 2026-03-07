#include <iostream>
#include <sys/mount.h>
#include <cerrno>
#include <cstring>
using namespace std;
 
int main() {
    // mount(source, target, filesystemtype, mountflags, data)
    // source     : device or filesystem to mount (e.g., "/dev/sdb1")
    // target     : mount point directory (must exist)
    // fstype     : filesystem type string (e.g., "ext4", "vfat")
    // mountflags : bitmask options (MS_RDONLY, MS_NOEXEC, etc.)
    // data       : filesystem-specific options string
 
    const char* source = "/dev/sdb1";     // Block device to mount
    const char* target = "/mnt/usb";      // Existing mount point
    const char* fstype = "vfat";          // Filesystem type
    unsigned long flags = MS_RDONLY;       // Read-only mount
    const char* options = "utf8";          // FS-specific options
 
    cout << "Attempting to mount " << source
         << " at " << target << "..." << endl;
 
    // Note: requires root privileges (CAP_SYS_ADMIN)
    if (mount(source, target, fstype, flags, options) == -1) {
        cerr << "mount() failed: " << strerror(errno) << endl;
        cerr << "(This is expected without root and real device)" << endl;
        return 1;
    }
    cout << "Mount successful!" << endl;
    // To unmount: umount(target) or umount2(target, MNT_FORCE)
    return 0;
}
// Key flags: MS_RDONLY (read-only), MS_NOEXEC (no execution),
// MS_NOSUID (ignore SUID bits), MS_REMOUNT (remount with new flags)

