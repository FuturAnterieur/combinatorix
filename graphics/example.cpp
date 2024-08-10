//------------------------------------------------------------------------------
//  quad-glfw.c
//  Indexed drawing, explicit vertex attr locations.
//  Draw a PRISMATIC SHIELD
//------------------------------------------------------------------------------
#define SOKOL_IMPL
#define SOKOL_GLCORE
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "glfw_glue.h"
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/scalar_constants.hpp> // glm::pi
#include <glm/ext/matrix_clip_space.hpp>
#include <fstream>

#include <math.h>

struct params_t {
  glm::mat4 mvp;
};


glm::mat4 isometric(){
  glm::mat4 rot1 = glm::rotate(glm::mat4(1.f), glm::pi<float>()/4.f, glm::vec3(0.f, 1.f, 0.f));
  glm::mat4 rot2 = glm::rotate(glm::mat4(1.f), 35.264f * glm::pi<float>()/180.f, glm::vec3(1.f, 0.f, 0.f));
  return rot2 * rot1;
}

glm::vec3 cam_pos;
bool use_wikipedia = true;
bool incr_time = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
      cam_pos.z += 0.1f;
      cam_pos.x -= 0.1f;
    }
    else if(key == GLFW_KEY_S && action == GLFW_PRESS){
      cam_pos.z -= 0.1f;
      cam_pos.x += 0.1f;
    } 
    if(key == GLFW_KEY_A && action == GLFW_PRESS) {
      cam_pos.x -= 0.1f;
      cam_pos.z -= 0.1f;
    }
    else if(key == GLFW_KEY_D && action == GLFW_PRESS) {
      cam_pos.x += 0.1f;
      cam_pos.z += 0.1f;
    }

    if(key == GLFW_KEY_U && action == GLFW_PRESS){
      use_wikipedia = !use_wikipedia;
    }

    if(key == GLFW_KEY_SPACE && action == GLFW_RELEASE){
      incr_time = !incr_time;
    }
}

int main() {

    constexpr int num_instances = 2;
    // create window and GL context via GLFW
    glfw_desc_t glfw_desc = {};
    glfw_desc.title = "quad-glfw.c";
    glfw_desc.width = 640;
    glfw_desc.height = 640;

    float native_size = 640.f;
   
    glfw_init(&glfw_desc);

    glfwSetKeyCallback(glfw_window(), key_callback);

    // setup sokol_gfx
    sg_desc sokol_desc = {};
    sokol_desc.environment = glfw_environment();
    sokol_desc.logger.func = slog_func;
    
    sg_setup(&sokol_desc);
    assert(sg_isvalid());

    // create a vertex buffer
    float vertices[] = {
        // pos                  color                       uvs
        -1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,     0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,     1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,     1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,     0.0f, 1.0f,

        -1.0f, -1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,     0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,     1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,     0.0f, 1.0f,

        -1.0f, -1.0f, -1.0f,    0.0f, 0.0f, 1.0f, 1.0f,     0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f, 1.0f, 1.0f,     1.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,    0.0f, 0.0f, 1.0f, 1.0f,     1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f, 1.0f, 1.0f,     0.0f, 1.0f,

         1.0f, -1.0f, -1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     1.0f, 1.0f,
         1.0f, -1.0f,  1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     0.0f, 1.0f,

        -1.0f, -1.0f, -1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     0.0f, 1.0f,

        -1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     1.0f, 1.0f,
         1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     0.0f, 1.0f
    };
    sg_buffer_desc vbuf_desc ={}; 
    vbuf_desc.data = SG_RANGE(vertices);
    vbuf_desc.label = "geometry-vertices";
    sg_buffer vbuf = sg_make_buffer(&vbuf_desc);

    // create an index buffer
    uint16_t indices[] = {
        0, 1, 2,  0,2,3,
        6, 5, 4,  7, 6, 4,
        8, 9, 10,  8, 10, 11,
        14, 13, 12,  15, 14, 12,
        16, 17, 18,  16, 18, 19,
        22, 21, 20,  23, 22, 20
    };
    sg_buffer_desc ibuf_desc = {}; 
    ibuf_desc.type = SG_BUFFERTYPE_INDEXBUFFER; 
    ibuf_desc.data = SG_RANGE(indices);
    ibuf_desc.label = "geometry-indices";
    sg_buffer ibuf = sg_make_buffer(&ibuf_desc);

    sg_buffer_desc instance_data = {};
    instance_data.size = num_instances * sizeof(glm::vec3);
    instance_data.usage = SG_USAGE_STREAM;
    instance_data.label = "instance_data";

    // define the resource bindings
    sg_bindings bind = {};
    bind.vertex_buffers[0] = vbuf;
    bind.vertex_buffers[1] = sg_make_buffer(&instance_data);
    bind.index_buffer = ibuf;
    
    sg_shader_desc shd_desc = {};
    

    shd_desc.vs.source = "#version 330\n"
            "uniform mat4 mvp;\n"
            "layout(location=0) in vec3 position;\n"
            "layout(location=1) in vec4 color0;\n"
            "layout(location=2) in vec2 texcoord0;\n"
            "layout(location=3) in vec3 instance_pos;\n"
            "out vec4 color;\n"
            "out vec2 uv;"
            "void main() {\n"
            "  vec4 pos = vec4(position + instance_pos, 1.0);\n"
            "  gl_Position = mvp * pos;\n"
            "  color = color0;\n"
            "  uv = texcoord0;\n"
            "}\n";

    std::ifstream file("C:/Users/Pierre/Documents/Programmation/ChaosCombined/graphics/shaders/cosdwarp2d.fs", std::ifstream::in);
    std::string source;

    while(file.good()) 
    {
      int c = file.get();
      if(c >= 0) 
      {
        source += (char)c;
      }
      else 
      {
        break;
      }
    }
    file.close();
    
    shd_desc.vs.uniform_blocks[0].size = sizeof(params_t);
    shd_desc.vs.uniform_blocks[0].uniforms[0].name = "mvp";
    shd_desc.vs.uniform_blocks[0].uniforms[0].type = SG_UNIFORMTYPE_MAT4;
    
    
    shd_desc.fs.uniform_blocks[0].size = sizeof(float);
    shd_desc.fs.uniform_blocks[0].uniforms[0].name = "time";
    shd_desc.fs.uniform_blocks[0].uniforms[0].type = SG_UNIFORMTYPE_FLOAT;
    shd_desc.fs.source = source.data();
    /*shd_desc.fs.source = "#version 330\n"
            "in vec4 color;\n"
            "out vec4 frag_color;\n"
            "void main() {\n"
            "  frag_color = color;\n"
            "}\n";*/
    

    // create a shader (use vertex attribute locations)
    sg_shader shd = sg_make_shader(&shd_desc);
    
    sg_pipeline_desc pip_desc = {};
    pip_desc.shader = shd;
    pip_desc.index_type = SG_INDEXTYPE_UINT16;
    pip_desc.layout.buffers[0].stride = 36;

    pip_desc.layout.buffers[1].stride = 12;
    pip_desc.layout.buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE;

    pip_desc.layout.attrs[0].offset = 0; 
    pip_desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[0].buffer_index = 0;

    pip_desc.layout.attrs[1].offset = 3 * sizeof(float); 
    pip_desc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT4;
    pip_desc.layout.attrs[1].buffer_index = 0;

    pip_desc.layout.attrs[2].offset = 7 * sizeof(float); 
    pip_desc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.layout.attrs[2].buffer_index = 0;

    pip_desc.layout.attrs[3].offset = 0; 
    pip_desc.layout.attrs[3].format = SG_VERTEXFORMAT_FLOAT3; 
    pip_desc.layout.attrs[3].buffer_index = 1;

    pip_desc.cull_mode = SG_CULLMODE_BACK;
    pip_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    pip_desc.depth.write_enabled = true;

    // create a pipeline object (default render state is fine)
    sg_pipeline pip = sg_make_pipeline(&pip_desc);

    // draw loop
    float rz = 0.0f;
    float timer = 0.f;
    params_t vs_params;
    glm::vec3 positions[num_instances];
    positions[0].x = 0.0f;
    positions[0].y = 0.0f;
    positions[0].z = 3.0f;
    positions[1].x = 0.0f;
    positions[1].y = 0.0f;
    positions[1].z = 0.0f;
    

    while (!glfwWindowShouldClose(glfw_window())) {
        
        sg_pass the_pass = {};

        float height = static_cast<float>(glfw_height());
        float width =  static_cast<float>(glfw_width());
        float aspect_ratio =  height / width;
        float x_ratio = width / native_size;
        float y_ratio = aspect_ratio * x_ratio;

        glm::mat4 pure_ortho = glm::ortho(-x_ratio, x_ratio, -y_ratio, y_ratio, -5.f, 5.f);   
        glm::mat4 iso_lookAt = glm::lookAt(glm::vec3(-5.f, 5.f, 10.f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); 

        glm::mat4 ortho_iso = use_wikipedia ? 
                              pure_ortho * isometric() : //glm::scale(isometric(), glm::vec3(1.f / x_ratio, 1.f/(aspect_ratio * x_ratio), 1.f)) :
                              pure_ortho * iso_lookAt;
        

        // rotated model matrix
        rz = timer;
        glm::mat4 model = glm::scale(glm::mat4(1.f), glm::vec3(0.25f, 0.25f, 0.25f));
        model = glm::rotate(model, rz, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, cam_pos);
        
        // model-view-projection matrix for vertex shader
        vs_params.mvp = ortho_iso * model;
        sg_range pos_buffers = sg_range{positions, num_instances * 3 * sizeof(float)};

        sg_update_buffer(bind.vertex_buffers[1], &pos_buffers);

        //cam_pos.X = cam_pos.Y = cam_pos.Z = 0.f;
        sg_range vs_uniforms = SG_RANGE(vs_params);
        sg_range fs_uniforms = SG_RANGE(timer);
        the_pass.swapchain = glfw_swapchain();
        sg_begin_pass(&the_pass);
        sg_apply_pipeline(pip);
        sg_apply_bindings(&bind);
        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &vs_uniforms);
        sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &fs_uniforms);
        sg_draw(0, 36, 2);
        sg_end_pass();
        sg_commit();
        glfwSwapBuffers(glfw_window());
        glfwPollEvents();
        timer += incr_time ? 0.005f : 0.f;
    }
    
    /* cleanup */
    sg_shutdown();
    glfwTerminate();

    return 0;
}
