#include "Context/Shader.h"
#include "Context/Context.h"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <string>

namespace mase {

Shader::Shader() {}

Shader::Shader(const std::string& code) {
  ci.setPCode((uint32_t*)code.data())
    .setCodeSize(code.size());
}

void Shader::Create() {
  m_shaderModule = Context::GetInstance().m_device.createShaderModule(ci);
}

void Shader::Create(const std::string& code) {
  ci.setPCode((uint32_t*)code.data())
    .setCodeSize(code.size());

  m_shaderModule = Context::GetInstance().m_device.createShaderModule(ci);
}

void Shader::Destroy() {
  Context::GetInstance().m_device.destroyShaderModule(m_shaderModule);
}

}
