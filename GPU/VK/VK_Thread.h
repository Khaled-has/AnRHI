#ifndef VK_THREAD_H
#define VK_THREAD_H

#include <iostream>
#include <functional>

#include "VK_CommandBufferPool.h"

namespace GPU
{

	class VK_Thread
	{
	public:
		VK_Thread() {}
		~VK_Thread() {}

		void Create();
		void Destroy();

		void FuncRunOnThread(std::function<void(const VkCommandBuffer& CmdBuf)> Func);

	private:
		VK_CommandBufferPool pCmdBufPool{};
		VkCommandBuffer pCmdBuf = VK_NULL_HANDLE;
	};

}

#endif