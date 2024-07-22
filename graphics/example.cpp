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

#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"

struct params_t {
  hmm_mat4 mvp;
};

int main() {

    // create window and GL context via GLFW
    glfw_desc_t glfw_desc = {};
    glfw_desc.title = "quad-glfw.c";
    glfw_desc.width = 640;
    glfw_desc.height = 480;
   
    glfw_init(&glfw_desc);

    // setup sokol_gfx
    sg_desc sokol_desc = {};
    sokol_desc.environment = glfw_environment();
    sokol_desc.logger.func = slog_func;
    
    sg_setup(&sokol_desc);
    assert(sg_isvalid());

    // create a vertex buffer
    float vertices[] = {
        // positions            colors
        -0.5f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 0.0f, 1.0f,
         0.0f, -1.0f, 0.5f,     1.0f, 1.0f, 1.0f, 1.0f
    };
    sg_buffer_desc vbuf_desc ={}; 
    
    
    vbuf_desc.data = SG_RANGE(vertices);
    sg_buffer vbuf = sg_make_buffer(&vbuf_desc);

    // create an index buffer
    uint16_t indices[] = {
        0, 1, 2,    // first triangle
        0, 2, 3,    // second triangle
        3, 2, 4
    };
    sg_buffer_desc ibuf_desc = {}; 
    
    ibuf_desc.type = SG_BUFFERTYPE_INDEXBUFFER; 
    ibuf_desc.data = SG_RANGE(indices);
    sg_buffer ibuf = sg_make_buffer(&ibuf_desc);

    // define the resource bindings
    sg_bindings bind = {};
    
    bind.vertex_buffers[0] = vbuf;
    bind.index_buffer = ibuf;
    
    sg_shader_desc shd_desc = {};
    

    shd_desc.vs.source = "#version 330\n"
            "uniform mat4 mvp;\n"
            "layout(location=0) in vec4 position;\n"
            "layout(location=1) in vec4 color0;\n"
            "out vec4 color;\n"
            "void main() {\n"
            "  gl_Position = mvp * position;\n"
            "  color = color0;\n"
            "}\n";

    shd_desc.vs.uniform_blocks[0].size = sizeof(params_t);
    shd_desc.vs.uniform_blocks[0].uniforms[0].name = "mvp";
    shd_desc.vs.uniform_blocks[0].uniforms[0].type = SG_UNIFORMTYPE_MAT4;
    shd_desc.fs.source = "#version 330\n"
            "in vec4 color;\n"
            "out vec4 frag_color;\n"
            "void main() {\n"
            "  frag_color = color;\n"
            "}\n";
    

    // create a shader (use vertex attribute locations)
    sg_shader shd = sg_make_shader(&shd_desc);
    
    sg_pipeline_desc pip_desc = {};
    

    pip_desc.shader = shd;
    pip_desc.index_type = SG_INDEXTYPE_UINT16;
    pip_desc.layout.attrs[0].offset = 0; pip_desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[1].offset = 3 * sizeof(float); pip_desc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT4;

    // create a pipeline object (default render state is fine)
    sg_pipeline pip = sg_make_pipeline(&pip_desc);

    // draw loop
    float rz = 0.0f;
    params_t vs_params;
    while (!glfwWindowShouldClose(glfw_window())) {
        sg_pass the_pass = {};
        
        hmm_mat4 proj = HMM_Perspective(60.0f, (float)glfw_width()/(float)glfw_height(), 0.01f, 10.0f);
        hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 6.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
        hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);

        // rotated model matrix
        rz += 1.0f;
        hmm_mat4 model = HMM_Rotate(rz, HMM_Vec3(0.0f, 0.0f, 1.0f));
        
        // model-view-projection matrix for vertex shader
        vs_params.mvp = HMM_MultiplyMat4(view_proj, model);

        sg_range uniforms = SG_RANGE(vs_params);
        the_pass.swapchain = glfw_swapchain();
        sg_begin_pass(&the_pass);
        sg_apply_pipeline(pip);
        sg_apply_bindings(&bind);
        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &uniforms);
        sg_draw(0, 9, 1);
        sg_end_pass();
        sg_commit();
        glfwSwapBuffers(glfw_window());
        glfwPollEvents();
    }

    /* cleanup */
    sg_shutdown();
    glfwTerminate();

    return 0;
}
