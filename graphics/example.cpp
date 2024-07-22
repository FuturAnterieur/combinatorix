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
            "layout(location=0) in vec4 position;\n"
            "layout(location=1) in vec4 color0;\n"
            "out vec4 color;\n"
            "void main() {\n"
            "  gl_Position = position;\n"
            "  color = color0;\n"
            "}\n";
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
    while (!glfwWindowShouldClose(glfw_window())) {
        sg_pass the_pass = {};
        //the_pass.action.colors[0].clear_value = sg_color{0.0f, 0.0f, 0.f, 1.f};
        the_pass.swapchain = glfw_swapchain();
        sg_begin_pass(&the_pass);
        sg_apply_pipeline(pip);
        sg_apply_bindings(&bind);
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
