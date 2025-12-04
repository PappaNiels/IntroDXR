# Raytraced shadows

This sample demonstrates how you can cast shadow rays in an optimal way. It uses a directional light that shades a cube and the cube casts a shadow on the platform.

<figure>
    <img src="../../data/images/Shadows_Result.png"
         alt="Shadows result"
         >
    <figcaption><i>The result of the simple shading of this sample with a shadow casted on a platform.</i></figcaption>
</figure>

### Controls
To see the full cube, you can move the camera around with the following controls:
- `W` : Go forwards.
- `A` : Go left.
- `S` : Go backwards.
- `D` : Go right.

- `Q` : Go up.
- `E` : Go down.

- `Up arrow` : Rotate up.
- `Left arrow` : Rotate left.
- `Down arrow` : Rotate down.
- `Right arrow` : Rotate right.

- `Esc` : Quit sample.

### What can be learned from this sample?
- Simple shading in the ray tracer.
- Getting vertex data in the closest hit shader.
- Casting a shadow ray.

### Notes
- This ray tracer uses a right-handed coordinate system, where the z coordinate is up.
- PIX events are missing.