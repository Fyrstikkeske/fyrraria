to run the program : make run

if your ide doesnt work, try "make bear".

# Bugs to fix
If the render distance is larger than world x or z, it will cause chunks to dissapear on movement.

# TODO

- embed images, currently only embeds shaders

- make the binary mesher handle cases outside of itself. Regenerate chunks next to a chunk addition and deletion (somewhat done now)

- LODS

## research todo

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

27-5/25: completed the VBO and added textures to it. Theres a couple of magic numbers in the vertex shader i have to figure out. last minute addition: Backface culling 

28/5-25: uh, dont really know but for large worlds the height algorythm feels better, BUT its aspect ratio varies with an increase of height now. which the previus did not suffer from. now, i have no idea on how to get the best of both worlds. constant aspect ratio of EXP method and the stability of linear method. There is a method out there, i dont know it.

29/5-25: mouse and coordingates are now local to the planet, nextsetp would prob be a refactor cuz holy hell its ugly

30/5-25: Day 1 of adding proper chunking. So technically its day 2, But i havent slept yet so it doesnt count. Regardless added uthash for that legendary O(1) in chunking logic. So now its stable 60 which is max due to opengl saying so. Which is super noice. No more lag(its still laggy with big render distances)

3/6-25: YAAAAA, OKOKOk, things i did last couple of days. Made the worldgen know which chunks to update and remove without iterating through the whole thing. Secondly, TANTANS BINARY FACE CULLING. thanks to theese things i dont even need to multithread world gen until like 10-15 render distance. (its still smart though thanks to perlin noise)

19/6-25: yo im back biaches, finally made the checkrenderablechunksthingy allow for a not thingy within. basically, LODS ARE SOON READY(I hope.plsplsplspls) i dont want to fucking do lod anymore and just optimise the gpu now, it would be so nice. there are some schizo ideas(vertex pulling) basically 32 or maybe even 16 bits per face. addendum. CHUNKS DONT RENDER NEARBOR CHUNK PARTS IF NOT NEEDED!!! like 1.5x fps somehow

9/7-25: oh yea, i was supposed to write here. troll. Regardless, summary of what happened. Fixed the opposite side chunk not rendering, changed to new hashmap which is betta. maybe basic Lod setup. just a thought