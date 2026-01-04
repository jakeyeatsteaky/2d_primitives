```bash
mkdir build
cd build 
cmake ..
make
```

# TODO
- [ ] 

# Understanding Problems
## Delta Time, Pixel coordinates and world units
My process for rendering a circle is as follows:
- Instantiate the location via mouse click
- Populate a buffer with the coordinates of a quarter arc:
    from the center, encode the number of samples
    each step becomes (pi/2)/numSamples
    At each angle, derive the x and y values using Sin and Cos
- On instantiation, the circle is added to a queue, and in the update
    loop, if there are items in the queue pop the circle, then use the 
    the arc to add all the point values of the rendered circle to a global
    buffer (shit system honestly. cant fuckin keep track of shit)

The system has its flaws because circles are instantiated, then lost.  I have no implemented way to keep track of where that circle's buffer values are in the global buffer.

Regardless, the main issue as it pertains to movement and animation is that each circle uses Pixel coordinates to reference it's location (via mouse click).

And when multiplying by delta time, the fractional portion often rounds out 
being zero

What I need to do is to establish some kind of world unit and world space. Such that
a positional change in my world can be done in fractional quantities, and then it can be transitioned to pixel space after update and before rendering.

## Better way of instantiating and rendering objects
Right now circle objects are not at all tied to their render data (circles are drawn with SDL_GetPoints which takes a buffer of SDL_Points)

What I definitely want to have is to be able to click and spawn a circle
so on click, I need to retain the initial coordinates of that click

From there, the circle will change position and thus that position data needs to be saved somewhere and associated with a circle

In addition to position data of that circle, there is render data associated with that position since my render is dependent on the center position 

So it would be intuitive to create a new Circle, where a circle object has a member array of point data to describe how its rendered.  

but is it not too costly to:
1) generate new position data on every update
2) overwrite the old render data based on the new position data
Each game tick would look like this:
spawn --> populate arc --> populate render data --> render --> update --> repeat

One thing to think about is this idea of scanlines
first off, you don't need to recomputer GEOMETRY. the trig functions, and produce 
quarter arc is computing geometry, but these circles arent changing.

A scanline is essentially: for any y value, what is the x RANGE of what the circle
would entail?
iterat over the loop for y =  -r --> r 

