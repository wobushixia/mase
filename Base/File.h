#ifndef FILE_H
#define FILE_H

#include <cstdint>
#include <cstdio>
#include <string>

namespace mase {

class File {

public:
  File();
  ~File();

  FILE* GetFilePointer() { return m_fp; }
  int64_t GetFileSize() { return m_fileSize; } 

  bool OpenFile(const std::string &file_path, const char* mode);
  bool OpenText(const std::string &file_path);
  bool OpenBinary(const std::string &file_path);

  bool ReadAll(std::vector<char>* buffer);

  void Close();

private:
  FILE* m_fp;
  int64_t m_fileSize;
};

}

#endif // !FILE_H

