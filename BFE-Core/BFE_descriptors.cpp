#include "BFE_descriptors.hpp"

#include <cassert>
#include <stdexcept>

namespace BFE {

    // *************** Descriptor Set Layout Builder *********************

    BFEDescriptorSetLayout::Builder& BFEDescriptorSetLayout::Builder::addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count) {
        assert(bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        bindings[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<BFEDescriptorSetLayout> BFEDescriptorSetLayout::Builder::build() const {
        return std::make_unique<BFEDescriptorSetLayout>(bfeDevice, bindings);
    }

    // *************** Descriptor Set Layout *********************

    BFEDescriptorSetLayout::BFEDescriptorSetLayout(
        BFEDevice& bfeDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : bfeDevice{ bfeDevice }, bindings{ bindings } {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto kv : bindings) {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(
            bfeDevice.device(),
            &descriptorSetLayoutInfo,
            nullptr,
            &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    BFEDescriptorSetLayout::~BFEDescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(bfeDevice.device(), descriptorSetLayout, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    BFEDescriptorPool::Builder& BFEDescriptorPool::Builder::addPoolSize(
        VkDescriptorType descriptorType, uint32_t count) {
        poolSizes.push_back({ descriptorType, count });
        return *this;
    }

    BFEDescriptorPool::Builder& BFEDescriptorPool::Builder::setPoolFlags(
        VkDescriptorPoolCreateFlags flags) {
        poolFlags = flags;
        return *this;
    }
    BFEDescriptorPool::Builder& BFEDescriptorPool::Builder::setMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<BFEDescriptorPool> BFEDescriptorPool::Builder::build() const {
        return std::make_unique<BFEDescriptorPool>(bfeDevice, maxSets, poolFlags, poolSizes);
    }

    // *************** Descriptor Pool *********************

    BFEDescriptorPool::BFEDescriptorPool(
        BFEDevice& bfeDevice,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes)
        : bfeDevice{ bfeDevice } {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(bfeDevice.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    BFEDescriptorPool::~BFEDescriptorPool() {
        vkDestroyDescriptorPool(bfeDevice.device(), descriptorPool, nullptr);
    }

    bool BFEDescriptorPool::allocateDescriptor(
        const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenebfer an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(bfeDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
            return false;
        }
        return true;
    }

    void BFEDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
        vkFreeDescriptorSets(
            bfeDevice.device(),
            descriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void BFEDescriptorPool::resetPool() {
        vkResetDescriptorPool(bfeDevice.device(), descriptorPool, 0);
    }

    // *************** Descriptor Writer *********************

    BFEDescriptorWriter::BFEDescriptorWriter(BFEDescriptorSetLayout& setLayout, BFEDescriptorPool& pool)
        : setLayout{ setLayout }, pool{ pool } {}

    BFEDescriptorWriter& BFEDescriptorWriter::writeBuffer(
        uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    BFEDescriptorWriter& BFEDescriptorWriter::writeImage(
        uint32_t binding, VkDescriptorImageInfo* imageInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.dstBinding = 1;
        write.dstArrayElement = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    bool BFEDescriptorWriter::build(VkDescriptorSet& set) {
        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
        if (!success) {
            return false;
        }
        overwrite(set);
        return true;
    }

    void BFEDescriptorWriter::overwrite(VkDescriptorSet& set) {
        for (auto& write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.bfeDevice.device(), writes.size(), writes.data(), 0, nullptr);
    }
}
