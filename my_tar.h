/* primary_header.h */
#ifndef MY_TAR_H
#define MY_TAR_H

#define TAR_HEADER_SIZE 512
#define TAR_FILENAME_LEN 100
#define TAR_MODE_LEN 8
#define TAR_UID_LEN 8
#define TAR_GID_LEN 8
#define TAR_SIZE_LEN 12
#define TAR_MTIME_LEN 12
#define TAR_CHKSUM_LEN 8
#define TAR_LINK_INDICATOR_LEN 1
#define TAR_LINKNAME_LEN 100
#define TAR_BLOCK_SIZE 512

typedef struct {
    char filename[TAR_FILENAME_LEN];
    char mode[TAR_MODE_LEN];
    char uid[TAR_UID_LEN];
    char gid[TAR_GID_LEN];
    char size[TAR_SIZE_LEN];
    char mtime[TAR_MTIME_LEN];
    char chksum[TAR_CHKSUM_LEN];
    char link[TAR_LINK_INDICATOR_LEN];
    char linkname[TAR_LINKNAME_LEN];
    char padding[TAR_HEADER_SIZE - TAR_FILENAME_LEN - TAR_MODE_LEN - TAR_UID_LEN - TAR_GID_LEN - TAR_SIZE_LEN - TAR_MTIME_LEN - TAR_CHKSUM_LEN - TAR_LINK_INDICATOR_LEN - TAR_LINKNAME_LEN];
} tar_header_t;



#endif /* MY_TAR_H */
