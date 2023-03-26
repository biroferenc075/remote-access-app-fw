#pragma once

#include "BFE_device.hpp"
#include "BFE_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace BFE {
	class BFEModel {
	public:

		struct Vertex {
			glm::vec3 position{};
			glm::vec2 texCoord{};
			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
			bool operator==(const Vertex& other) const {
				return position == other.position &&
					texCoord == other.texCoord;
			}
		};

		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::string& fpath);
		};
		BFEModel(BFEDevice& device, const BFEModel::Builder &builder);
		~BFEModel();
		static std::unique_ptr<BFEModel> createModelFromFile(BFEDevice& device, const std::string& fpath);
		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);
		BFEDevice& bfeDevice;
		std::unique_ptr<BFEBuffer> vertexBuffer;
		uint32_t vertexCount;


		bool hasIndexBuffer = false;
		std::unique_ptr<BFEBuffer> indexBuffer;
		uint32_t indexCount;

		std::unique_ptr<BFEBuffer> textureBuffer;

		BFEModel(const BFEModel&);
		BFEModel& operator=(const BFEModel&);
	};
}