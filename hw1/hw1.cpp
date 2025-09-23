/*
  CSCI 420 Computer Graphics, Computer Science, USC
  Assignment 1: Height Fields with Shaders.
  C/C++ starter code

  Student username: yangchar
*/

#include "openGLHeader.h"
#include "glutHeader.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "pipelineProgram.h"
#include "vbo.h"
#include "vao.h"

#include <iostream>
#include <cstring>
#include <memory>

// additional includes for my new screenshot function
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <csignal>

namespace fs = std::filesystem;

#if defined(WIN32) || defined(_WIN32)
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#if defined(WIN32) || defined(_WIN32)
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper";
#endif

using namespace std;

int mousePos[2]; // x,y screen coordinates of the current mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// Transformations of the terrain.
float terrainRotate[3] = { 0.0f, 0.0f, 0.0f }; 
// terrainRotate[0] gives the rotation around x-axis (in degrees)
// terrainRotate[1] gives the rotation around y-axis (in degrees)
// terrainRotate[2] gives the rotation around z-axis (in degrees)
float terrainTranslate[3] = { 0.0f, 0.0f, 0.0f };
float terrainScale[3] = { 1.0f, 1.0f, 1.0f };

// Width and height of the OpenGL window, in pixels.
int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 Homework 1";


// Number of vertices in the single triangle (starter code).
int numVertices;

// CSCI 420 helper classes.
OpenGLMatrix matrix;
PipelineProgram pipelineProgram;
VBO vboVertices;
VBO vboColors;
VAO vao;

// pipeline for smooth
// PipelineProgram pipelineProgramSmooth;

// additional global variables
int renderMode = 1; // 1=points, 2=lines, 
                    // 3=triangles, 4=smoothing
float scale = 1.0f;
float exponent = 1.0f;

// counters
int numPoints, numLines, numTriangles;
int numTriangleVertices;

// additional VAOs and VBOs
VAO vaoPoints, vaoLines, vaoTriangles;
VBO vboPointsPos, vboPointsColor;
VBO vboLinesPos, vboLinesColor;
VBO vboTrianglesPos, vboTrianglesColor;

VAO vaoSmooth;
VBO vboCenter, vboLeft, vboRight, 
    vboUp, vboDown, vboColor;
// VBO vboSmoothPos, vboSmoothColor, vboSmoothNormal;

int numSmoothVerts;

// gamer control helper
bool keyStates[256] = { false }; // true if key is pressed

// additional global variables for screenshot
string sessionFolder;

string getCurrentTime(const char* format) {
  auto now = chrono::system_clock::now();
  time_t t = chrono::system_clock::to_time_t(now);
  tm lt = *localtime(&t);
  ostringstream oss;
  oss << put_time(&lt, format);

  auto ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;
  oss << '.' << setfill('0') << setw(3) << ms.count();

  return oss.str();
}

string getScreenshotBaseDir() {
  string baseDir = "screenshots";
  if (!fs::exists(baseDir)) {
    fs::create_directory(baseDir);
  }
  return baseDir;
}

// Write a screenshot to the specified filename.
void saveScreenshot() // const char * filename)
{
  // std::unique_ptr<unsigned char[]> screenshotData = std::make_unique<unsigned char[]>(windowWidth * windowHeight * 3);
  // glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData.get());

  // ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData.get());

  // if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
  //   cout << "File " << filename << " saved successfully." << endl;
  // else cout << "Failed to save file " << filename << '.' << endl;

  string timeStamp = getCurrentTime("%H%M%S_");
  string filename = sessionFolder + "/" + timeStamp + ".jpg";

  unique_ptr<unsigned char[]> screenshotData = make_unique<unsigned char[]>(windowWidth * windowHeight * 3);
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData.get());
  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData.get());
  if (screenshotImg.save(filename.c_str(), ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;
}

void idleFunc()
{
  // Do some stuff... 
  // For example, here, you can save the screenshots to disk (to make the animation).
  float moveSpeed = 0.02f; // WASD
  float zoomSpeed = 0.05f; // ZC
  float rotateSpeed = 0.5f; // RF

  if (keyStates['w']) terrainTranslate[1] -= moveSpeed;
  if (keyStates['s']) terrainTranslate[1] += moveSpeed;
  if (keyStates['a']) terrainTranslate[0] += moveSpeed;
  if (keyStates['d']) terrainTranslate[0] -= moveSpeed;

  if (keyStates['z']) terrainTranslate[2] -= zoomSpeed;
  if (keyStates['c']) terrainTranslate[2] += zoomSpeed;

  if (keyStates['r']) terrainRotate[0] += rotateSpeed;
  if (keyStates['f']) terrainRotate[0] -= rotateSpeed;

  if (keyStates['q']) terrainRotate[1] -= rotateSpeed;
  if (keyStates['e']) terrainRotate[1] += rotateSpeed;
  
  // Notify GLUT that it should call displayFunc.
  glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  // When the window has been resized, we need to re-set our projection matrix.
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.LoadIdentity();
  // You need to be careful about setting the zNear and zFar. 
  // Anything closer than zNear, or further than zFar, will be culled.
  const float zNear = 0.1f;
  const float zFar = 10000.0f;
  const float humanFieldOfView = 60.0f;
  matrix.Perspective(humanFieldOfView, 1.0f * w / h, zNear, zFar);
}

void mouseMotionDragFunc(int x, int y)
{
  // Mouse has moved, and one of the mouse buttons is pressed (dragging).

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

  switch (controlState)
  {
    // translate the terrain
    case TRANSLATE:
      if (middleMouseButton)
      {
        // control x,y translation via the left mouse button
        terrainTranslate[0] += mousePosDelta[0] * 0.01f;
        terrainTranslate[1] -= mousePosDelta[1] * 0.01f;
        cout << "[ACTION] Translating" << endl;
      }
      break;

    // rotate the terrain
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        terrainRotate[0] += mousePosDelta[1];
        terrainRotate[1] += mousePosDelta[0];
        cout << "[ACTION] Rotating" << endl;
      }
      break;

    // scale the terrain
    case SCALE:
      if (rightMouseButton) {
        terrainScale[0] += mousePosDelta[1] * 0.01f;
        terrainScale[1] += mousePosDelta[1] * 0.01f;
        terrainScale[2] += mousePosDelta[1] * 0.01f;
        cout << "[ACTION] Scaling" << endl;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // Mouse has moved.
  // Store the new mouse position.
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // A mouse button has been pressed or released.

  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
      cout << "[INPUT] LEFT BUTTON" << endl;
      controlState = ROTATE;
      break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
      cout << "[INPUT] MIDDLE BUTTON" << endl;
      controlState = TRANSLATE;
      break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
      cout << "[INPUT] RIGHT BUTTON" << endl;
      controlState = SCALE;
      break;

    // case 3: // mouse wheel up
    //   terrainTranslate[2] -= 0.1f; // zoom in
    //   cout << "[INPUT] MOUSE WHEEL UP" << endl;
    //   break;

    // case 4: // mouse wheel down
    //   terrainTranslate[2] += 0.1f; // zoom out
    //   cout << "[INPUT] MOUSE WHEEL DOWN" << endl;
    //   break;
  }

  // Store the new mouse position for drag calculations.
  mousePos[0] = x;
  mousePos[1] = y;
}

// removed old keyboard function

void keyboardDownFunc(unsigned char key, int x, int y)
{
  keyStates[key] = true;

  if (key == 27) exit(0); // key esc
  if (key == 'x') {
    saveScreenshot();
    cout << "[SCREENSHOT] at time " 
         << getCurrentTime("%H%M%S_") 
         << endl;
  }

  // display mode switching
  if (key == '1') {
    renderMode = 1;
    cout << "[RENDER MODE] " << key << endl;
  }; // points
  if (key == '2') {
    renderMode = 2;
    cout << "[RENDER MODE] " << key << endl;
  } // lines
  if (key == '3') {
    renderMode = 3;
    cout << "[RENDER MODE] " << key << endl;
  } // triangles
  if (key == '4') {
    renderMode = 4;
    cout << "[RENDER MODE] " << key << endl;
  } // smoothing

  // reset everything
  if (key == 't') {
    terrainRotate[0] = 0.0f;
    terrainRotate[1] = 0.0f;
    terrainRotate[2] = 0.0f;

    terrainTranslate[0] = -0.5f;
    terrainTranslate[1] = 0.0f;
    terrainTranslate[2] = -0.5f;

    terrainScale[0] = 5.0f;
    terrainScale[1] = 5.0f;
    terrainScale[2] = 5.0f;

    cout << "[DISPLAY] Reset" << endl;
  }
}

void keyboardUpFunc(unsigned char key, int x, int y)
{
  keyStates[key] = false;
}

void buildPoints(ImageIO *heightmapImage, int width, int height, float heightScale)
{
  numPoints = width * height;

  vector<float> positions;
  vector<float> colors;
  positions.reserve(numPoints * 3);
  colors.reserve(numPoints * 4);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      float h = heightmapImage->getPixel(j, i, 0);

      positions.push_back((float)i / height - 0.5f);
      positions.push_back(h * heightScale);
      positions.push_back(-(float)j / width - 0.5f);

      float gray = h / 255.0f;
      colors.insert(colors.end(), { gray, gray, gray, 1.0f });
    }
  }

  vboPointsPos.Gen(numPoints, 3, positions.data(), GL_STATIC_DRAW);
  vboPointsColor.Gen(numPoints, 4, colors.data(), GL_STATIC_DRAW);
  vaoPoints.Gen();
  vaoPoints.Bind();
  vaoPoints.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboPointsPos, "position");
  vaoPoints.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboPointsColor, "color");
}

void buildLines(ImageIO *heightmapImage, int width, int height, float heightScale)
{
  vector<float> positions;
  vector<float> colors;

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      float h = heightmapImage->getPixel(j, i, 0);
      float y = h * heightScale;
      float x = (float)i / height - 0.5f;
      float z = -(float)j / width - 0.5f;
      float gray = h / 255.0f;

      if (j + 1 < width) {
        float h2 = heightmapImage->getPixel(j + 1, i, 0);
        float y2 = h2 * heightScale;
        float z2 = -(float)(j + 1) / width - 0.5f;

        positions.insert(positions.end(), { x, y, z, x, y2, z2 });
        colors.insert(colors.end(), { gray, gray, gray, 1.0f,
                                      h2/255.0f, h2/255.0f, h2/255.0f, 1.0f });
      }

      if (i + 1 < height) {
        float h2 = heightmapImage->getPixel(j, i + 1, 0);
        float y2 = h2 * heightScale;
        float x2 = (float)(i + 1) / height - 0.5f;

        positions.insert(positions.end(), { x, y, z, x2, y2, z });
        colors.insert(colors.end(), { gray, gray, gray, 1.0f,
                                      h2/255.0f, h2/255.0f, h2/255.0f, 1.0f });
      }
    }
  }

  numLines = positions.size() / 3;

  vboLinesPos.Gen(numLines, 3, positions.data(), GL_STATIC_DRAW);
  vboLinesColor.Gen(numLines, 4, colors.data(), GL_STATIC_DRAW);
  vaoLines.Gen();
  vaoLines.Bind();
  vaoLines.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboLinesPos, "position");
  vaoLines.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboLinesColor, "color");
}

void buildTriangles(ImageIO *heightmapImage, int width, int height, float heightScale)
{
  vector<float> positions;
  vector<float> colors;

  for (int i = 0; i < height - 1; i++) {
    for (int j = 0; j < width - 1; j++) {
      float h00 = heightmapImage->getPixel(j, i, 0);
      float h01 = heightmapImage->getPixel(j + 1, i, 0);
      float h10 = heightmapImage->getPixel(j, i + 1, 0);
      float h11 = heightmapImage->getPixel(j + 1, i + 1, 0);

      float x00 = (float)i / height - 0.5f, y00 = h00 * heightScale, z00 = -(float)j / width - 0.5f;
      float x01 = (float)i / height - 0.5f, y01 = h01 * heightScale, z01 = -(float)(j + 1) / width - 0.5f;
      float x10 = (float)(i + 1) / height - 0.5f, y10 = h10 * heightScale, z10 = -(float)j / width - 0.5f;
      float x11 = (float)(i + 1) / height - 0.5f, y11 = h11 * heightScale, z11 = -(float)(j + 1) / width - 0.5f;

      float g00 = h00 / 255.0f, g01 = h01 / 255.0f, g10 = h10 / 255.0f, g11 = h11 / 255.0f;

      // triangle 1
      positions.insert(positions.end(), { x00,y00,z00, x10,y10,z10, x01,y01,z01 });
      colors.insert(colors.end(), { g00,g00,g00,1.0f, g10,g10,g10,1.0f, g01,g01,g01,1.0f });

      // triangle 2
      positions.insert(positions.end(), { x10,y10,z10, x11,y11,z11, x01,y01,z01 });
      colors.insert(colors.end(), { g10,g10,g10,1.0f, g11,g11,g11,1.0f, g01,g01,g01,1.0f });
    }
  }

  numTriangleVertices = positions.size() / 3;

  vboTrianglesPos.Gen(numTriangleVertices, 3, positions.data(), GL_STATIC_DRAW);
  vboTrianglesColor.Gen(numTriangleVertices, 4, colors.data(), GL_STATIC_DRAW);
  vaoTriangles.Gen();
  vaoTriangles.Bind();
  vaoTriangles.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboTrianglesPos, "position");
  vaoTriangles.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboTrianglesColor, "color");
}

void buildSmoothSurface(ImageIO *image, int width, int height, float heightScale)
{
  vector<float> centers, lefts, rights, ups, downs, colors;

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      float h = image->getPixel(j, i, 0);
      float y = h * heightScale;
      float x = (float)i / height - 0.5f;
      float z = -(float)j / width - 0.5f;
      float gray = h / 255.0f;

      // Current vertex
      centers.insert(centers.end(), { x, y, z });
      colors.insert(colors.end(), { gray, gray, gray, 1.0f });

      // Neighbor function
      auto neighbor = [&](int ni, int nj) {
        ni = std::max(0, std::min(height - 1, ni));
        nj = std::max(0, std::min(width - 1, nj));
        float nh = image->getPixel(nj, ni, 0);
        return vector<float>{
          (float)ni / height - 0.5f,
          nh * heightScale,
          -(float)nj / width - 0.5f
        };
      };

      auto L = neighbor(i, j - 1);
      auto R = neighbor(i, j + 1);
      auto U = neighbor(i - 1, j);
      auto D = neighbor(i + 1, j);

      lefts.insert(lefts.end(), L.begin(), L.end());
      rights.insert(rights.end(), R.begin(), R.end());
      ups.insert(ups.end(), U.begin(), U.end());
      downs.insert(downs.end(), D.begin(), D.end());
    }
  }

  numSmoothVerts = centers.size() / 3;

  // Upload to GPU
  vboCenter.Gen(numSmoothVerts, 3, centers.data(), GL_STATIC_DRAW);
  vboLeft.Gen(numSmoothVerts, 3, lefts.data(), GL_STATIC_DRAW);
  vboRight.Gen(numSmoothVerts, 3, rights.data(), GL_STATIC_DRAW);
  vboUp.Gen(numSmoothVerts, 3, ups.data(), GL_STATIC_DRAW);
  vboDown.Gen(numSmoothVerts, 3, downs.data(), GL_STATIC_DRAW);
  vboColor.Gen(numSmoothVerts, 4, colors.data(), GL_STATIC_DRAW);

  vaoSmooth.Gen();
  vaoSmooth.Bind();
  vaoSmooth.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboCenter, "center");
  vaoSmooth.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboLeft, "left");
  vaoSmooth.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboRight, "right");
  vaoSmooth.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboUp, "up");
  vaoSmooth.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboDown, "down");
  vaoSmooth.ConnectPipelineProgramAndVBOAndShaderVariable(&pipelineProgram, &vboColor, "color");
}

void displayFunc()
{
  // This function performs the actual rendering.

  // First, clear the screen.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set up the camera position, focus point, and the up vector.
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.LoadIdentity();
  matrix.LookAt(0.0, 0.5, 2.0,
                0.0, 0.0, 0.0,
                0.0, 1.0, 0.0);

  // In here, you can do additional modeling on the object, such as performing translations, rotations and scales.
  // ...

  matrix.Translate(terrainTranslate[0], terrainTranslate[1], terrainTranslate[2]);
  matrix.Rotate(terrainRotate[0], 1.0f, 0.0f, 0.0f);
  matrix.Rotate(terrainRotate[1], 0.0f, 1.0f, 0.0f);
  matrix.Rotate(terrainRotate[2], 0.0f, 0.0f, 1.0f);
  matrix.Scale(terrainScale[0], terrainScale[1], terrainScale[2]);

  // Read the current modelview and projection matrices from our helper class.
  // The matrices are only read here; nothing is actually communicated to OpenGL yet.
  float modelViewMatrix[16];
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.GetMatrix(modelViewMatrix);

  float projectionMatrix[16];
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.GetMatrix(projectionMatrix);

  // Upload the modelview and projection matrices to the GPU. Note that these are "uniform" variables.
  // Important: these matrices must be uploaded to *all* pipeline programs used.
  // In hw1, there is only one pipeline program, but in hw2 there will be several of them.
  // In such a case, you must separately upload to *each* pipeline program.
  // Important: do not make a typo in the variable name below; otherwise, the program will malfunction.
  pipelineProgram.SetUniformVariableMatrix4fv("modelViewMatrix", GL_FALSE, modelViewMatrix);
  pipelineProgram.SetUniformVariableMatrix4fv("projectionMatrix", GL_FALSE, projectionMatrix);

  // deleted original code

  if (renderMode == 1) {
    vaoPoints.Bind();
    glDrawArrays(GL_POINTS, 0, numPoints);
  } else if (renderMode == 2) {
    vaoLines.Bind();
    glDrawArrays(GL_LINES, 0, numLines);
  } else if (renderMode == 3) {
    vaoTriangles.Bind();
    glDrawArrays(GL_TRIANGLES, 0, numTriangleVertices);
  } else if (renderMode == 4) {
    vaoSmooth.Bind();
    glDrawArrays(GL_TRIANGLES, 0, numSmoothVerts);
  }

  // Swap the double-buffers.
  glutSwapBuffers();
}

void initScene(int argc, char *argv[])
{
  // Load the image from a jpeg disk file into main memory.
  std::unique_ptr<ImageIO> heightmapImage = std::make_unique<ImageIO>();
  if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
  {
    cout << "Error reading image " << argv[1] << "." << endl;
    exit(EXIT_FAILURE);
  }

  // Set the background color.
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black color.

  // Enable z-buffering (i.e., hidden surface removal using the z-buffer algorithm).
  glEnable(GL_DEPTH_TEST);

  // Create a pipeline program. This operation must be performed BEFORE we initialize any VAOs.
  // A pipeline program contains our shaders. Different pipeline programs may contain different shaders.
  // In this homework, we only have one set of shaders, and therefore, there is only one pipeline program.
  // In hw2, we will need to shade different objects with different shaders, and therefore, we will have
  // several pipeline programs (e.g., one for the rails, one for the ground/sky, etc.).
  // Load and set up the pipeline program, including its shaders.
  if (pipelineProgram.BuildShadersFromFiles(shaderBasePath, "vertexShader.glsl", "fragmentShader.glsl") != 0)
  {
    cout << "Failed to build the pipeline program." << endl;
    throw 1;
  } 
  cout << "Successfully built the pipeline program." << endl;
  
  // if (pipelineProgramSmooth.BuildShadersFromFiles(shaderBasePath, "vertexShader_smooth.glsl", "fragmentShader_smooth.glsl") != 0)
  // {
  //   cout << "Failed to build the smooth-shading pipeline program." << endl;
  //   throw 1;
  // }
  // cout << "Successfully built the smooth-shading pipeline program." << endl;

  // Bind the pipeline program that we just created. 
  // The purpose of binding a pipeline program is to activate the shaders that it contains, i.e.,
  // any object rendered from that point on, will use those shaders.
  // When the application starts, no pipeline program is bound, which means that rendering is not set up.
  // So, at some point (such as below), we need to bind a pipeline program.
  // From that point on, exactly one pipeline program is bound at any moment of time.
  pipelineProgram.Bind();

  // deleted hard-coded triangle

  // new code
  int width = heightmapImage->getWidth();
  int height = heightmapImage->getHeight();
  float heightScale = 0.5f / 255.0f;

  buildPoints(heightmapImage.get(), width, height, heightScale);
  buildLines(heightmapImage.get(), width, height, heightScale);
  buildTriangles(heightmapImage.get(), width, height, heightScale);
  buildSmoothSurface(heightmapImage.get(), width, height, heightScale);

  // default scene
  terrainTranslate[0] = -0.5f;
  terrainTranslate[2] = -0.5f;
  terrainScale[0] = 5.0f;
  terrainScale[1] = 5.0f;
  terrainScale[2] = 5.0f;

  glPointSize(2.0f); // make the points easier to see

  // Check for any OpenGL errors.
  std::cout << "GL error status is: " << glGetError() << std::endl;
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Initializing GLUT..." << endl;
  glutInit(&argc,argv);

  cout << "Initializing OpenGL..." << endl;

  #ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #endif

  // screenshot folder

  string baseDir = "screenshots";
  fs::create_directories(baseDir);
  string runStamp = getCurrentTime("%Y%m%d_%H%M%S");
  string pictureName = string(argv[1]);
  size_t pos = pictureName.find_last_of("/\\");
  if (pos != string::npos) pictureName = pictureName.substr(pos + 1);

  sessionFolder = baseDir + "/RUN_" + runStamp + "_" + pictureName;
  fs::create_directories(sessionFolder);

  cout << "Session screenshots at: " << sessionFolder << endl;

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);  
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  #ifdef __APPLE__
    // This is needed on recent Mac OS X versions to correctly display the window.
    glutReshapeWindow(windowWidth - 1, windowHeight - 1);
  #endif

  // Tells GLUT to use a particular display function to redraw.
  glutDisplayFunc(displayFunc);
  // Perform animation inside idleFunc.
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  // glutKeyboardFunc(keyboardFunc);
  glutKeyboardFunc(keyboardDownFunc);
  glutKeyboardUpFunc(keyboardUpFunc);

  // init glew
  #ifdef __APPLE__
    // nothing is needed on Apple
  #else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
      cout << "error: " << glewGetErrorString(result) << endl;
      exit(EXIT_FAILURE);
    }
  #endif

  // Perform the initialization.
  initScene(argc, argv);

  // Sink forever into the GLUT loop.
  glutMainLoop();
}