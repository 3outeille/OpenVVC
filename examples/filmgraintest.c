#include "post_proc.c"
#include "pp_film_grain.c"
#include "fg_compute_block_avg_asm.h"

#include <time.h>
#include <stdlib.h>
#include <string.h>

static int
write_decoded_frame_to_file(OVFrame *const frame, FILE *out_file)
{
    uint8_t component = 0;
    int ret = 0;
    struct Window output_window = frame->output_window;
    int bd_shift = (frame->frame_info.chroma_format == OV_YUV_420_P8) ? 0: 1;

    uint16_t add_w = (output_window.offset_lft + output_window.offset_rgt);
    uint16_t add_h = (output_window.offset_abv + output_window.offset_blw);

    if (add_w || add_h || frame->width < (frame->linesize[0] >> bd_shift)) {
        for (component = 0; component < 3; component++) {
            uint16_t comp_w = component ? add_w : add_w << 1;
            uint16_t comp_h = component ? add_h : add_h << 1;
            uint16_t win_left  =  component ? output_window.offset_lft : output_window.offset_lft << 1;
            uint16_t win_top   =  component ? output_window.offset_abv : output_window.offset_abv << 1;
            int frame_h = (frame->height >> (!!component)) - comp_h;
            int frame_w = (frame->width  >> (!!component)) - comp_w;

            int offset_h = win_top * frame->linesize[component] ;
            int offset   = offset_h + (win_left << bd_shift) ;
            const uint8_t *data = (uint8_t*)frame->data[component] + offset;

            for (int j = 0; j < frame_h; j++) {
                ret += fwrite(data, frame_w << bd_shift, sizeof(uint8_t), out_file);
                data += frame->linesize[component];
            }
        }
    } else {
        const uint8_t *data = (uint8_t*)frame->data[0];
        const uint8_t *data_cb = (uint8_t*)frame->data[1];
        const uint8_t *data_cr = (uint8_t*)frame->data[2];
        ret += fwrite(data, frame->size[0], sizeof(uint8_t), out_file);
        ret += fwrite(data_cb, frame->size[1], sizeof(uint8_t), out_file);
        ret += fwrite(data_cr, frame->size[2], sizeof(uint8_t), out_file);
    }

    return ret;
}

void run_benchmark_pp_process_frame(int (* func)(const OVSEI*, OVFrame **), char* name, size_t nb_fct_call, size_t iter_per_fct, OVSEI sei, OVFrame *frame)  {
    
    volatile double dummy;
    double total_avg_time = 0;

    for (size_t i = 0; i < nb_fct_call; ++i)
    {
        double avg_time = 0;

        for (size_t j = 0; j < iter_per_fct; ++j)
        {
            double t1 = clock();
            volatile const float r = func(&sei, &frame);
            double t2 = clock();

            dummy = r;
            double dt = (t2 - t1) * 1000 / CLOCKS_PER_SEC; // millisecond

            avg_time += dt;
        }

        avg_time /= (double)iter_per_fct;
        total_avg_time += avg_time;
    }
    
    total_avg_time /= (double)nb_fct_call;
    printf("%-20s: %f ms (average time / %lu calls of %lu iteration each)]\n", name, total_avg_time, nb_fct_call, iter_per_fct);
}


int main(int argc, char **argv)
{
    struct FramePool *pool = NULL;
    int ret = ovframepool_init(&pool, OV_YUV_420_P8, 8, 1920, 1080);
    /* TODO ret */

    OVFrame *frame = ovframepool_request_frame(pool);

    // memset(frame->data[0], 128, frame->size[0]);
    // memset(frame->data[1], 128, frame->size[1]);
    // memset(frame->data[2], 128, frame->size[2]);
    // printf("%lu, %lu, %lu\n", frame->size[0], frame->size[1],frame->size[2]);
    memset(frame->data[0], 0, frame->size[0]);
    memset(frame->data[1], 0, frame->size[1]);
    memset(frame->data[2], 0, frame->size[2]);

    struct OVSEIFGrain sei_fg = {
    /* uint8_t */ .fg_characteristics_cancel_flag = 0,
    /* uint8_t */ .fg_model_id = 0,
    /* uint8_t */ .fg_separate_colour_description_present_flag = 0,
    /* uint8_t */ .fg_bit_depth_luma_minus8 = 0,
    /* uint8_t */ .fg_bit_depth_chroma_minus8 = 0,
    /* uint8_t */ .fg_full_range_flag = 0,
    /* uint8_t */ .fg_colour_primaries = 0,
    /* uint8_t */ .fg_transfer_characteristics = 0,
    /* uint8_t */ .fg_matrix_coeffs = 0,
    /* uint8_t */ .fg_blending_mode_id = 0,
    /* uint8_t */ .fg_log2_scale_factor = 0,

    /* uint8_t[3] */ .fg_comp_model_present_flag = { 1, 1, 1},
    /* uint8_t[3] */ .fg_num_intensity_intervals_minus1 = { 0 },
    /* uint8_t[3] */ .fg_num_model_values_minus1 = { 0 },

    // /* uint8_t[3][8] */ .fg_intensity_interval_lower_bound = { { 0 } },
    /* uint8_t[3][8] */ .fg_intensity_interval_lower_bound = { { 254 } },
    /* uint8_t[3][8] */ .fg_intensity_interval_upper_bound = { { 0, 0, 0, 0, 0, 0, 0, 255 }, { 0, 0, 0, 0, 0, 0, 0, 255 }, { 0, 0, 0, 0, 0, 0, 0, 255 }},
    // /* uint8_t[3][8] */ .fg_intensity_interval_upper_bound = { { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }},
    
    /* int16_t[3][8][3] */ .fg_comp_model_value = { { { 0 } } },

    /* uint8_t */ .fg_characteristics_persistence_flag = 0,
    /* int16_t */ .fg_idr_pic = 0,
    };

    OVSEI sei =
    {
        .sei_fg = &sei_fg,
    };

    srand(42);

    // printf("First time\n");
    // pp_process_frame(&sei, &frame);
    // FILE *out_file = fopen("ref.frame", "w");
    // write_decoded_frame_to_file(frame, out_file);

    // printf("Second time\n");
    // pp_process_frame(&sei, &frame);

    size_t nb_fct_call = 100;
    size_t iter_per_fct = 10;

    run_benchmark_pp_process_frame(&pp_process_frame,
     "pp_process_frame",
      nb_fct_call,
      iter_per_fct,
      sei,
      frame
    );

    // func_fg_compute_block_avg_asm();

    return 0;
}
