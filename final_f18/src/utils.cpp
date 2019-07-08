#include <sys/stat.h>
#include <unistd.h>

#include "GTypes.h"

bool is_dir(const char path[]) {
    struct stat status;
    return !stat(path, &status) && (status.st_mode & S_IFDIR);
}

bool mk_dir(const char path[]) {
    if (is_dir(path)) {
        return true;
    }
    if (!access(path, F_OK)) {
        fprintf(stderr, "%s exists but is not a directory\n", path);
        return false;
    }

#ifdef _WIN32
    if (_mkdir(path)) {
        fprintf(stderr, "error creating dir %s\n", path);
        return false;
    }
#else
    if (mkdir(path, 0777)) {
        fprintf(stderr, "error creating dir %s\n", path);
        return false;
    }
#endif

    return true;
}


