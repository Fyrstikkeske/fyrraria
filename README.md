to run the program : make run

if your ide doesnt work, try "make bear".


# TODO

- embed images, currently only embeds shaders
- proper chunking. inwork

## research todo

- make the torus go down instead of up?

# technology using

CGLM: pretty weird, its using a concept of modifying pointers values instead of returning values. Otherwise pretty good
Glad: pretty decent for now
STB: does its job very good
RGFW: Decent

# Done

11/5/25: first commit
13/5/25: rotation 
15/5/25: added basic viewmatrix functionality from learnopengl
20/5/25: Moved back to c, added movement with mouse
21/5-25: donut
22/5-25: proper donut finally(i think), and finally cracked bindless textures
22/5-25: uh, improper chunking? this is how it goes, xyz BUT yeah, x is always the biggest one, the large ring, z is the small ring while y is width. But the way chunking is done right now, its first x then y then z. Which means biggest ring, height, smallest ring. At least i think so.
24-5/25: the height distortions are finally less
26-5/25: added VBO, results:... Oh no. nevermind Trick was to not have one shared vao but a new one for every chunk.