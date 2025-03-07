#include "Graphics.h"

#include <iostream>

#include <webgpu/webgpu_cpp.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

namespace
{
	const char* m_ShaderCode;
	double m_LastUpdate = 0.0f;

	// WebGPU core objects
	wgpu::Instance m_Instance;
	wgpu::Adapter m_Adapter;
	wgpu::Device m_Device;

	// Reference to the surface of the canvas
	wgpu::Surface m_Surface;

	// Objects to interact with the shader
	wgpu::Buffer m_Buffer;
	wgpu::BindGroup m_BindGroup;

	// Pipeline representation that holds the shader
	wgpu::RenderPipeline m_Pipeline;
}

void Graphics::Initialize(const char* shaderCode)
{
	// Store shader code
	m_ShaderCode = shaderCode;

	// This is the first function call in a sequence of async function calls to setup the WebGPU environment
	GetInstance();
}
void Graphics::GetInstance()
{
	// Get instance
	m_Instance = wgpu::CreateInstance();
	// Call the next async setup function
	m_Instance.RequestAdapter(nullptr, GetAdapter, nullptr);
}
void Graphics::GetAdapter(WGPURequestAdapterStatus status, WGPUAdapter cAdapter, const char* message, void* userdata)
{
	if (status != WGPURequestAdapterStatus_Success)
	{
		std::cerr << "Could not get adapter" << std::endl;
		return;
	}

	// Get adapter
	m_Adapter = wgpu::Adapter::Acquire(cAdapter);
	// Call the next async setup function
	m_Adapter.RequestDevice(nullptr, GetDevice, nullptr);
}
void Graphics::GetDevice(WGPURequestDeviceStatus status, WGPUDevice cDevice, const char* message, void* userdata)
{
	// Get device
	m_Device = wgpu::Device::Acquire(cDevice);
	m_Device.SetUncapturedErrorCallback
	(
		[](WGPUErrorType type, const char* message, void* userdata) { std::cout << "Uncaptured error (captured by C++): " << type << " - Message: " << message; },
		nullptr
	);
	
	// Setup the rest of the graphics pipeline
	SetupPipeline();
}
void Graphics::SetupPipeline()
{
	#pragma region Surface
	
	// Create the surface
	wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDescriptor{};
	canvasDescriptor.selector = "#canvas";
	wgpu::SurfaceDescriptor surfaceDescriptor{ .nextInChain = &canvasDescriptor };
	m_Surface = m_Instance.CreateSurface(&surfaceDescriptor);

	// Create the format
	wgpu::SurfaceCapabilities capabilities;
	m_Surface.GetCapabilities(m_Adapter, &capabilities);
	wgpu::TextureFormat format = capabilities.formats[0];

	// Configure the surface
	wgpu::SurfaceConfiguration config
	{
		.device = m_Device,
		.format = format,
	};
	m_Surface.Configure(&config);

	#pragma endregion

	#pragma region Buffer

	// Uniform buffer descriptor
    wgpu::BufferDescriptor ubd =
	{
		.usage				= wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,	// Flags for CPU write and GPU read
		.size				= 4 * sizeof(float),										// Uniform buffer size
		.mappedAtCreation	= false														// We will not set any default values at creation
	};

    // Create the uniform buffer
    m_Buffer = m_Device.CreateBuffer(&ubd);

	#pragma endregion

	#pragma region Bind Group

    // Uniform buffer layout
    wgpu::BindGroupLayoutEntry bindGroupLayoutEntry =
	{
		.binding = 0, // Matches binding @binding(0) in WGSL
		.visibility = wgpu::ShaderStage::Fragment,
		.buffer = { .type = wgpu::BufferBindingType::Uniform }
	};

	// Bind group layout
    wgpu::BindGroupLayoutDescriptor bgld =
	{
		.entryCount = 1,
		.entries = &bindGroupLayoutEntry
	};
    wgpu::BindGroupLayout bindGroupLayout = m_Device.CreateBindGroupLayout(&bgld);

    // Bind group entry
    wgpu::BindGroupEntry bindGroupEntry =
	{
		.binding	= 0,
		.buffer		= m_Buffer,				// Buffer object
		.offset		= 0,
		.size		= 4 * sizeof(float)		// Uniform buffer size
	};

    // Bind group
    wgpu::BindGroupDescriptor bgd =
	{
		.layout = bindGroupLayout,
		.entryCount = 1,
		.entries = &bindGroupEntry
	};
    m_BindGroup = m_Device.CreateBindGroup(&bgd);

	#pragma endregion

	#pragma region Shader and Pipeline

    // Pipeline layout descriptor
    wgpu::PipelineLayoutDescriptor pld =
	{
		.bindGroupLayoutCount = 1,
		.bindGroupLayouts = &bindGroupLayout
	};
	
	// Get shader code
    wgpu::ShaderModuleWGSLDescriptor wgsld{};
    wgsld.code = m_ShaderCode;

	// Compile shader code
    wgpu::ShaderModuleDescriptor shaderModuleDescriptor{ .nextInChain = &wgsld };
    wgpu::ShaderModule shaderModule = m_Device.CreateShaderModule(&shaderModuleDescriptor);
    
	// Fragment shader
    wgpu::ColorTargetState colorTargetState{ .format = format };
    wgpu::FragmentState fragmentState
    {
        .module = shaderModule,
        .targetCount = 1,
        .targets = &colorTargetState
    };

    // Create the pipeline
    wgpu::RenderPipelineDescriptor rpd =
	{
		.layout = m_Device.CreatePipelineLayout(&pld),
		.vertex = { .module = shaderModule },
		.fragment = &fragmentState
	};
    m_Pipeline = m_Device.CreateRenderPipeline(&rpd);
	
	#pragma endregion

	// Set the Update function as the main loop
	emscripten_set_main_loop(Update, 0, false);
}

void Graphics::Update()
{
	// Get frame time (for debug purposes)
//	double now = emscripten_get_now() / 1000.0;
//	double deltaTime = now - m_LastUpdate;
//	m_LastUpdate = now;
//	std::cout << deltaTime << " ms" << std::endl;
//	std::cout << 1.0 / deltaTime << " FPS" << std::endl;

	// Get time in seconds since the beginning of the program
	float elapsedTime = emscripten_get_now() / 1000.0f;

	// Calculate sin and cos of time to pass as constant buffers to the shader and use as transition alphas
	float sinTime = 0.5f + 0.5f * sinf(0.5f * elapsedTime);
	float cosTime = 0.5f + 0.5f * cosf(0.5f * elapsedTime);

    // Assemble the data into an array
	const float newData[] = { sinTime, cosTime, 0.0f, 0.0f };

	// Update the uniform buffer
    m_Device.GetQueue().WriteBuffer(m_Buffer, 0, &newData, 4 * sizeof(float));

	// Get the current surface texture
	wgpu::SurfaceTexture surfaceTexture;
	m_Surface.GetCurrentTexture(&surfaceTexture);

	// Render pass color attachment
	wgpu::RenderPassColorAttachment attachment
	{
		.view = surfaceTexture.texture.CreateView(),
		.loadOp = wgpu::LoadOp::Clear,
		.storeOp = wgpu::StoreOp::Store
	};

	// Render pass descriptor
	wgpu::RenderPassDescriptor rpd
	{
		.colorAttachmentCount = 1,
		.colorAttachments = &attachment
	};

	// Create command encoder
	wgpu::CommandEncoder encoder = m_Device.CreateCommandEncoder();
	
	// Begin the render pass
	wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rpd);
	
	// Bind the pipeline
	pass.SetPipeline(m_Pipeline);

	// Bind the uniform buffer
	pass.SetBindGroup(0, m_BindGroup);

	// Draw the fullscreen quad
	pass.Draw(6);
	
	// End render pass
	pass.End();

	// Submit the commands
	wgpu::CommandBuffer commands = encoder.Finish();
	m_Device.GetQueue().Submit(1, &commands);
}

