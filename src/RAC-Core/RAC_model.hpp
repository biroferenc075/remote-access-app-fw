#pragma once

#include "RAC_device.hpp"
#include "RAC_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace RAC {
	class RACModel {
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
		RACModel(size_t pid, RACDeviceBase& device, const RACModel::Builder &builder);
		~RACModel();
		static std::unique_ptr<RACModel> createModelFromFile(size_t pid, RACDeviceBase& device, const std::string& fpath);
		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
	private:
		void createVertexBuffers(size_t pid, const std::vector<Vertex>& vertices);
		void createIndexBuffers(size_t pid, const std::vector<uint32_t>& indices);
		RACDeviceBase& racDevice;
		std::unique_ptr<RACBuffer> vertexBuffer;
		uint32_t vertexCount;


		bool hasIndexBuffer = false;
		std::unique_ptr<RACBuffer> indexBuffer;
		uint32_t indexCount;

		std::unique_ptr<RACBuffer> textureBuffer;

		RACModel(const RACModel&);
		RACModel& operator=(const RACModel&);
	};
}