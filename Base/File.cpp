#include "Base/File.h"
#include <cstdio>

namespace mase {

bool File::OpenFile(const std::string &file_path, const char* mode) {
  if(m_fp) {
    Close();
  }

  m_fp = fopen(file_path.c_str(), mode);

  if(!m_fp) {
    return false;
  }

  if (fseek(m_fp, 0, SEEK_END) != 0) { return false; }
  m_fileSize = ftell(m_fp);
  if (m_fileSize < 0) { return false; }
  rewind(m_fp);

  return true;
}

bool File::OpenText(const std::string &file_path) {
  return OpenFile(file_path, "r");
}

bool File::OpenBinary(const std::string &file_path) {
  return OpenFile(file_path, "rb");
}

void File::Close() {
  if(m_fp) {
    fclose(m_fp);
    m_fp = nullptr;
  }
}

bool File::ReadAll(std::string *buffer) {
	if(buffer == nullptr || m_fp == nullptr) {
		return false;
	}

	buffer->resize(m_fileSize);
	
	if(fseek(m_fp, 0, SEEK_SET) != 0) {
		return false;
	}
	if(fread((*buffer).data(), 1, m_fileSize, m_fp) != m_fileSize) {
		return false;
	}

  return true;
}

File::~File() {
  Close();
}

File::File() : m_fp(nullptr), m_fileSize(0) {}

}

