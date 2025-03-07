#include <iostream>
#include <string>
#include <chrono>

#include "Shader.h"
#include "Graphics.h"

int main()
{
	// Get time at the beginning of the program to use as an initial seed
	auto now = std::chrono::high_resolution_clock::now();
	uint64_t currentTime = std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch().count();

	// Generate the first shader using time as seed
	std::string pixelShader = GenerateShaderCode(currentTime);
	
	// Create window and initialize graphics API
	Graphics::Initialize(pixelShader.c_str());

	return 0;
}

