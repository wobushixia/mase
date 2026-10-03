#ifndef SHADER_H
#define SHADER_H

#include "vulkan/vulkan.hpp"
#include <string>

namespace mase {

class Shader {
public:
  Shader(const std::string& path);
  ~Shader();

  vk::ShaderModule m_shaderModule;

private:

};

}

#endif // !SHADER_H
