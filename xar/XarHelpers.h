// Copyright (c) 2018-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.
//
// Helper functions; mainly here to make it testable rather than for
// actual re-use.

#pragma once

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Inspired by glog's CHECK/PCHECK, these macros don't rely on glog
// itself, which we intentionally avoid due to issues using it in a
// setuid context.
#define CHECK_SIMPLE(test) \
  do {                     \
    if (!(test)) {         \
      try {                \
        FATAL << #test;    \
      } catch (...) {      \
      }                    \
      abort();             \
    }                      \
  } while (0)
#define PCHECK_SIMPLE(test)                        \
  do {                                             \
    if (!(test)) {                                 \
      try {                                        \
        FATAL << #test << ": " << strerror(errno); \
      } catch (...) {                              \
      }                                            \
      abort();                                     \
    }                                              \
  } while (0)

// A simple, poor man's version of Google logging.  Use the FATAL
// macro and not this class directly.
#define FATAL                                \
  (::tools::xar::detail::LogFatal().stream() \
   << "FATAL " << __FILE__ << ":" << __LINE__ << ": ")

namespace tools {
namespace xar {

namespace detail {
class LogFatal {
 public:
  // The attributes here are to prevent optimizations that may
  // obfuscate our stack trace.
  ~LogFatal() __attribute__((__noreturn__, __noinline__));
  std::ostream& stream() {
    return ostream;
  }

 private:
  std::stringstream ostream;
};
} // namespace detail

// The unmount command used for unmounting the squash fs. Differs depending on
// OS.
extern const char* UNMOUNT_CMD;

namespace {
size_t delimSize(char) {
  return 1;
}
size_t delimSize(const std::string& s) {
  return s.size();
}
} // namespace

// A slow, simple split function.
template <typename DelimType>
std::vector<std::string> split(const DelimType& delim, std::string s) {
  std::vector<std::string> ret;

  while (true) {
    auto next = s.find(delim);
    if (next == std::string::npos) {
      ret.push_back(s);
      break;
    }

    ret.emplace_back(s.begin(), s.begin() + next);
    s.erase(s.begin(), s.begin() + next + delimSize(delim));
  }

  return ret;
}

// Return true if the host has enabled "user_allow_other" in
// /etc/fuse.conf.
//
// Takes path as a parameter for testing purposes.
bool fuse_allows_visible_mounts(std::string fuse_conf_path);

// Check if directory is owned by one of the groups the user is in.
bool is_user_in_group(gid_t dir_gid);

// Close all fds that aren't 0, 1, 2
void close_non_std_fds();

// Check if squashfs is mounted.
bool is_squashfs_mounted(const struct statfs& buf);

// Returns the default mount points for XAR.
std::vector<std::string> default_mount_roots();

// Prints a help message for when no mount roots can be found.
void no_mount_roots_help_message(std::ostream& out);
} // namespace xar
} // namespace tools
