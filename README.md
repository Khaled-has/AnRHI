#AnRHI (Advanced Render Hardware Interface)

AnRHI is a lightweight and flexible Render Hardware Interface (RHI) designed for learning, experimentation, and building custom rendering systems.

The goal of AnRHI is to provide a clean abstraction over modern graphics APIs while still exposing enough control for advanced usage.

---

🚀 Current Status

- ✅ Vulkan: Fully supported
- ⏳ DirectX 12: Coming soon

AnRHI is designed from the ground up to support multiple graphics backends, allowing developers to target different platforms using a single codebase.

---

🌍 Cross-Platform Support

AnRHI supports multiple platforms with the same code:

- 🖥️ Windows
- 🐧 Linux
- 📱 Android

This is achieved by separating:

- Graphics backend (Vulkan / DX12)
- Windowing system (user-defined)

---

🧠 Design Philosophy

AnRHI is built with the following principles:

- Minimal abstraction
  Do not hide important GPU concepts.

- High performance
  Avoid unnecessary overhead (e.g. no per-frame command recording by default).

- User control
  The developer decides how to manage shaders, resources, and rendering flow.

- Modern API inspired
  Designed around Vulkan/DX12 concepts:
  
  - Command buffers
  - Render passes
  - Explicit synchronization

---

🪟 Window System (Flexible by Design)

AnRHI does not enforce any specific windowing library.

Instead, you can plug in any library by implementing a simple backend interface.

✅ Currently supported examples:

- SDL3
- GLFW
- Android Native Activity

You can easily create your own integration for any library you prefer.

pBackend->GetLibBackend() = lib_backend::CreateBackend_SDL3();
pBackend->GetLibBackend()->Init(windowHandle);

This design allows:

- Maximum flexibility
- Easy platform adaptation
- Clean separation between rendering and OS-specific code

---

⚙️ Core System Overview

🔹 GPU Backend

Handles the underlying graphics API (currently Vulkan).

RHI::GPU_Backend* backend = RHI::CreateVulkanBackend();

---

🔹 Buffers

Used for vertex data, dynamic updates, and GPU resources.

RHI::GPU_Buffer* buffer = RHI::CreateBuffer(data, size, RHI::GPU_BUFFER_TYPE_STATIC);

Supports:

- Static buffers
- Dynamic buffers (per-frame updates)

---

🔹 Draw Commands

Encapsulates draw calls and supports advanced techniques like:

- Instancing
- Indirect drawing (no need to re-record commands)

drawCmd->Draw(0, 3);

---

🔹 Shaders

Flexible shader system:

- User decides shader format (GLSL, SPIR-V, etc.)
- No forced unification between APIs

shader->InitFromFile("vertex.vert", "fragment.frag");

---

🔹 Render Pass System

Supports:

- Multiple color attachments
- Depth attachments
- Per-frame images (swapchain-aware)

RHI::GPU_RenderPass* renderPass = RHI::CreateRenderPass(info);

---

🔹 Command Recording

Commands are recorded once and reused for performance.

backend->BeginRecord();
// record commands
backend->EndRecord(renderPass);

You can re-record only when needed.

---

🔄 Frame System & Synchronization

AnRHI internally manages:

- Multiple swapchain images
- Per-frame resources
- Fences & semaphores
- Command buffers

This allows:

- Safe CPU/GPU parallelism
- Updating buffers while rendering

---

⚡ Performance Features

- ✅ Multi-buffering (frames in flight)
- ✅ No per-frame command recording required
- ✅ Instanced rendering
- ✅ Indirect draw support
- ✅ Internal synchronization system

---

🎯 Use Cases

- Learning modern graphics APIs (Vulkan / DX12)
- Building custom game engines
- Creating rendering experiments
- Developing tools (editors, visualizers, etc.)

---

🔮 Future Plans

- DirectX 12 backend
- More advanced resource management
- Descriptor system improvements
- GPU-driven rendering utilities

---

📌 Example

// Create backend
auto backend = RHI::CreateVulkanBackend();

// Attach window backend (any library)
backend->GetLibBackend() = lib_backend::CreateBackend_SDL3();
backend->GetLibBackend()->Init(window);

// Create buffer
auto buffer = RHI::CreateBuffer(vertices, size, STATIC);

// Create draw command
auto draw = RHI::CreateDraw();
draw->SetBuffer(buffer, STATIC, 0);
draw->Create();

// Record once
backend->BeginRecord();
// ...
backend->EndRecord(renderPass);

---

📖 Notes

AnRHI is primarily a learning-oriented project, but it follows real-world rendering architecture and can evolve into a production-ready system.

---

🤝 Contributing

Contributions, experiments, and ideas are welcome.

---

📄 License
You are free to use, modify, and distribute this project, even for commercial use, as long as you include the original license.