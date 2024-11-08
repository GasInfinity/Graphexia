#include "Graphexia.hpp"
#include "logo.hpp"

#include <cstdlib>
#include <iostream>
#include <sokol/sokol_app.h>
#include <sokol/sokol_gfx.h>
#include <sokol/sokol_glue.h>
#include <sokol/sokol_log.h>
#include "sokol/sokol_gp.h"

#include <nuklear/nuklear.h>
#include <sokol/util/sokol_nuklear.h>

void init(void* userdata) {
    Graphexia* graphexia = reinterpret_cast<Graphexia*>(userdata);

    sg_desc sg{};
    sg.logger.func = slog_func;
    sg.environment = sglue_environment();
    sg_setup(sg);
    
    if(!sg_isvalid()) {
        std::cout << "Error initializing sokol_gfx context" << std::endl; 
        std::exit(-1);
    }

    sgp_desc sgp{};
    sgp.color_format = SG_PIXELFORMAT_RGBA8;
    sgp.depth_format = SG_PIXELFORMAT_DEPTH_STENCIL;
    sgp.sample_count = 1;
    sgp.max_vertices = 65536;
    sgp.max_commands = 8192;
    sgp_setup(&sgp);

    if(!sgp_is_valid()) {
        std::cout << "Error initializing sokol_gp context: " << sgp_get_error_message(sgp_get_last_error()) << std::endl; 
        std::exit(-1);
    }

    snk_desc_t snk{};
    snk.max_vertices = 65536;
    snk.image_pool_size = 256;
    snk.color_format = SG_PIXELFORMAT_RGBA8;
    snk.depth_format = SG_PIXELFORMAT_DEPTH_STENCIL;
    snk.sample_count = 1;
    snk.dpi_scale = sapp_dpi_scale();
    snk.logger.func = slog_func;
    snk_setup(snk);

    graphexia->Init();
}

void frame(void* userdata) {
    Graphexia* graphexia = reinterpret_cast<Graphexia*>(userdata);

    nk_context* nuklearContext = snk_new_frame();
    graphexia->Update(nuklearContext);

    sg_pass defaultPass{};
    defaultPass.swapchain = sglue_swapchain();

    sg_begin_pass(&defaultPass);
    sgp_begin(sapp_width(), sapp_height());
    graphexia->Render();
    sgp_flush();
    sgp_end();

    snk_render(sapp_width(), sapp_height());
    sg_end_pass();
    sg_commit();
}

void cleanup(void* userdata) {
    Graphexia* graphexia = reinterpret_cast<Graphexia*>(userdata);
    delete graphexia;

    snk_shutdown();
    sgp_shutdown();
    sg_shutdown();
}

void event(const sapp_event* event, void* userdata) {
    Graphexia* graphexia = reinterpret_cast<Graphexia*>(userdata);

    if(snk_handle_event(event)) {
        return;
    }

    graphexia->Event(event);
}

sapp_desc sokol_main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    sapp_desc desc{};
    desc.window_title = "Graphexia";
    desc.icon = sapp_icon_desc{
        .sokol_default = false,
        .images = {
            { .width = 16, .height = 16, .pixels = SAPP_RANGE(x16Logo) },
            { .width = 32, .height = 32, .pixels = SAPP_RANGE(x32Logo) },
            { .width = 64, .height = 64, .pixels = SAPP_RANGE(x64Logo) },
            { .width = 128, .height = 128, .pixels = SAPP_RANGE(x128Logo) }
        }
    };
    desc.width = 1280;
    desc.height = 720;
    desc.logger.func = slog_func;
    desc.init_userdata_cb = init;
    desc.frame_userdata_cb = frame;
    desc.cleanup_userdata_cb = cleanup;
    desc.event_userdata_cb = event;
    desc.user_data = new Graphexia();
    return desc;
}
