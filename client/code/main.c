#define GLAD_GL_IMPLEMENTATION


#include "shader.h"
#include "cc.h"
#include "gl.h"
#include "utils.h"

#define STB_IMAGE_IMPLEMENTATION

#define RGFW_IMPLEMENTATION
#include <RGFW.h>


#include <cglm/cglm.h>
#include <stb_image.h>
#include "worldgen.h"
#include "cc.h"


void keyfunc(RGFW_window* win, unsigned char key, unsigned char keyChar, unsigned char keyMod, unsigned char pressed) {
    if (key == RGFW_escape && pressed) {
		RGFW_window_setShouldClose(win, True);
    }
}


void glm_perspective_infinite_z(float fovy, float aspect, float nearZ, mat4 dest) {
    float f = 1.0f / tanf(fovy / 2.0f);

    glm_mat4_zero(dest);
    dest[0][0] = f / aspect;
    dest[1][1] = f;
    dest[2][2] = 0.0f;
    dest[2][3] = -1.0f;
    dest[3][2] = nearZ;
}


int main() {
    const vec3u8 world_size = {13 ,10 ,10};


    char windowTitle[256] = "Fyrraria";
    RGFW_window* win = RGFW_createWindow(windowTitle, RGFW_RECT(0, 0, 800, 600), RGFW_windowCenter);
    gladLoadGL((GLADloadfunc) RGFW_getProcAddress);

    stbi_set_flip_vertically_on_load(1);
    
    if (!win) {
        fprintf(stderr, "Failed to create window or OpenGL context.\n");
        return -1;
    }

    vec( map(vec3int, Chunk) ) lodLayers;
    init( &lodLayers );

    map(vec3int, Chunk) baselevel;
    init(&baselevel);

    if( !push( &lodLayers, baselevel ) ){
        printf("out of memory\n");
        abort();
    }

    // how_many_lods should be the smallest value of world_size.xyz
    #define MIN(a, b) ((a) < (b) ? (a) : (b))
    const uint8_t how_many_lods = MIN(world_size.x, MIN(world_size.y, world_size.z));

    for (uint8_t lod_adding = 0; lod_adding < how_many_lods; lod_adding++) {
        map(vec3int, Chunk) LodOne;
        init(&LodOne);

        if( !push( &lodLayers, LodOne ) ){
            printf("out of memory for lod %hhu\n", lod_adding);
            abort();
        }
    }



    Player player;
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, player.position);
    const int render_distance = 2;
    vec3int previus_chunk_center;
    //do this for safe run
    previus_chunk_center = (vec3int){100, 100, 100};

    int shaderProgram = make_shader_program();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glEnable(GL_CULL_FACE);
    glClearDepth(0.0);

    uint64_t textuer_handles[] = {
        [GRASS] = load_texture_into_shader_bindless(shaderProgram, "client/textures/grass.png"),
        [TEST] = load_texture_into_shader_bindless(shaderProgram, "client/textures/tnt_test.png"),
        [LEAF] = load_texture_into_shader_bindless(shaderProgram, "client/textures/leaf.png"),
        [WOOD_LOG] = load_texture_into_shader_bindless(shaderProgram, "client/textures/log.png"),
        [WATER] = load_texture_into_shader_bindless(shaderProgram, "client/textures/water.png"),
    };

    float yaw = -90.0f;
    float pitch = 0.0f;
    const float sensitivity = 0.1f;

    RGFW_window_showMouse(win, 0);
    RGFW_window_mouseHold(win, RGFW_AREA(win->r.w / 2, win->r.h / 2));
    RGFW_setKeyCallback(keyfunc);

    
    RGFW_window_swapInterval(win, 0);
    int locations[6];
    get_GL_uniform_locations(shaderProgram, locations);

    glUniform1f(locations[GRID_X], (1 << world_size.x) * CHUNK_SIZE);
    glUniform1f(locations[GRID_Y], (1 << world_size.y) * CHUNK_SIZE);
    glUniform1f(locations[GRID_Z], (1 << world_size.z) * CHUNK_SIZE);

    u32 fps = 0;
    u32 frames = 0;
    const double startTime = RGFW_getTime();
    double lastFrameTime = RGFW_getTime();
    
    while (RGFW_window_shouldClose(win) == RGFW_FALSE) {
        PROFILE_BEGIN(GameLoop);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        double currentFrameStart = RGFW_getTime();
        double frameTime = currentFrameStart - lastFrameTime;
        lastFrameTime = currentFrameStart;

        while (RGFW_window_checkEvent(win) != NULL) {
            if (win->event.type == RGFW_quit) break;
            
            if (win->event.type == RGFW_mousePosChanged) {
                yaw -= win->event.vector.x * sensitivity;
                pitch -= win->event.vector.y * sensitivity;
            }

            if (win->event.type == RGFW_focusIn) {
                RGFW_window_showMouse(win, 0);
                RGFW_window_mouseHold(win, RGFW_AREA(win->r.w/2, win->r.h/2));
            }

            if (win->event.type == RGFW_windowResized) {
                glViewport(0, 0, win->r.w, win->r.h);
            }
        }
        
        // Clamp pitch values
        pitch = pitch > 89.0f ? 89.0f : pitch;
        pitch = pitch < -89.0f ? -89.0f : pitch;
        
        vec3 player_normal;
        analytical_torus_normal(player.position, player_normal, 1 << world_size.x, 1 << world_size.z, CHUNK_SIZE);
        
        vec3 transformed_position;
        transform_to_global_position_exp(player.position, transformed_position, 1 << world_size.x, 1 << world_size.z, CHUNK_SIZE);

        vec3 tangent, bitangent;
        get_torus_frame(player.position, tangent, bitangent, 1 << world_size.x, 1 << world_size.z, CHUNK_SIZE);

        vec3 camera_front_local;
        vec3 camera_front = {0.0f, 0.0f, -1.0f};
        rotate_camera(pitch, yaw, tangent, bitangent, camera_front, player_normal, camera_front_local);
        
        handle_movement(win, camera_front_local, player.position);
        // Update and generate chunks using hashmap
        
        update_nearby_chunks(&lodLayers, world_size, render_distance, player.position, &previus_chunk_center, textuer_handles);
        // Update window title with FPS
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Fyrraria: %d | Frame: %.2fms | Pos:X:%.1f Y:%.1f Z:%.1f", fps, frameTime * 1000.0, player.position[0], player.position[1], player.position[2]);
        RGFW_window_setName(win, buffer);

        // Set up view matrix
        mat4 view_matrix;
        vec3 center;
        glm_vec3_add(transformed_position, camera_front, center);
        glm_lookat(transformed_position, center, player_normal, view_matrix);
        glUniformMatrix4fv(locations[VIEW], 1, GL_FALSE, (const float *)view_matrix);

        // Set up projection matrix
        mat4 projection_matrix;
        glm_perspective_infinite_z(glm_rad(70.0f), (float)win->r.w / win->r.h, 0.1f, projection_matrix);
        glUniformMatrix4fv(locations[PROJECTION], 1, GL_FALSE, (const float *)projection_matrix);

        // Set model matrix
        mat4 model_matrix = GLM_MAT4_IDENTITY_INIT;
        glUniformMatrix4fv(locations[MODEL], 1, GL_FALSE, (const float *)model_matrix);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        PROFILE_BEGIN(Renderloop);
        // Render chunks from hashmap
        int lod = 0;
        for_each(&lodLayers, layer){
            glUniform1f(locations[LOD], (float)(1 << lod));
            for_each( layer, key, chunk ){
                if (chunk->vertices == 0) continue;
                    glBindVertexArray(chunk->VAO);
                    glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);
                    glDrawArrays(GL_TRIANGLES, 0, chunk->vertices);
            };
            lod++;
        };
        PROFILE_END(Renderloop);
        PROFILE_BEGIN(SwapBuffer);
        RGFW_window_swapBuffers(win);
        PROFILE_END(SwapBuffer);
        PROFILE_END(GameLoop);
        fps = RGFW_checkFPS(startTime, frames, 60);
        frames++;
    }

    // Clean up hashmap
    // NO MUHUHAHUA
    
    RGFW_window_close(win);
    return 0;
}
