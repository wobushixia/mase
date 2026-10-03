#include "Base/File.h"
#include <cstdio>

namespace mase {

bool File::OpenFile(const std::string &file_path, const char* mode) {
  if(m_fp) {
    Close();
  }

  fopen(file_path.c_str(), mode);

  if(!m_fp) {
    return false;
  }

  return true;
}

bool File::OpenText(const std::string &file_path) {
  return OpenFile(file_path, "r");
}

bool File::OpenBinary(const std::string &file_path) {
  return OpenFile(file_path, "rb");
}

void File::Close() {
  if(!m_fp) {
    fclose(m_fp);
    m_fp = nullptr;
  }
}

bool File::ReadAll(char *buffer) {
  return true;
}

}

