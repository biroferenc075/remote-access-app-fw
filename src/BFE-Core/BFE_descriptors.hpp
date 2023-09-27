#pragma once

#include "BFE_device.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace BFE {

class BFEDescriptorSetLayout {
 public:
  class Builder {
   public:
    Builder(BFEDeviceBase &bfeDevice) : bfeDevice{bfeDevice} {}

    Builder &addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count = 1);
    std::unique_ptr<BFEDescriptorSetLayout> build() const;

   private:
       BFEDeviceBase &bfeDevice;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };

  BFEDescriptorSetLayout(
      BFEDeviceBase& bfeDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  ~BFEDescriptorSetLayout();
  BFEDescriptorSetLayout(const BFEDescriptorSetLayout&) = delete;
  BFEDescriptorSetLayout&operator=(const BFEDescriptorSetLayout&) = delete;

  VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

 private:
     BFEDeviceBase& bfeDevice;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

  friend class BFEDescriptorWriter;
};

class BFEDescriptorPool {
 public:
  class Builder {
   public:
       Builder(BFEDeviceBase& bfeDevice) : bfeDevice{ bfeDevice } {}

    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<BFEDescriptorPool> build() const;

   private:
       BFEDeviceBase& bfeDevice;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };

  BFEDescriptorPool(
      BFEDeviceBase& bfeDevice,
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~BFEDescriptorPool();
  BFEDescriptorPool(const BFEDescriptorPool&) = delete;
  BFEDescriptorPool&operator=(const BFEDescriptorPool&) = delete;

  bool allocateDescriptor(
      const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

  void resetPool();

 private:
  BFEDeviceBase& bfeDevice;
  VkDescriptorPool descriptorPool;

  friend class BFEDescriptorWriter;
};

class BFEDescriptorWriter {
 public:
     BFEDescriptorWriter(BFEDescriptorSetLayout &setLayout,BFEDescriptorPool &pool);

     BFEDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
     BFEDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);

 private:
  BFEDescriptorSetLayout &setLayout;
  BFEDescriptorPool &pool;
  std::vector<VkWriteDescriptorSet> writes;
};

}