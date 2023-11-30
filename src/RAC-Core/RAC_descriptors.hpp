#pragma once

#include "RAC_device.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace RAC {

class RACDescriptorSetLayout {
 public:
  class Builder {
   public:
    Builder(RACDeviceBase &racDevice) : racDevice{racDevice} {}

    Builder &addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count = 1);
    std::unique_ptr<RACDescriptorSetLayout> build() const;

   private:
       RACDeviceBase &racDevice;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };

  RACDescriptorSetLayout(
      RACDeviceBase& racDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  ~RACDescriptorSetLayout();
  RACDescriptorSetLayout(const RACDescriptorSetLayout&) = delete;
  RACDescriptorSetLayout&operator=(const RACDescriptorSetLayout&) = delete;

  VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

 private:
     RACDeviceBase& racDevice;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

  friend class RACDescriptorWriter;
};

class RACDescriptorPool {
 public:
  class Builder {
   public:
       Builder(RACDeviceBase& racDevice) : racDevice{ racDevice } {}

    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<RACDescriptorPool> build() const;

   private:
       RACDeviceBase& racDevice;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };

  RACDescriptorPool(
      RACDeviceBase& racDevice,
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~RACDescriptorPool();
  RACDescriptorPool(const RACDescriptorPool&) = delete;
  RACDescriptorPool&operator=(const RACDescriptorPool&) = delete;

  bool allocateDescriptor(
      const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

  void resetPool();

 private:
  RACDeviceBase& racDevice;
  VkDescriptorPool descriptorPool;

  friend class RACDescriptorWriter;
};

class RACDescriptorWriter {
 public:
     RACDescriptorWriter(RACDescriptorSetLayout &setLayout,RACDescriptorPool &pool);

     RACDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
     RACDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);

 private:
  RACDescriptorSetLayout &setLayout;
  RACDescriptorPool &pool;
  std::vector<VkWriteDescriptorSet> writes;
};

}