#ifndef VK_COMMANDBUFFERPOOL_H
#define VK_COMMANDBUFFERPOOL_H

#include <functional>

#include <vulkan/vulkan.h>

#define BIND_COMMAND_DRAW_FN(x) std::bind(x, this, std::placeholders::_1, std::placeholders::_2)

namespace GPU
{

	class VK_CommandBufferPool
	{
	public:
		VK_CommandBufferPool() {}
		~VK_CommandBufferPool() {}

		void Create();
		void Destroy();

		void CreateCommandBuffers(uint32_t Count, VkCommandBuffer* CmdBufs) const;
		void FreeCommandBuffers(uint32_t Count, VkCommandBuffer* CmdBufs);

	private:
		VkCommandPool pCommandPool = VK_NULL_HANDLE;
	};

	void BeginCommandBuffer(const VkCommandBuffer& CmdBuf, VkCommandBufferUsageFlags UsageFlags);
	void RecordCommandBuffer(const std::function<void(const VkCommandBuffer&, uint32_t ImageIndex)>& DrawCommand);

}

#endif