# Intro to DirectX Raytracing

This repository is the code for the three part article series called 'Intro to DirectX Raytracing' by Niels Jaspers. This repository aims to make the step towards DirectX Raytracing easy for the beginner. It tries to show how to use the API and how you can utilize the most from the API.

Have a look at the samples, to get familiar with the framework and what is possible with the API. Anything from a simple triangle, to basic lighting (wip).

If you want to read my posts, have a look at the articles:
- [Part 1 (the concepts)](https://pappaniels.github.io/posts/intro-dxr/part-1) (wip)
- [Part 2 (the implementation)]() (wip)
- [Part 3 (debugging the ray tracer)]() (wip)

## Building

### Dependencies
- A Windows machine
- Visual Studio 2022:
    - Desktop Development with C++ extension:
        - Windows SDK 10.0.22621.0 or higher
        - HLSL Tools (?)
- A GPU with D3D12 support, shader model 6.6 support, and ray tracing capabilities.
- Git

### Downloading the code
To download the code, clone the repo by using `git clone https://github.com/PappaNiels/IntroDXR.git`. Then to get the submodules, use `git submodule update --init`, to make sure you have all the dependencies.

### Building the code
To build the code, open the visual studio solution. Select the sample you want to view, by setting the start project to the sample project. Press `F5` to build and run with the debugger attached, or press `ctrl + b` on the sample project to just build the executable. No further steps are needed to run the sample.

## How to use
Soon!

### Command line arguments
- `-validation` : Enables the validation layers.
- `-console` : Opens a console window as a logger.
- `-warp` : Use Microsoft's software renderer for the rendering, rather than the dedicated GPU. This would be useful to ensure that the DirectX API gets used properly, and use features that are not available for your GPU. 

## License
This codebase that can be found under [`code/`](https://github.com/PappaNiels/IntroDXR/tree/main/code) falls under the MIT license as seen in [LICENSE](https://github.com/PappaNiels/IntroDXR/blob/main/code/LICENSE). The code in [`vendor/`](https://github.com/PappaNiels/IntroDXR/tree/main/vendor) falls under its own license respectively.

## Contributing
If there are bugs, ways to optimize the code, or make it a better structure to make the library easier to use. Feel free to contribute to the repository. 

## Contact
If there are any concerns or questions, reach out to me via my contact details or discord (pappaniels).