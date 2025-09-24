#version 150

// For modes 1–3
in vec3 position;
in vec4 color;

// For mode 4 (smooth)
in vec3 center;
in vec3 left;
in vec3 right;
in vec3 up;
in vec3 down;

// Uniforms
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform int mode;       // 0 = points/lines/triangles, 1 = smooth
uniform float scale;
uniform float exponent;

// Outputs
out vec4 vColor;

void main()
{
  vec3 pos;
  vec4 col;

  if (mode == 0) {
    // === Normal modes (1, 2, 3) ===
    pos = position;
    col = color;
  } else {
    // === Smooth mode (4) ===
    vec3 avgPos = (center + left + right + up + down) / 5.0;

    float y = avgPos.y;
    float yTransformed = scale * pow(y, exponent);

    pos = vec3(avgPos.x, yTransformed, avgPos.z);

    float c = clamp(pow(y, exponent), 0.0, 1.0);
    col = vec4(c, c, c, 1.0);
  }

  gl_Position = projectionMatrix * modelViewMatrix * vec4(pos, 1.0);
  vColor = col;
}
