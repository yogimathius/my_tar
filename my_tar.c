#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

typedef struct {
  char filename[256];
  size_t size;
  time_t mod_time;
} FileEntry;

#define TAR_BLOCK_SIZE 512

void error_msg(int fd, const char *msg) { write(fd, msg, strlen(msg)); }

void my_printf(int fd, const char *msg) { write(fd, msg, strlen(msg)); }

void my_strncpy(char *dest, const char *src, size_t n) {
  size_t i;
  for (i = 0; i < n && src[i] != '\0'; i++) {
    dest[i] = src[i];
  }
  for (; i < n; i++) {
    dest[i] = '\0';
  }
}

int contains_dir(char *pathname) {
    for (int i = 0; i < strlen(pathname); i++) {
        if (pathname[i] == '/') {
            return i;
        }
    }
    return 0;
}

char* copyString(char s[], int size)
{
    char* s2;
    s2 = (char*)malloc(size);
 
    strcpy(s2, s);
    return (char*)s2;
}

void my_memset(void *s, int c, size_t n) {
  unsigned char* p = s;
  while (n--) {
    *p++ = (unsigned char)c;
  }
}

void create_archive(const char *filename, char *files[], int file_count) {
  int archive_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (archive_fd == -1) {
    error_msg(STDERR_FILENO, "Cannot create archive\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < file_count; ++i) {
    int file_fd = open(files[i], O_RDONLY);
    if (file_fd == -1) {
      close(archive_fd);
      fprintf(stderr, "tar: %s: Cannot stat: No such file or directory\n", files[i]);
      exit(EXIT_FAILURE);
    }

    struct stat file_stat;
    if (fstat(file_fd, &file_stat) != 0) {
      error_msg(STDERR_FILENO, "Cannot stat file\n");
      close(file_fd);
      close(archive_fd);
      exit(EXIT_FAILURE);
    }

    FileEntry entry;
    my_memset(&entry, 0, sizeof(entry)); 
    my_strncpy(entry.filename, files[i], sizeof(entry.filename) - 1);
    entry.size = file_stat.st_size; 
    entry.mod_time = file_stat.st_mtime;

    if (write(archive_fd, &entry, sizeof(entry)) != sizeof(entry)) {
      error_msg(STDERR_FILENO, "Failed to write file entry to archive\n");
      close(file_fd);
      close(archive_fd);
      exit(EXIT_FAILURE);
    }

    char buffer[TAR_BLOCK_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = read(file_fd, buffer, TAR_BLOCK_SIZE)) > 0) {
      if (write(archive_fd, buffer, bytesRead) != bytesRead) {
        error_msg(STDERR_FILENO, "Failed to write file contents to archive\n");
        close(file_fd);
        close(archive_fd);
        exit(EXIT_FAILURE);
      }
    }

    if (bytesRead == -1) {
      error_msg(STDERR_FILENO, "Failed to read file contents\n");
      close(file_fd);
      close(archive_fd);
      exit(EXIT_FAILURE);
    }

    close(file_fd);
  }

  close(archive_fd);
}

void append_archive(const char *filename, char *files[], int file_count) {
  // TODO: Implement
  int i = 0;
  int tarfile_name = open(filename, O_RDONLY);

  if (tarfile_name == -1) {
    perror("Failed to open the file");
    exit(EXIT_FAILURE);
  }

  while (i < file_count) {
    printf("iterating over: %s\n", files[i]);
    i++;
  }
}

void list_archive(const char *filename) {
  int archive_fd = open(filename, O_RDONLY);
  if (archive_fd == -1) {
    error_msg(STDERR_FILENO, "Failed to open the file\n");
    return;
  }

  FileEntry entry;
  while (read(archive_fd, &entry, sizeof(entry)) == sizeof(entry)) {
    if (entry.filename[0] == '\0') {
      break;
    }

    my_printf(STDOUT_FILENO, entry.filename);
    my_printf(STDOUT_FILENO, "\n");

    if (lseek(archive_fd, entry.size, SEEK_CUR) == -1) {
      error_msg(STDERR_FILENO, "Seek failed\n");
      close(archive_fd);
      return;
    }
  }

  close(archive_fd);
}

void update_archive(const char *filename, char *files[], int file_count) {
  // TODO: Implement
}

void extract_archive(const char *filename) {
  int archive_fd = open(filename, O_RDONLY);
  if (archive_fd == -1) {
    error_msg(STDERR_FILENO, "Failed to open the file\n");
    return;
  }

  FileEntry entry;
  while (read(archive_fd, &entry, sizeof(entry)) == sizeof(entry)) {
    if (entry.filename[0] == '\0') {
      break;
    }
    int i;
    char *dir;
    if ((i = contains_dir(entry.filename)) > 0) {
        dir = copyString(entry.filename, i);
        dir[i] = '\0';
        printf("dir copied: %s\n", dir);
        mkdir(dir, S_IRWXU | S_IRWXG | S_IRWXO);
    }

    int file_desc = open(entry.filename, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);

    if (file_desc == -1) {
        error_msg(STDERR_FILENO, "Cannot extract file\n");
        exit(EXIT_FAILURE);
    }

    char buffer[TAR_BLOCK_SIZE];
    write(file_desc, buffer, entry.size);
    my_printf(STDOUT_FILENO, entry.filename);
    my_printf(STDOUT_FILENO, "\n");

    char *fileData = malloc(entry.size);
    read(file_desc, fileData, entry.size);

    printf("file data: %lu\n", entry.size);

    if (lseek(archive_fd, entry.size, SEEK_CUR) == -1) {
      error_msg(STDERR_FILENO, "Seek failed\n");
      close(archive_fd);
      return;
    }
  }

  close(archive_fd);
}

int main(int argc, char **argv) {

  const char *mode = argv[1];
  const char *filename = argv[2];
  if (strcmp(mode, "-cf") == 0) {
    create_archive(filename, &argv[3], argc - 3);
  } else if (strcmp(mode, "-rf") == 0) {
    append_archive(filename, &argv[3], argc - 3);
  } else if (strcmp(mode, "-tf") == 0) {
    list_archive(filename);
  } else if (strcmp(mode, "-uf") == 0) {
    update_archive(filename, &argv[3], argc - 3);
  } else if (strcmp(mode, "-xf") == 0) {
    extract_archive(filename);
  } else {
    fprintf(stderr, "Invalid mode: %s\n", mode);
    return 1;
  }

  return 0;
}

/*
Can you run with ./my_tar -cf file.tar source.c source.h?
Can inspect the content of the tarball with  ./my_tar -tf file.tar
Can you run with  ./my_tar -rf file.tar new_source.c
Can you extract the tarball with ./my_tar -xf file.tar
Does the command ./my_tar -cf file.tar source.h i_don_t_exist handles file that
doesn't exist?
Can you use my_tar to untar an archive created by the real tar?
*/

/*
-c Create a new archive containing the specified items.
-r Like -c, but new entries are appended to the archive. The -f option is
required. -t List archive contents to stdout. -u Like -r, but new entries are
added only if they have a modification date newer than the corresponding entry
in the archive. The -f option is required. -x Extract to disk from the archive.
If a file with the same name appears more than once in the archive, each copy
will be extracted, with later copies overwriting (replacing) earlier copies.
*/