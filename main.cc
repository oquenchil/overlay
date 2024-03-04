#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>

#include <filesystem>
#include <iostream>
#include <vector>

uid_t global_outer_uid;
gid_t global_outer_gid;

void WriteFile(const std::string &filename, const char *fmt, ...) {
  FILE *stream = fopen(filename.c_str(), "w");
  if (stream == nullptr) {
    std::cout << "Buh 2" << std::endl;
  }

  va_list ap;
  va_start(ap, fmt);
  int r = vfprintf(stream, fmt, ap);
  va_end(ap);

  if (r < 0) {
  }

  if (fclose(stream) != 0) {
  }
}

int Simple(void *sync_pipe_param) {
  uid_t inner_uid = 0;
  gid_t inner_gid = 0;

  WriteFile("/proc/self/uid_map", "%u %u 1\n", inner_uid, global_outer_uid);
  WriteFile("/proc/self/gid_map", "%u %u 1\n", inner_gid, global_outer_gid);
  std::cout << "hello " << getuid() << std::endl;
  std::filesystem::remove_all("/tmp/mountdir");
  std::filesystem::remove_all("/tmp/lowerdir");
  std::filesystem::remove_all("/tmp/upperdir");
  std::filesystem::remove_all("/tmp/workdir");
  std::filesystem::create_directories("/tmp/mountdir");
  std::filesystem::create_directories("/tmp/lowerdir");
  std::filesystem::create_directories("/tmp/upperdir");
  std::filesystem::create_directories("/tmp/workdir");
  if (mount("overlay", "/tmp/mountdir", "overlay", 0,
            "lowerdir=/tmp/lowerdir,upperdir=/tmp/upperdir,workdir=/"
            "tmp/workdir") < 0) {
    std::cerr << "Error mounting " << errno << std::endl;
    exit(1);
  }
  return 0;
}

int main() {
  const int kStackSize = 1024 * 1024;
  std::vector<char> child_stack(kStackSize);

  global_outer_uid = getuid();
  global_outer_gid = getgid();

  int sync_pipe[2];

  int clone_flags =
      CLONE_NEWUSER | CLONE_NEWNS | CLONE_NEWIPC | CLONE_NEWPID | SIGCHLD;

  clone(Simple, child_stack.data() + kStackSize, clone_flags, sync_pipe);
  return 0;
}
