#version 330 core

in vec3 position;
in vec4 color;

// Smooth mode attributes (only used if mode == 1)
in vec3 center;
in vec3 left;
in vec3 right;
in vec3 up;
in vec3 down;

// Uniforms
uniform int mode;        // 0 = normal, 1 = smooth
uniform float scale;
uniform float exponent;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

out vec4 vColor;

void main() {
  vec3 pos;
  vec4 outColor;

  if (mode == 0) {
    // Modes 1, 2, 3
    pos = position;
    outColor = color;
  } else {
    // Mode 4: smooth
    vec3 avg = (center + left + right + up + down) / 5.0;

    float y_mod = scale * pow(avg.y, exponent);
    pos = vec3(avg.x, y_mod, avg.z);

    float g = pow(avg.y, exponent);
    outColor = vec4(g, g, g, 1.0);
  }

  gl_Position = projectionMatrix * modelViewMatrix * vec4(pos, 1.0);
  vColor = outColor;
}
