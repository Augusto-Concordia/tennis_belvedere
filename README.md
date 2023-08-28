![1693239011_tennis_belvedere](https://github.com/Augusto-Concordia/tennis_belvedere/assets/33916325/685bbca7-db12-4fca-bfaa-41ef9dd38b99)


# tennis_belvedere
Quiz #2 - COMP 371

## Features
* Many keybinds to control the camera and the model
* Animated and fun-shaded tennis racket with ball
* Lit model using the simple Phong Lighting model

## Getting Started
### From a zipped folder (TAs ⚠️)
All depended upon libraries should already be included in the vendor folder.

1. Generate project files with CMake
2. Set the working directory to the root of the project
3. Run the `tennis_belvedere` project!

### From the git repository
Similar to the above instructions, with the key difference being to clone the repository with `--recurse-submodules` to include the depended upon libraries.

1. Generate project files with CMake
2. Set the working directory to the root of the project
3. Run the `tennis_belvedere` project!

## Keybinds
* `Home` & `Keypad 5`: Resets the camera's position & rotation
* `Tab`: Resets the current model's position & rotation

<br/>

* `U`: Scale current model up
* `J`: Scale current model down

<br/>

_The following controls' direction were decided relative to the default camera's position, because it was assumed that the user wouldn't orbit the camera much._
* `W`: Move model forward (along the X axis)
* `A`: Move model left (along the Y axis)
* `S`: Move model backward (along the X axis)
* `D`: Move model right (along the Y axis)

<br/>

* `q`: Rotate model forward (around the X axis)
* `e`: Rotate model backward (around the X axis)
* `w`: Rotate model left (around the Y axis)
* `s`: Rotate model right (around the Y axis)
* `a`: Rotate model clockwise (around the Z axis)
* `d`: Rotate model counter-clockwise (around the Z axis)

<br/>

* `Up Arrow + Shift`: Rotate forward current model's upper arm 
* `Down Arrow + Shift`: Rotate backward current model's upper arm
* `Right Arrow + Shift`: Rotate outward current model's upper arm
* `Left Arrow + Shift`: Rotate inward current model's upper arm

<br/>

_The following changes the world orientation (orbits the camera) and only apply when the default camera view is selected._
* `Up Arrow`: Orbit up
* `Right Arrow`: Orbit right
* `Down Arrow`: Orbit down
* `Left Arrow`: Orbit left

<br/>

* `Keypad 7`: Move camera upward (along the Z direction)
* `Keypad 9`: Move camera downward
* `Keypad 8`: Move camera forward (along where it's looking)
* `Keypad 4`: Move camera left
* `Keypad 2`: Move camera backward (along where it's looking)
* `Keypad 6`: Move camera right

<br/>

* `Right Click` & `Horizontal Mouse`: Pan the camera (rotates it horizontally)
* `Middle Click` & `Vertical Mouse`: Tilt the camera (rotates it vertically)
* `Left Click` & `Mouse`: Moves the camera in and out of the scene (along where it's looking)

<br/>

* `M`: Cycle through the camera views
* `R`: Reset camera view to default 
* `T`: Renders the model as filled triangles (i.e. polygons)

<br/>

* `L`: Toggles lights on/off
* `B`: Toggles shadow mapping on/off

## Attributions

[Sky Texture](https://www.freepik.com/free-vector/watercolor-blue-cotton-clouds-background_22379917.htm#query=sky%20texture%20seamless&position=11&from_view=keyword&track=ais)
[Metal Texture](https://www.deviantart.com/hhh316/art/Seamless-metal-texture-182943398)
[Tattoo Texture](https://www.freepik.com/free-vector/vintage-tattoos-monochrome-seamless-pattern_8136380.htm#query=tattoo%20texture&position=1&from_view=keyword&track=ais)
