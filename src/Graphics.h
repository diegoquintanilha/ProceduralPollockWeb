#pragma once

#include <webgpu/webgpu_cpp.h>

namespace Graphics
{
	// Setup
	void Initialize(const char* shaderCode);
	void GetInstance();
	void GetAdapter(WGPURequestAdapterStatus status, WGPUAdapter cAdapter, const char* message, void* userdata);
	void GetDevice(WGPURequestDeviceStatus status, WGPUDevice cDevice, const char* message, void* userdata);
	void SetupPipeline();

	// Runtime
	void Update();
}

