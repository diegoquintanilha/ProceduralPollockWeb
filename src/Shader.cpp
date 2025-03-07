#include "Shader.h"

#include <iostream>
#include <string>

#define RANDFS_IMPLEMENTATION
#include "RandFS.h"

// Comment the line below to generate static images
#define ANIMATE

std::string GenerateShaderCode(uint64_t seed)
{
	// Uncomment here to set a specific seed
//	seed = 302817110064ULL;
	Random rand(seed);

	// Depths between 6 and 12 tend to generate interesting images
	// Add two random values to bias towards the middle (9)
	int maxDepth = rand.IntBetween(3, 7) + rand.IntBetween(3, 7);

	#pragma region Function definitions

	static constexpr char functionDefinitions[] =
	R"(
	
	// 1 input
	
	fn fInv(x: f32) -> f32
	{
		return 1.0f - x;
	}
	
	fn fSqr(x: f32) -> f32
	{
		return x * x;
	}
	
	fn fSqrt(x: f32) -> f32
	{
		return sqrt(x);
	}
	
	fn fSmooth(x: f32) -> f32
	{
		let x2: f32 = x * x;
		let x3: f32 = x2 * x;
		return x2 + x2 + x2 - x3 - x3;
	}

	fn fSharp(x: f32) -> f32
	{
		return x * (x * (x + x - 3.0f) + 2.0f);
	}
	
	// -------------------------------------
	// 2 inputs
	
	fn fAdd(x: f32, y: f32) -> f32
	{
		let res: f32 = x + y;
		if (res > 1.0f)
		{
			return 2.0f - res;
		}
		return res;
	}
	
	fn fSub(x: f32, y: f32) -> f32
	{
		let res: f32 = x - y;
		if (res < 0.0f)
		{
			return -res;
		}
		return res;
	}
	
	fn fMul(x: f32, y: f32) -> f32
	{
		return x * y;
	}
		
	fn fDiv(x: f32, y: f32) -> f32
	{
		var min: f32 = x;
		var max: f32 = y;
		
		if (x > y)
		{
			min = y;
			max = x;
		}
		if (max < 0.0001f)
		{
			max = 0.0001f;
		}
		return min / max;
	}
	
	fn fAvg(x: f32, y: f32) -> f32
	{
		return (x + y) * 0.5f;
	}
	
	fn fGeom(x: f32, y: f32) -> f32
	{
		return sqrt(x * y);
	}
	
	fn fHarm(x: f32, y: f32) -> f32
	{
		var den: f32 = x + y;
		if (den < 0.0001f)
		{
			den = 0.0001f;
		}
		return (2.0f * x * y) / den;
	}

	fn fHypo(x: f32, y: f32) -> f32
	{
		return 0.70710678f * sqrt(x * x + y * y); // Scale by 1 / sqrt(2)
	}	

	fn fMax(x: f32, y: f32) -> f32
	{
		return select(y, x, x > y);
	}
	
	fn fMin(x: f32, y: f32) -> f32
	{
		return select(y, x, x < y);
	}
	
	fn fPow(x: f32, y: f32) -> f32
	{
		let exp1: f32 = y + y - 1.0f;
		let exp2: f32 = pow(10.0f, exp1);
		return pow(x, exp2);
	}

	fn fBell(x: f32, y: f32) -> f32
	{
		let y2: f32 = y * y;
		return pow(4.0f * x * (1.0f - x), 20.0f * y2 * y2 + 0.3f);
	}
	
	fn fWave(x: f32, y: f32) -> f32
	{
		const MAX_FREQUENCY: f32 = 6.0f * 3.1415927f;
		return 0.5f + 0.5f * cos(MAX_FREQUENCY * x * y);
	}
	
	fn fBounce(x: f32, y: f32) -> f32
	{
		const FREQUENCY_FACTOR: f32 = 3.0f * 3.1415927f;
		return abs(cos(FREQUENCY_FACTOR * x * (y + 0.5f)) * exp2(-3.0f * x));
	}

	// -------------------------------------
	// 3 inputs
	
	fn fLerp(x: f32, y: f32, z: f32) -> f32
	{
		return (1.0f - z) * x + z * y;
	}
	
	fn fMlerp(x: f32, y: f32, z: f32) -> f32
	{
		let xMin = select(x, 0.0001f, x < 0.0001f);
		return xMin * pow(y / xMin, z);
	}
	
	fn fClamp(x: f32, y: f32, z: f32) -> f32
	{
		var min: f32 = x;
		var max: f32 = y;
		
		if (x > y)
		{
			min = y;
			max = x;
		}
		if (z < min)
		{
			return min;
		}
		else if (z > max)
		{
			return max;
		}
		return z;
	}
	
	// -------------------------------------
	// 4 inputs
		
	fn fDist(x: f32, y: f32, z: f32, w: f32) -> f32
	{
		let dx: f32 = x - z;
		let dy: f32 = y - w;
		return 0.70710678f * sqrt(dx * dx + dy * dy); // Scale by 1 / sqrt(2)
	}
	
	fn fDistLine(x: f32, y: f32, z: f32, w: f32) -> f32
	{
		if (z < 0.499f)
		{
			let m: f32 = tan(z * 3.1415927f);
			let n: f32 = (1.0f - w) * (1.0f + m) - m;
			let c: f32 = (x + y * m - m * n) / (m * m + 1.0f);
			let dx: f32 = c - x;
			let dy: f32 = m * c + n - y;
			return 0.70710678f * sqrt(dx * dx + dy * dy);
		}
		else if (z > 0.501f)
		{
			let m: f32 = tan(z * 3.1415927f);
			let n: f32 = w - m * w;
			let c: f32 = (x + y * m - m * n) / (m * m + 1.0f);
			let dx: f32 = c - x;
			let dy: f32 = m * c + n - y;
			return 0.70710678f * sqrt(dx * dx + dy * dy);
		}
		else
		{
			return 0.70710678f * abs(w - x);
		}
	}

	// -------------------------------------
	// Masks

	// Implement lerp since WGSL doesn't have it natively
	fn lerp(a: vec3f, b: vec3f, t: vec3f) -> vec3f
	{
		return a + t * (b - a);
	}

	fn fInv3(v: vec3f) -> vec3f
	{
		return vec3f(1.0f, 1.0f, 1.0f) - v;
	}

	fn fAdd3(v: vec3f, x: f32) -> vec3f
	{
		let res: vec3f = v + vec3f(x, x, x);
		return lerp(res, 2.0f - res, step(vec3f(1.0f, 1.0f, 1.0f), res));
	}
	
	fn fSub3(v: vec3f, x: f32) -> vec3f
	{
		let res: vec3f = v - vec3f(x, x, x);
		return lerp(-res, res, step(vec3f(0.0f, 0.0f, 0.0f), res));
	}
	
	)";

	#pragma endregion

	#pragma region Main function

	std::string mainFunction =
	R"(

	struct VertexOutput
	{
		@builtin(position) Position : vec4f,
		@location(0) uv : vec2f
	};

	@vertex
	fn vertexMain(@builtin(vertex_index) i : u32) -> VertexOutput
	{
		// Fullscreen quad
		const positions = array
		(
			vec2f(-1.0f, 1.0f), vec2f(1.0f, 1.0f), vec2f(-1.0f, -1.0f),
			vec2f(-1.0f, -1.0f), vec2f(1.0f, 1.0f), vec2f(1.0f, -1.0f)
		);

		// UV coordinates
		const uvs = array
		(
			vec2f(0.0f, 1.0f), vec2f(1.0f, 1.0f), vec2f(0.0f, 0.0f),
			vec2f(0.0f, 0.0f), vec2f(1.0f, 1.0f), vec2f(1.0f, 0.0f)
		);
		
		// Assemble output
		var output: VertexOutput;
		output.Position = vec4f(positions[i], 0.0f, 1.0f);
		output.uv = uvs[i];
		return output;
	}

	@group(0) @binding(0) var<uniform> buf : vec4f;

	@fragment
	fn fragmentMain(input: VertexOutput) -> @location(0) vec4f
	{
		let invX = 1.0f - input.uv.x;
		let invY = 1.0f - input.uv.y;
		let sinTime = buf.x;
		let cosTime = buf.y;

		let rgb: vec3f = vec3f(&, &, &);
		let rgbMasked = &MASK&;

		return vec4f(rgbMasked, 1.0f);
	}

	)";

	#pragma endregion

	static const char* values[] =
	{
		"input.uv.x", // Normalized x coordinate
		"input.uv.y", // Normalized y coordinate
		"invX", // 1.0f - uv.x
		"invY", // 1.0f - uv.y
#ifdef ANIMATE
		"sinTime", // sin(time)
		"cosTime", // cos(time)
#endif
		"#", // Random constant
		"#" // Double chance
	};
	const int valuesSize = sizeof(values) / sizeof(const char*);
	
	static const char* functions[] =
	{
		"fInv(&)",
		"fSqr(&)",
		"fSqrt(&)",
		"fSmooth(&)",
		"fSharp(&)",
		"fAdd(&, &)",
		"fSub(&, &)",
		"fMul(&, &)",
		"fInv(fMul(&, &))", // Compensate for bias
		"fDiv(&, &)",
		"fAvg(&, &)",
		"fGeom(&, &)",
		"fHarm(&, &)",
		"fHypo(&, &)",
		"fInv(fHypo(&, &))", // Compensate for bias
		"fMax(&, &)",
		"fMin(&, &)",
		"fPow(&, &)",
		"fBell(&, &)",
		"fInv(fBell(&, &))", // Compensate for bias
		"fWave(&, &)",
		"fWave(&, &)", // Double the chance
		//"fBounce(&, &)",
		//"fInv(fBounce(&, &))", // These generate jittery, noisy images
		"fLerp(&, &, &)",
		"fMlerp(&, &, &)",
		//"fClamp(&, &, &)", // This generates ugly discontinuities, keep deactivated
		"fDist(&, &, &, &)",
		"fDist(&, &, #, #)", // Compare variables to fixed point
		"fDist(input.uv.x, input.uv.y, &, &)", // Compare pixel coords to variables
		"fDist(input.uv.x, input.uv.y, #, #)", // Compare pixel coords to fixed point
		"fInv(fDist(&, &, &, &))", // Compensate for bias
		"fInv(fDist(&, &, #, #))", // Compensate for bias
		"fInv(fDist(input.uv.x, input.uv.y, &, &))", // Compensate for bias
		"fInv(fDist(input.uv.x, input.uv.y, #, #))", // Compensate for bias
		"fDistLine(&, &, &, &)",
		"fDistLine(&, &, #, #)", // Compare variables to fixed line
		"fDistLine(input.uv.x, input.uv.y, &, &)", // Compare pixel coords to variable line
		"fDistLine(input.uv.x, input.uv.y, #, #)", // Compare pixel coords to fixed line
		"fInv(fDistLine(&, &, &, &))", // Compensate for bias
		"fInv(fDistLine(&, &, #, #))", // Compensate for bias
		"fInv(fDistLine(input.uv.x, input.uv.y, &, &))", // Compensate for bias
		"fInv(fDistLine(input.uv.x, input.uv.y, #, #))" // Compensate for bias
	};
	const int functionsSize = sizeof(functions) / sizeof(const char*);

	static const char* masks[] =
	{
		"rgb",
		"rgb", // Increase the chance of no mask
		"rgb", // Increase the chance of no mask
		"fAdd3(rgb, &)",
		"fSub3(rgb, &)",
		"fAdd3(fSub3(rgb, &), &)",
		"fSub3(fAdd3(rgb, &), &)",
		"fInv3(fAdd3(rgb, &))",
		"fInv3(fSub3(rgb, &))",
		"fInv3(fAdd3(fSub3(rgb, &), &))",
		"fInv3(fSub3(fAdd3(rgb, &), &))"
	};
	const int masksSize = sizeof(masks) / sizeof(const char*);

	// Replace mask token for one of the masks selected randomly
	std::string maskToken("&MASK&");
	size_t maskPos = mainFunction.find(maskToken);
	const char* mask = rand.Element(masks, masksSize);
	mainFunction.replace(maskPos, maskToken.length(), mask);

	// Run until maxDepth because at maxDepth all tokens must be replaced by constants
	for (int i = 0; i <= maxDepth; i++)
	{
		// Find all '&' tokens and replace for '$' tokens
		// This marks all tokens for replacement in this iteration
		for (char& c : mainFunction)
		{
			if (c == '&')
				c = '$';
		}

		// Replace all '$' tokens for either a function or a value
		size_t pos = mainFunction.find('$');
		while (pos != std::string::npos)
		{
			// Decide whether to replace the token with a function or a fixed value
			// At depth 0, it is guaranteed to use a function, and at MAX_DEPTH it is guaranteed to use a fixed value
			// The progression is quadratic, which makes it more likely to choose functions over values than if the chance progressed linearly
			std::string replacement = rand.IntBetween(1, maxDepth * maxDepth) > i * i
				? rand.Element(functions, functionsSize)
				: rand.Element(values, valuesSize);

			mainFunction.replace(pos, 1, replacement);
			pos = mainFunction.find('$');
		}
	}

	// Replace '#' tokens with random constants
	size_t pos = mainFunction.find('#');
	while (pos != std::string::npos)
	{
		mainFunction.replace(pos, 1, std::to_string(rand.FloatO()) + 'f');
		pos = mainFunction.find('#');
	}

	//std::cout << mainFunction << std::endl;
	//std::cout << "Shader code generated using seed " << seed << std::endl;

	return functionDefinitions + mainFunction;
}

