#include "VK_Thread.h"

#include <thread>

namespace GPU
{
	void VK_Thread::Create()
	{
		// # Create the command buffer pool
		pCmdBufPool.Create();
		// # Create the command buffer
		pCmdBufPool.CreateCommandBuffers(1, &pCmdBuf);
	}

	void VK_Thread::Destroy()
	{
		pCmdBufPool.FreeCommandBuffers(1, &pCmdBuf);
		pCmdBufPool.Destroy();
	}

	void VK_Thread::FuncRunOnThread(std::function<void(const VkCommandBuffer& CmdBuf)> Func)
	{
		std::thread (Func, pCmdBuf).detach();
	}

}