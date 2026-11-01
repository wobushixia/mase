#ifndef SHADER_H
#define SHADER_H

#include "vulkan/vulkan.hpp"
#include <string>

namespace mase {

class Shader {
public:
  Shader(const std::string& code);
  Shader();
  void Destroy();

  void Create();
  void Create(const std::string& code);

  vk::ShaderModule GetShaderModule() { return m_shaderModule; };

  vk::ShaderModule m_shaderModule;

private:
  vk::ShaderModuleCreateInfo ci;
};

}

#endif // !SHADER_H
