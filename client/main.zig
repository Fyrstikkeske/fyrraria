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

const zm = @import("zm");
const zigfp = @import("zigfp");

const GodStruct = struct { render: Render };

const Render = struct {
    win: ([*c]RGFW.struct_RGFW_window),
    fps: u32,
    delta: f32,
    shaderProgram: c_uint,
    VAO: c_uint,
};

const vertexshader = @embedFile("shaders/cube.vert");
const fragmentshader = @embedFile("shaders/cube.frag");

const vertices = [_]f32{
    -0.5, -0.5, -0.5,  0.0, 0.0,
     0.5, -0.5, -0.5,  1.0, 0.0,
     0.5,  0.5, -0.5,  1.0, 1.0,
     0.5,  0.5, -0.5,  1.0, 1.0,
    -0.5,  0.5, -0.5,  0.0, 1.0,
    -0.5, -0.5, -0.5,  0.0, 0.0,
    -0.5, -0.5,  0.5,  0.0, 0.0,
     0.5, -0.5,  0.5,  1.0, 0.0,
     0.5,  0.5,  0.5,  1.0, 1.0,
     0.5,  0.5,  0.5,  1.0, 1.0,
    -0.5,  0.5,  0.5,  0.0, 1.0,
    -0.5, -0.5,  0.5,  0.0, 0.0,
    -0.5,  0.5,  0.5,  1.0, 0.0,
    -0.5,  0.5, -0.5,  1.0, 1.0,
    -0.5, -0.5, -0.5,  0.0, 1.0,
    -0.5, -0.5, -0.5,  0.0, 1.0,
    -0.5, -0.5,  0.5,  0.0, 0.0,
    -0.5,  0.5,  0.5,  1.0, 0.0,
     0.5,  0.5,  0.5,  1.0, 0.0,
     0.5,  0.5, -0.5,  1.0, 1.0,
     0.5, -0.5, -0.5,  0.0, 1.0,
     0.5, -0.5, -0.5,  0.0, 1.0,
     0.5, -0.5,  0.5,  0.0, 0.0,
     0.5,  0.5,  0.5,  1.0, 0.0,
    -0.5, -0.5, -0.5,  0.0, 1.0,
     0.5, -0.5, -0.5,  1.0, 1.0,
     0.5, -0.5,  0.5,  1.0, 0.0,
     0.5, -0.5,  0.5,  1.0, 0.0,
    -0.5, -0.5,  0.5,  0.0, 0.0,
    -0.5, -0.5, -0.5,  0.0, 1.0,
    -0.5,  0.5, -0.5,  0.0, 1.0,
     0.5,  0.5, -0.5,  1.0, 1.0,
     0.5,  0.5,  0.5,  1.0, 0.0,
     0.5,  0.5,  0.5,  1.0, 0.0,
    -0.5,  0.5,  0.5,  0.0, 0.0,
    -0.5,  0.5, -0.5,  0.0, 1.0
};

const indices = [_]u32{ // note that we start from 0!
    0, 1, 3, // first triangle
    1, 2, 3, // second triangle
};

fn keyfunc(win: [*c]RGFW.RGFW_window, key: u8, keyChar: u8, keyMod: u8, pressed: u8) callconv(.C) void {
    _ = keyChar;
    _ = keyMod;
    if (key == RGFW.RGFW_escape and pressed != 0) {
        RGFW.RGFW_window_setShouldClose(win, RGFW.True);
    }
}

pub fn getandset_textureintoshaderfunction(shaderProgram: c_uint) void {
    //TEXTURE STUFF

    //texture
    var width: i32 = 0;
    var height: i32 = 0;
    var channels: c_int = 0;

    stbi.stbi_set_flip_vertically_on_load(1);
    const data = stbi.stbi_load("client/textures/grass_top.png", &width, &height, &channels, 0);
    if (data == null) {
        std.debug.print("Failed to load texture!\n", .{});
        unreachable;
    }
    defer stbi.stbi_image_free(data);

    const format: c_uint = if (channels == 4) glad.GL_RGBA else glad.GL_RGB;

    var texture1: glad.GLuint = 0;
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
}
pub fn getandsetupdrgfwandopenglsprobgram() GodStruct {
    RGFW.RGFW_setGLHint(RGFW.RGFW_glMajor, 3);
    RGFW.RGFW_setGLHint(RGFW.RGFW_glMinor, 3);

    const win = RGFW.RGFW_createWindow("Fyrraria", RGFW.RGFW_RECT(100, 100, 800, 600), RGFW.RGFW_windowCenter);
    if (win == null) {
        std.debug.print("Failed to create window\n", .{});
        unreachable;
    }
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

    glad.glVertexAttribPointer(0, 3, glad.GL_FLOAT, glad.GL_FALSE, 5 * @sizeOf(f32), null);
    glad.glEnableVertexAttribArray(0);

    glad.glVertexAttribPointer(1, 2, glad.GL_FLOAT, glad.GL_FALSE, 5 * @sizeOf(f32), @ptrFromInt(3 * @sizeOf(f32)));
    glad.glEnableVertexAttribArray(1);

    return GodStruct{ .render = .{ .win = win, .fps = 0, .VAO = VAO, .shaderProgram = shaderProgram, .delta = 0.0 } };
}

pub fn main() !void {
    var frames: u32 = 0;
    var startTime: i64 = std.time.microTimestamp();

    //init much to keep main clean
    var godStruct: GodStruct = getandsetupdrgfwandopenglsprobgram();


    var cameraPos: zm.Vec3f = .{ 0.0, 0.0, 3.0 };
    const cameraFront: zm.Vec3f = .{ 0.0, 0.0, -1.0 };
    const cameraUp: zm.Vec3f = .{ 0.0, 1.0, 0.0 };

    //Texture
    getandset_textureintoshaderfunction(godStruct.render.shaderProgram);
    
    glad.glEnable(glad.GL_DEPTH_TEST);
    
    // Matrices
    var move: f32 = 0.0;
    _ = RGFW.RGFW_setKeyCallback(keyfunc);
    while (RGFW.RGFW_window_shouldClose(godStruct.render.win) == RGFW.RGFW_FALSE) {
        glad.glClear(glad.GL_COLOR_BUFFER_BIT | glad.GL_DEPTH_BUFFER_BIT);
        const lastframe: i64 = std.time.microTimestamp();
        move += godStruct.render.delta;
        while (RGFW.RGFW_window_checkEvent(godStruct.render.win) != null) {
            if (godStruct.render.win.*.event.type == RGFW.RGFW_windowResized) {
                glad.glViewport(0, 0, godStruct.render.win.*.r.w, godStruct.render.win.*.r.h);
            }
            if (godStruct.render.win.*.event.type == RGFW.RGFW_quit) {break;}
        }
        if (godStruct.render.win.*.event.key == 'w') {cameraPos += cameraFront * @Vector(3, f32){ 0.05, 0.05, 0.05 };}
        if (godStruct.render.win.*.event.key == 's') {cameraPos -= cameraFront * @Vector(3, f32){ 0.05, 0.05, 0.05 };}
        //glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100.0f);

        var model = zm.Mat4f.identity();
        model = model.multiply(zm.Mat4f.rotation(zm.Vec3f{ 1.0, 0.0, 0.0 }, std.math.degreesToRadians(-55.0)));

        var view = zm.Mat4f.identity();
        view = view.multiply(zm.Mat4f.lookAt(cameraPos, cameraPos + cameraFront, cameraUp));

        const projection = zm.Mat4f.perspective(std.math.degreesToRadians(45), @as(f32, @floatFromInt(godStruct.render.win.*.r.w)) / @as(f32, @floatFromInt(godStruct.render.win.*.r.h)), 0.1, 100.0);

        const modelLoc: i32 = glad.glGetUniformLocation(godStruct.render.shaderProgram, "model");
        const viewLoc: i32 = glad.glGetUniformLocation(godStruct.render.shaderProgram, "view");
        const projectionLoc: i32 = glad.glGetUniformLocation(godStruct.render.shaderProgram, "projection");

        if (modelLoc == -1) {
            std.debug.print("cant find model in shader location \n", .{});
        }
        if (viewLoc == -1) {
            std.debug.print("cant find view in shader location \n", .{});
        }
        if (projectionLoc == -1) {
            std.debug.print("cant find projection in shader location \n", .{});
        }
        glad.glUniformMatrix4fv(modelLoc, 1, glad.GL_FALSE, &model.transpose().data[0]);
        glad.glUniformMatrix4fv(viewLoc, 1, glad.GL_FALSE, &view.transpose().data[0]);
        glad.glUniformMatrix4fv(projectionLoc, 1, glad.GL_FALSE, &projection.transpose().data[0]);

        var buffer: [32]u8 = undefined;
        const fps_str = std.fmt.bufPrintZ(&buffer, "Fyrraria: {d}", .{godStruct.render.fps}) catch unreachable;
        RGFW.RGFW_window_setName(godStruct.render.win, fps_str);
        //render only beneath
        glad.glClearColor(0.1, 0.1, 0.1, 1.0);
        glad.glClear(glad.GL_COLOR_BUFFER_BIT);
        glad.glBindVertexArray(godStruct.render.VAO);

        glad.glDrawArrays(glad.GL_TRIANGLES, 0, 36);
        //glad.glDrawElements(glad.GL_TRIANGLES, 36, glad.GL_UNSIGNED_INT, null);


        
        RGFW.RGFW_window_swapBuffers(godStruct.render.win);
        const thisframe: i64 = std.time.microTimestamp();
        godStruct.render.delta = @as(f32, @floatFromInt(thisframe - lastframe)) * 0.000001;
        if (startTime + 1_000_000 < thisframe) {
            godStruct.render.fps = frames;
            frames = 0;
            startTime = thisframe;
        }
        frames += 1;
    }

    RGFW.RGFW_window_close(godStruct.render.win);
}
