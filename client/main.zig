const std = @import("std");

const RGFW = @cImport({
    @cInclude("RGFW.h");
});

const glad = @cImport({
    @cInclude("gl.h");
});

const stbi = @cImport({
    @cInclude("stb_image.h");
});

const GodStruct = struct { render: Render };

const Render = struct {
    win: ([*c]RGFW.struct_RGFW_window),
    fps: u32,
};

const vertexshader = @embedFile("shaders/cube.vert");
const fragmentshader = @embedFile("shaders/cube.frag");

const vertices = [_]f32{
    // positions          // colors           // texture coords
     0.5,  0.5, 0.0,   1.0, 0.0, 0.0,   1.0, 1.0,   // top right
     0.5, -0.5, 0.0,   0.0, 1.0, 0.0,   1.0, 0.0,   // bottom right
    -0.5, -0.5, 0.0,   0.0, 0.0, 1.0,   0.0, 0.0,   // bottom left
    -0.5,  0.5, 0.0,   1.0, 1.0, 0.0,   0.0, 1.0,    // top left 
};

const indices = [_]u32{  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};

fn keyfunc(win: [*c]RGFW.RGFW_window, key: u8, keyChar: u8 , keyMod: u8 , pressed: u8) callconv(.C) void{
    _ = keyChar;
    _ = keyMod;
    if (key == RGFW.RGFW_escape and pressed != 0) {
        RGFW.RGFW_window_setShouldClose(win, RGFW.True);
    }
}


pub fn main() !void {
    var frames: u32 = 0;
    var startTime: i64 = std.time.microTimestamp();

    RGFW.RGFW_setGLHint(RGFW.RGFW_glMajor, 3);
    RGFW.RGFW_setGLHint(RGFW.RGFW_glMinor, 3);

    const win = RGFW.RGFW_createWindow("Fyrraria", RGFW.RGFW_RECT(100, 100, 800, 600), RGFW.RGFW_windowCenter);
    if (win == null) {
        std.debug.print("Failed to create window\n", .{});
        return;
    }
    var godStruct = GodStruct{ .render = .{ .win = win, .fps = 0 } };
    _ = glad.gladLoadGL(@as(glad.GLADloadfunc, RGFW.RGFW_getProcAddress));

    var VAO: c_uint = 0;
    glad.glGenVertexArrays(1, &VAO);  
    glad.glBindVertexArray(VAO);

    var VBO: c_uint = 0;
    glad.glGenBuffers(1, &VBO);  
    glad.glBindBuffer(glad.GL_ARRAY_BUFFER, VBO);  
    glad.glBufferData(glad.GL_ARRAY_BUFFER, @sizeOf(@TypeOf(vertices)), &vertices, glad.GL_STATIC_DRAW);
   

    var EBO: c_uint = 0;
    glad.glGenBuffers(1, &EBO);

    glad.glBindBuffer(glad.GL_ELEMENT_ARRAY_BUFFER, EBO);
    glad.glBufferData(glad.GL_ELEMENT_ARRAY_BUFFER, @sizeOf(@TypeOf(indices)), &indices, glad.GL_STATIC_DRAW); 
    


    //load vs shader into storage
    const vs = glad.glCreateShader(glad.GL_VERTEX_SHADER);
    const vs_src = [1][*c]const u8{vertexshader.ptr};
    const vs_len = [1]c_int{@intCast(vertexshader.len)};

    glad.glShaderSource(vs, 1, &vs_src, &vs_len);
    glad.glCompileShader(vs);

    //load fs into storage
    const fs = glad.glCreateShader(glad.GL_FRAGMENT_SHADER);
    const fs_src = [1][*c]const u8{fragmentshader.ptr};
    const fs_len = [1]c_int{@intCast(fragmentshader.len)};

    glad.glShaderSource(fs, 1, &fs_src, &fs_len);
    glad.glCompileShader(fs);

    //Load shaders into program i think
    const shaderProgram: c_uint = glad.glCreateProgram();

    glad.glAttachShader(shaderProgram, vs);
    glad.glAttachShader(shaderProgram, fs);
    glad.glLinkProgram(shaderProgram);
    glad.glUseProgram(shaderProgram);

    glad.glVertexAttribPointer(0, 3, glad.GL_FLOAT, glad.GL_FALSE, 8 * @sizeOf(f32), null);
    glad.glEnableVertexAttribArray(0);

    glad.glVertexAttribPointer(1, 3, glad.GL_FLOAT, glad.GL_FALSE, 8 * @sizeOf(f32), @ptrFromInt(3 * @sizeOf(f32)));
    glad.glEnableVertexAttribArray(1);

    glad.glVertexAttribPointer(2, 2, glad.GL_FLOAT, glad.GL_FALSE, 8 * @sizeOf(f32), @ptrFromInt(6 * @sizeOf(f32)));
    glad.glEnableVertexAttribArray(2);

    //TEXTURE STUFF

    //texture
    var  width: i32 = 0;
    var  height: i32 = 0;
    var channels: c_int = 0;

    const data = stbi.stbi_load("client/textures/grass_top.png", &width, &height, &channels, 0);
    defer stbi.stbi_image_free(data);

    const format:c_uint = if (channels == 4) glad.GL_RGBA else glad.GL_RGB;

    var texture1: glad.GLuint = 1;
    glad.glGenTextures(1, &texture1);
    glad.glBindTexture(glad.GL_TEXTURE_2D, texture1);
    glad.glTexImage2D(glad.GL_TEXTURE_2D, 0, @intCast(format), width, height, 0, format, glad.GL_UNSIGNED_BYTE, data);



    glad.glTexParameteri(glad.GL_TEXTURE_2D, glad.GL_TEXTURE_MIN_FILTER, glad.GL_NEAREST);
    glad.glTexParameteri(glad.GL_TEXTURE_2D, glad.GL_TEXTURE_MAG_FILTER, glad.GL_NEAREST);
    
    

    const handle1: glad.GLuint64 = glad.glGetTextureHandleARB(texture1);
    glad.glMakeTextureHandleResidentARB(handle1);


    var texture2: glad.GLuint = 0;
    glad.glGenTextures(1, &texture2);
    glad.glBindTexture(glad.GL_TEXTURE_2D, texture2);
    glad.glTexParameteri(glad.GL_TEXTURE_2D, glad.GL_TEXTURE_MIN_FILTER, glad.GL_NEAREST);
    glad.glTexParameteri(glad.GL_TEXTURE_2D, glad.GL_TEXTURE_MAG_FILTER, glad.GL_NEAREST);



    glad.glTexImage2D(glad.GL_TEXTURE_2D, 0, @intCast(format), width, height, 0, format, glad.GL_UNSIGNED_BYTE, data);

    const handle2: glad.GLuint64 = glad.glGetTextureHandleARB(texture2);
    glad.glMakeTextureHandleResidentARB(handle2);
    

    const loc1: glad.GLint = glad.glGetUniformLocation(shaderProgram, "texture1");
    const loc2: glad.GLint = glad.glGetUniformLocation(shaderProgram, "texture2");

    if (loc1 != -1) glad.glUniformHandleui64ARB(loc1, handle1);
    if (loc2 != -1) glad.glUniformHandleui64ARB(loc2, handle2);

    _ = RGFW.RGFW_setKeyCallback(keyfunc);

    while (RGFW.RGFW_window_shouldClose(godStruct.render.win) == RGFW.RGFW_FALSE) {
        while (RGFW.RGFW_window_checkEvent(godStruct.render.win) != null) {
            if (win.*.event.type == RGFW.RGFW_windowResized){
                glad.glViewport(0, 0, win.*.r.w, win.*.r.h);
            }
        }

        var buffer: [32]u8 = undefined;
        const fps_str = std.fmt.bufPrintZ(&buffer, "Fyrraria: {d}", .{godStruct.render.fps}) catch unreachable;
        RGFW.RGFW_window_setName(godStruct.render.win, fps_str);
        //render only beneath
        glad.glClearColor(0.1, 0.1, 0.1, 1.0);
        glad.glClear(glad.GL_COLOR_BUFFER_BIT);
        glad.glBindVertexArray(VAO);
        glad.glDrawElements(glad.GL_TRIANGLES, 6, glad.GL_UNSIGNED_INT, null);

        RGFW.RGFW_window_swapBuffers(godStruct.render.win);
        const now = std.time.microTimestamp();
        if (startTime + 1_000_000 < now) {
            godStruct.render.fps = frames;
            frames = 0;
            startTime = now;
        }
        frames += 1;
    }

    RGFW.RGFW_window_close(godStruct.render.win);
}