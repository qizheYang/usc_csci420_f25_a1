# Core Functions Implemented

Mode 1, 2, 3 for vertices (points), lines and triangles.

Mode 4 for smoothened with keyboard input to change scale/exponent.

Mouse input to rotate, translate and scale.

Grayscale coloring.

# Additional Features

Keyboard input to manipulate: WASD to move around, QE to turn left and right, RF to see up and down, and ZC to zoom in and out. This is inspired by Cities: Skylines.

Jet Color Map for smoothened version.

GL Draw Elements for triangles.

Inspired by the `earthquake` thing in class and the audio responsive wallpapers on Wallpaper Engine, made a random changing terrain thing that randomly changes the height of any pixel within a safe range (calculated from its original and neighbors).

# Animation

A python script is included in animation_helper folder and running it automatically converts the screenshots of latest run into an animation of 15 fps for 20 seconds. Please go to ```~/usc_csci420_f25_a1/animation_helper/animation_ver_i``` where i is the biggest one (obviously sorted to the last) to see. The animation file is called animation.mp4.

# For Grading

## To view all run results

Please see screenshots folder in /hw1.

## To view animation results

Please see ../animation_helper/runs.

## Other requirements satisfied

### Handle at least a 256x256 image at interactive frame rates. 

It can handle up to 768x768 at least. Tested with 800x800 and failed.

### Render points, lines, triangles

As can be seen from running or animation.

### Render smoothened

Same as above.

### Render linearly proportional to height

Higher is brighter, lower is dimmer.

### Control with Mouse

`GLUT_ACTIVE_CTRL` and `GLUT_ACTIVE_ALT` does not work on my computer, so I actually deleted these two functions and instead control rotate, translate, and scale with left button, middle button, and right button.

### Random Height

Unfortunately, this changing algorithm is a high time complexity, meaning it might run slower than designed (0.01s / frame) if the input is big. It runs smoothly for the spiral.jpg in my testing.

### Coding

Unfortunately, the code is a bit messed because functions are implemented one by one. Certain global variables are added later for newer functions that could sometimes replace original variables in function scope.