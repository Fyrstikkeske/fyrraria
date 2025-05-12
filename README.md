to run the program : Zig build run

for small build: zig build small


# TODO
- embed images, currently only does shaders
- Zig based opengl library?
- zig based image loader?

Less likely
zig windowing libraru instead of rgfw?

# technology using
ZM: remember its right handed y up, But row-major. which means we have to use transpose() at every zm to opengl call.
Glad: pretty decent for now
STB: does its job very good
RGFW: Decent