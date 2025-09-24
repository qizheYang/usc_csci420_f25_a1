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

// implement JetColorMap
vec3 JetColorMap(float x)
{
  float r, g, b;
  float a;

  if (x < 0.0) {
    return vec3(0.0, 0.0, 0.0);
  }
  else if (x < 0.125) {
    a = x / 0.125;
    r = 0.0;
    g = 0.0;
    b = 0.5 + 0.5 * a;
  }
  else if (x < 0.375) {
    a = (x - 0.125) / 0.25;
    r = 0.0;
    g = a;
    b = 1.0;
  }
  else if (x < 0.625) {
    a = (x - 0.375) / 0.25;
    r = a;
    g = 1.0;
    b = 1.0 - a;
  }
  else if (x < 0.875) {
    a = (x - 0.625) / 0.25;
    r = 1.0;
    g = 1.0 - a;
    b = 0.0;
  }
  else if (x <= 1.0) {
    a = (x - 0.875) / 0.125;
    r = 1.0 - 0.5 * a;
    g = 0.0;
    b = 0.0;
  }
  else {
    r = 1.0;
    g = 1.0;
    b = 1.0;
  }

  return vec3(r, g, b);
}

void main()
{
  vec3 pos;
  vec4 col;

  if (mode == 0) {
    pos = position;
    col = color;

    gl_Position = projectionMatrix * modelViewMatrix * vec4(pos, 1.0);
    vColor = col;
  } else {
    vec3 avgPos = (center + left + right + up + down) / 5.0;

    float y = avgPos.y;
    float gray = clamp(y, 0.0, 1.0);

    float adjustedY = scale * pow(y, exponent);

    gl_Position = projectionMatrix * modelViewMatrix * vec4(avgPos.x, adjustedY, avgPos.z, 1.0);

    vec3 jet = JetColorMap(gray);
    vColor = vec4(jet, 1.0);
  }
}
