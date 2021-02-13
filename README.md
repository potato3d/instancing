# instancing
Source code for the article [Instanced Rendering of Massive CAD Models Using Shape Matching](https://ieeexplore.ieee.org/document/6915326)

We present an efficient approach to greatly reduce memory usage and to achieve high rendering performance for massive 3D CAD models. Our work is based on the observation that these models often contain many instances of the same geometries. Using a shape matching algorithm, we identify these instances and estimate the optimal affine transformation between them. The resulting sets of unique geometries only require a fraction of the memory used by the original 3D model. This compact representation can be rendered efficiently using hardware-accelerated geometry instancing. As another benefit, this allows for interactive rendering of highly dynamic scenes of up to a million moving objects. Our proposal assumes no knowledge about the scene, and thus can be applied to other kinds of massive 3D models with similar properties.

# Description

The main tecnique is implemented in app/duplicate_instance_renderer.*

Given two geometries with the same number of vertices in the same order, the algorithm approximates an affine transformation between them using least squares. If the transformation result is within an error threshold, we consider the second geometry to be an instance (a duplicate) of the first geometry. We do this for every pair of potentially duplicate geometries in the scene. The sets of transformations are stored within the GPU using Shader Storage Buffer Objects (SSBO), occupying several orders of magnitude less memory than if we were to store all vertices for each duplicate geometry.

For real-time rendering, we use OpenGL's geometry instancing API together with a vertex shader that accesses the current instance's transformation in the SSBO. This significantly reduces API call overhead and moves the bottleneck entirely to the GPU. We can render massive models that would otherwise not fit inside the GPU with faster performance than other approaches.

# Results

Here are some rendered images:

![fig1](https://github.com/potato3d/instancing/blob/main/imgs/fig1.png "example")

Here are some performance and memory results:

![fig3](https://github.com/potato3d/instancing/blob/main/imgs/fig3.png "speed static")
![fig4](https://github.com/potato3d/instancing/blob/main/imgs/fig4.png "speed dynamic")
![fig2](https://github.com/potato3d/instancing/blob/main/imgs/fig2.png "memory")
