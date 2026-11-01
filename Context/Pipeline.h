#ifndef PIPELINE_H
#define PIPELINE_H

#include "vulkan/vulkan.hpp"
namespace mase {

class Pipeline {
public:
  vk::Pipeline m_pipeline;
  vk::PipelineLayout m_pipelineLayout;

  void Init();
  void Destroy();
};

}

#endif //!PIPELINE_H

