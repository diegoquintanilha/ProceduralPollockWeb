# ProceduralPollockWeb

ProceduralPollockWeb is the WebGPU version of ProceduralPollock, an open source C++ project that generates abstract, math-based art using procedurally generated shaders.

The program creates an HTML canvas and a WebGPU context, then uses a 64-bit seed (usually from the system time, but can be set manually) to procedurally generate a WGSL fragment shader that is then displayed on the browser window canvas. The generation process is deterministic, i.e. the same seed will always generate the same shader.

The generated images look like abstract expressionistic art, somewhat resembling to the style of Jackson Pollock, hence the name of the project.

You can view the project's output directly on the GitHub Pages link <https://diegoquintanilha.github.io/ProceduralPollockWeb/>. No need to build or download anything. Simply reload the page to generate a new shader.

This project uses a custom PRNG (see `src/RandFS.h`) to procedurally generate a function that receives as input the X and Y coordinates of each pixel, along with the time, and outputs an RGB value for that pixel. From the given seed, it uses different techniques to compose primitive mathematical formulas and generate a single function. This function is then incorporated into a WGSL fragment shader, compiled into WebAssembly and rendered on the browser window.

This technique was originally proposed in the paper [Hash Visualization: a New Technique to improve Real-World Security](https://users.ece.cmu.edu/~adrian/projects/validation/validation.pdf) by Adrian Perrig and Dawn Song. This project is just one possible implementation of the general idea outlined in the paper.

By default, the program uses time as an input to generate animated images. The time value always pass through sine and cosine functions, making the animation loop perfectly. To generate only static images (no animation), open `src/Shader.cpp` and comment out the line `#define ANIMATE`.

Also, please check out other versions of this project:

- [ProceduralPollock](https://github.com/diegoquintanilha/ProceduralPollock): standalone window version implemented with DirectX 11
- PerpetualPollock: infinite stream of animated abstract images, never repeating (Coming soon!)
- PerpetualPollockWeb: browser version of PerpetualPollock (Coming soon!)

## Build

This project requires [Emscripten](https://emscripten.org/) to build. Please follow the instructions from their website to download and setup their compiler. Make sure to have `emcc` in your `PATH` environment variable.

Run the following command from the root folder to compile all C++ code and generate the .js and the .wasm files:

```
emcc src/main.cpp src/Shader.cpp src/Graphics.cpp -o main.js -s USE_WEBGPU=1 -s ALLOW_MEMORY_GROWTH=1
```

Emscripten provides a quick and easy way to run a local web server for testing. To start a server and open the project in your browser, use the following command:

```
emrun --port 8080 .
```

Please note that you can view the project's output instantly, no need to build or download anything. Just visit the GitHub Pages link <https://diegoquintanilha.github.io/ProceduralPollockWeb/>.

Support for CMake will be added in the future.

## Future features

Features to be implemented:

- Button to generate new shader
- Custom seed input
- Separate buttons for generating animated and static images
- Exporting animated images as video files and static images as PNG files

Feel free to contribute or suggest any new features!

Questions and feedback are welcome. Please send them to my email [diego.quintanilha@hotmail.com](mailto:diego.quintanilha@hotmail.com).

