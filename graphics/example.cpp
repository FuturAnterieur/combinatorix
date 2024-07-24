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

    constexpr int num_instances = 2;
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
        -0.5f,  0.5f, 0.0f,     1.0f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, 0.0f,     0.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,     1.0f, 1.0f, 0.0f, 1.0f,
    };
    sg_buffer_desc vbuf_desc ={}; 
    vbuf_desc.data = SG_RANGE(vertices);
    vbuf_desc.label = "geometry-vertices";
    sg_buffer vbuf = sg_make_buffer(&vbuf_desc);

    // create an index buffer
    uint16_t indices[] = {
        0, 1, 2,    // first triangle
        0, 2, 3,    // second triangle
    };
    sg_buffer_desc ibuf_desc = {}; 
    ibuf_desc.type = SG_BUFFERTYPE_INDEXBUFFER; 
    ibuf_desc.data = SG_RANGE(indices);
    ibuf_desc.label = "geometry-indices";
    sg_buffer ibuf = sg_make_buffer(&ibuf_desc);

    sg_buffer_desc instance_data = {};
    instance_data.size = num_instances * sizeof(hmm_vec3);
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
            "layout(location=2) in vec3 instance_pos;\n"
            "out vec4 color;\n"
            "void main() {\n"
            "  vec4 pos = vec4(position + instance_pos, 1.0);"
            "  gl_Position = mvp * pos;\n"
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
    pip_desc.layout.buffers[0].stride = 28;
    pip_desc.layout.buffers[1].stride = 12;
    pip_desc.layout.buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE;
    pip_desc.layout.attrs[0].offset = 0; 
    pip_desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[0].buffer_index = 0;
    pip_desc.layout.attrs[1].offset = 3 * sizeof(float); 
    pip_desc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT4;
    pip_desc.layout.attrs[1].buffer_index = 0;
    pip_desc.layout.attrs[2].offset = 0; 
    pip_desc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT3; 
    pip_desc.layout.attrs[2].buffer_index = 1;

    // create a pipeline object (default render state is fine)
    sg_pipeline pip = sg_make_pipeline(&pip_desc);

    // draw loop
    float rz = 0.0f;
    float timer = 0.f;
    params_t vs_params;
    hmm_vec3 positions[num_instances];
    positions[0].X = 0.0f;
    positions[0].Y = 0.0f;
    positions[0].Z = 0.0f;
    positions[1].X = 0.0f;
    positions[1].Y = 0.f;
    positions[1].Z = 0.0f;
    

    while (!glfwWindowShouldClose(glfw_window())) {
        sg_pass the_pass = {};

        
        
        /*float length = HMM_SQRTF(1.f/3.f);

        hmm_vec3 eye = HMM_Vec3(length, length, length);
        hmm_vec3 center = HMM_Vec3(0.0f, 0.0f, 0.0f);
        hmm_vec3 up = HMM_Vec3(0.f, 1.f, 0.f);*/
        //hmm_mat4 proj = HMM_Perspective(60.0f, (float)glfw_width()/(float)glfw_height(), 0.01f, 10.0f);
        //hmm_mat4 view = HMM_LookAt(eye, center, HMM_Vec3(0.0f, 1.0f, 0.f));
        //hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);

        hmm_mat4 iso = {0};

        //hmm_vec3 F = HMM_NormalizeVec3(HMM_SubtractVec3(center, eye));
        //hmm_vec3 S = HMM_NormalizeVec3(HMM_Cross(F, up));
        //hmm_vec3 U = HMM_Cross(S, F);

        /*float factor = sqrtf(1/6.f);
        iso.Elements[0][0] = sqrtf(3) * factor;
        iso.Elements[0][1] = 0.0f;
        iso.Elements[0][2] = -sqrtf(3.f) * factor;

        iso.Elements[1][0] = 1.f * factor;
        iso.Elements[1][1] = 2.f * factor;
        iso.Elements[1][2] = 1.f * factor;

        iso.Elements[2][0] = sqrtf(2.f) * factor;
        iso.Elements[2][1] = -sqrtf(2.f) * factor;
        iso.Elements[2][2] = sqrtf(2.f) * factor;

        iso.Elements[3][3] = 1.0f;*/

        hmm_mat4 iso1 = HMM_Rotate(45.f, HMM_Vec3(0.f, 0.f, 1.f));
        hmm_mat4 iso2 = HMM_Rotate(35.264f, HMM_Vec3(1.f, 0.f, 0.f));
        iso = HMM_MultiplyMat4(iso2, iso1);
        iso.Elements[3][3] = 1.f;

        hmm_mat4 ortho = {0};
        ortho.Elements[0][0] = 1.f;
        ortho.Elements[1][1] = 1.f;
        ortho.Elements[2][2] = 1.f;
        ortho.Elements[3][3] = 1.f;

        hmm_mat4 ortho_iso = HMM_MultiplyMat4(ortho, iso);

        // rotated model matrix
        rz += 0.f;
        hmm_mat4 model = HMM_Rotate(rz, HMM_Vec3(0.0f, 0.0f, 1.0f));
        hmm_mat4 scale = HMM_Scale(HMM_Vec3(0.5f, 0.5f, 0.5f));
        hmm_mat4 model_scale = HMM_MultiplyMat4(model, scale);
        // model-view-projection matrix for vertex shader
        vs_params.mvp = HMM_MultiplyMat4(ortho_iso, model_scale);
        sg_range pos_buffers = sg_range{positions, num_instances * 3 * sizeof(float)};

        sg_update_buffer(bind.vertex_buffers[1], &pos_buffers);

        sg_range uniforms = SG_RANGE(vs_params);
        the_pass.swapchain = glfw_swapchain();
        sg_begin_pass(&the_pass);
        sg_apply_pipeline(pip);
        sg_apply_bindings(&bind);
        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &uniforms);
        sg_draw(0, 9, num_instances);
        sg_end_pass();
        sg_commit();
        glfwSwapBuffers(glfw_window());
        glfwPollEvents();
        timer += 0.005f;
    }

    /* cleanup */
    sg_shutdown();
    glfwTerminate();

    return 0;
}
