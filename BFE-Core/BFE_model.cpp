#include "BFE_model.hpp"
#include "BFE_utils.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define GLM_ENABLE_EXPERINENTAL
#include <glm/gtx/hash.hpp>
#include <cstring>
#include <unordered_map>

namespace std {
	template <>
	struct hash<BFE::BFEModel::Vertex> {
		size_t operator()(BFE::BFEModel::Vertex const& bfertex) const {
			size_t seed = 0;
			BFE::hashCombine(seed, bfertex.position, bfertex.texCoord);
			return seed;
		}
	};

}
namespace BFE {
	BFEModel::BFEModel(size_t pid, BFEDevice& device, const BFEModel::Builder& builder) : bfeDevice{ device } {
		createVertexBuffers(pid, builder.vertices);
		createIndexBuffers(pid, builder.indices);
	}
	BFEModel::~BFEModel() {}

	std::unique_ptr<BFEModel> BFEModel::createModelFromFile(size_t pid, BFEDevice& device, const std::string& fpath) {
		Builder builder{};
		builder.loadModel(fpath);

		return std::make_unique<BFEModel>(pid, device, builder);
	}
	void BFEModel::createVertexBuffers(size_t pid, const std::vector<Vertex>& vertices) {
		vertexCount = static_cast<uint32_t>(vertices.size());
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		BFEBuffer stagingBuffer{ bfeDevice, vertexSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, };

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());

		vertexBuffer = std::make_unique<BFEBuffer>(bfeDevice, vertexSize, vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		bfeDevice.copyBuffer(pid, stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
	}

	void BFEModel::createIndexBuffers(size_t pid, const std::vector<uint32_t>& indices) {
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;
		if (!hasIndexBuffer) return;
		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		uint32_t indexSize = sizeof(indices[0]);
		BFEBuffer stagingBuffer{
			bfeDevice, indexSize, indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, };
		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());

		indexBuffer = std::make_unique<BFEBuffer>(bfeDevice, indexSize, indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		bfeDevice.copyBuffer(pid, stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
	}

	void BFEModel::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = { vertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}

	}
	void BFEModel::draw(VkCommandBuffer commandBuffer) {
		if (hasIndexBuffer) {
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		}

	}

	std::vector<VkVertexInputBindingDescription> BFEModel::Vertex::getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}
	std::vector<VkVertexInputAttributeDescription> BFEModel::Vertex::getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);



		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, texCoord);
		return attributeDescriptions;
	}

	void BFEModel::Builder::loadModel(const std::string& fpath) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, fpath.c_str())) {
			throw std::runtime_error(warn + err);
		}

		vertices.clear();
		indices.clear();
		std::unordered_map<Vertex, uint32_t> uniqueVertices{};
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex bfertex{};

				if (index.vertex_index >= 0) {
					bfertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2],
					};


					if (index.texcoord_index >= 0) {
						bfertex.texCoord = {
							attrib.texcoords[2 * index.texcoord_index + 0],
							attrib.texcoords[2 * index.texcoord_index + 1],
						};
					}

					if (uniqueVertices.count(bfertex) == 0) {
						uniqueVertices[bfertex] = static_cast<uint32_t>(vertices.size());
						vertices.push_back(bfertex);
					}
					indices.push_back(uniqueVertices[bfertex]);
				}
			}
		}
	}
}