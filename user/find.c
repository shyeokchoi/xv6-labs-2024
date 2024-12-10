// clang-format off
#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
// clang-format on
#include "user/user.h"

#define BUF_SIZE 1000

int compare_file_name(char* path, char* file_name)
{
    uint path_len = strlen(path);
    uint file_name_len = strlen(file_name);

    if (file_name_len > path_len) {
        return 0;
    }

    char* pptr = path + path_len - file_name_len;
    char* fptr = file_name;
    while (*pptr == *fptr) {
        if (*pptr == '\0') {
            return 1;
        }
        ++pptr;
        ++fptr;
    }
    return 0;
}

char* format_path(char* path, char* file_name)
{
    static char buf[BUF_SIZE];
    uint64 path_len = strlen(path);
    uint64 file_name_len = strlen(file_name);
    int ptr = 0;

    memcpy(buf, path, path_len);
    ptr += path_len;
    buf[ptr] = '/';
    ++ptr;
    memcpy(buf + ptr, file_name, file_name_len);
    ptr += file_name_len;
    buf[ptr] = '\0';
    return buf;
}

int search(char* path, char* file_name)
{
    struct stat st;
    int fd;

    if ((fd = open(path, O_RDONLY)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return -1;
    }

    if ((fstat(fd, &st)) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return -1;
    }

    if (st.type == T_DIR) {
        uint64 bytes_read = 0;
        char* buf = malloc(st.size);
        struct dirent* dirent_arr = (struct dirent*)buf; // easy access to buf

        uint64 n; // temp var to store return value of `read` function for each call
        while ((n = read(fd, buf + bytes_read, st.size - bytes_read)) > 0) {
            bytes_read += n;
        }

        if (n < 0) {
            fprintf(2, "read error.\n");
            free(buf);
            return -1;
        }

        // now, buffer full of the directory file
        for (int i = 0; i < st.size / sizeof(struct dirent); ++i) {
            if (dirent_arr[i].inum == 0 || strcmp(dirent_arr[i].name, ".") == 0
                || strcmp(dirent_arr[i].name, "..") == 0) {
                continue;
            }

            if (strlen(path) + 1 + strlen(dirent_arr[i].name) + 1 > BUF_SIZE) { /* plus 1's for '/' and '\0' */
                fprintf(2, "too long path.\n");
                free(buf);
                close(fd);
                return -1;
            }

            char* next = format_path(path, dirent_arr[i].name);
            char call_buf[BUF_SIZE];
            strcpy(call_buf, next);
            if (search(call_buf, file_name) < 0) {
                free(buf);
                close(fd);
                return -1;
            }
        }
        free(buf);
        close(fd);
        return 0;
    } else if (st.type == T_FILE) {
        if (compare_file_name(path, file_name)) { /* when file name match */
            fprintf(1, "%s\n", path);
        }
    }

    close(fd);
    return 0;
}

int main(int argc, char* argv[])
{
    int res;

    if (argc != 3) {
        fprintf(1, "Usage: find <root> <file_name>\n");
        exit(0);
    }

    res = search(argv[1], argv[2]);
    if (res < 0) {
        fprintf(2, "find failed\n");
        exit(1);
    }

    exit(0);
}
