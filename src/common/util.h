// Copyright (c) 2017-2018, The Fonero Project.
// Copyright (c) 2014-2017 The Monero Project.
// Portions Copyright (c) 2012-2013 The Cryptonote developers.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

#pragma once

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <system_error>
#include <csignal>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>

#include "crypto/hash.h"

/*! \brief Various Tools
 *
 *
 *
 */
namespace tools
{
  //! Functional class for closing C file handles.
  struct close_file
  {
    void operator()(std::FILE* handle) const noexcept
    {
      if (handle)
      {
        std::fclose(handle);
      }
    }
  };

  //! \return File only readable by owner. nullptr if `filename` exists.
  std::unique_ptr<std::FILE, close_file> create_private_file(const std::string& filename);

  /*! \brief Returns the default data directory.
   *
   * \details Windows < Vista: C:\\Documents and Settings\\Username\\Application Data\\CRYPTONOTE_NAME
   *
   * Windows >= Vista: C:\\Users\\Username\\AppData\\Roaming\\CRYPTONOTE_NAME
   *
   * Mac: ~/Library/Application Support/CRYPTONOTE_NAME
   *
   * Unix: ~/.CRYPTONOTE_NAME
   */
  std::string get_default_data_dir();

#ifdef WIN32
  /**
   * @brief
   *
   * @param nfolder
   * @param iscreate
   *
   * @return
   */
  std::string get_special_folder_path(int nfolder, bool iscreate);
#endif

  /*! \brief Returns the OS version string
   *
   * \details This is a wrapper around the primitives
   * get_windows_version_display_string() and
   * get_nix_version_display_string()
   */
  std::string get_os_version_string();

  /*! \brief creates directories for a path
   *
   *  wrapper around boost::filesyste::create_directories.
   *  (ensure-directory-exists): greenspun's tenth rule in action!
   */
  bool create_directories_if_necessary(const std::string& path);
  /*! \brief std::rename wrapper for nix and something strange for windows.
   */
  std::error_code replace_file(const std::string& replacement_name, const std::string& replaced_name);

  bool sanitize_locale();

  /*! \brief Defines a signal handler for win32 and *nix
   */
  class signal_handler
  {
  public:
    /*! \brief installs a signal handler  */
    template<typename T>
    static bool install(T t)
    {
#if defined(WIN32)
      bool r = TRUE == ::SetConsoleCtrlHandler(&win_handler, TRUE);
      if (r)
      {
        m_handler = t;
      }
      return r;
#else
      /* Only blocks SIGINT and SIGTERM */
      signal(SIGINT, posix_handler);
      signal(SIGTERM, posix_handler);
      m_handler = t;
      return true;
#endif
    }

  private:
#if defined(WIN32)
    /*! \brief Handler for win */
    static BOOL WINAPI win_handler(DWORD type)
    {
      if (CTRL_C_EVENT == type || CTRL_BREAK_EVENT == type)
      {
        handle_signal(type);
      }
      else
      {
        MGINFO_RED("Got control signal " << type << ". Exiting without saving...");
        return FALSE;
      }
      return TRUE;
    }
#else
    /*! \brief handler for NIX */
    static void posix_handler(int type)
    {
      handle_signal(type);
    }
#endif

    /*! \brief calles m_handler */
    static void handle_signal(int type)
    {
      static boost::mutex m_mutex;
      boost::unique_lock<boost::mutex> lock(m_mutex);
      m_handler(type);
    }

    /*! \brief where the installed handler is stored */
    static std::function<void(int)> m_handler;
  };

  void set_strict_default_file_permissions(bool strict);

  void set_max_concurrency(unsigned n);
  unsigned get_max_concurrency();

  bool is_local_address(const std::string &address);
  int vercmp(const char *v0, const char *v1); // returns < 0, 0, > 0, similar to strcmp, but more human friendly than lexical - does not attempt to validate

  bool sha256sum(const uint8_t *data, size_t len, crypto::hash &hash);
  bool sha256sum(const std::string &filename, crypto::hash &hash);
}
