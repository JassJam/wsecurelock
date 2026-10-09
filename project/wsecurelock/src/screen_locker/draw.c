#include "screen_locker/draw.h"

#include "screen_locker/logger.h"
#include "screen_locker/screen_locker.h"

#include <wayland-client.h>
#include "utils/memory.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define SCREENLOCK_FD_MEM_NAME "wsecurelock-memfd"

#define GET_R(x) ((x >> 16) & 0xFF)
#define GET_G(x) ((x >> 8) & 0xFF)
#define GET_B(x) ((x & 0xFF))

static void color_blend(const uint32_t src, uint32_t* pdst, int a)
{
    if (a <= 0)
    {
        return;
    }

    if (a > 255)
    {
        a = 255;
    }

    const uint32_t inv_a = 255 - a;
    const uint32_t dst   = *pdst;

    const uint32_t r = ((GET_R(src) * a) + (GET_R(dst) * inv_a)) / 0xFF;
    const uint32_t g = ((GET_G(src) * a) + (GET_G(dst) * inv_a)) / 0xFF;
    const uint32_t b = ((GET_B(src) * a) + (GET_B(dst) * inv_a)) / 0xFF;

    *pdst = (r << 16u) | (g << 8u) | b;
}

static uint32_t cov_rrect(const int x,
                          const int y,
                          const int x0,
                          const int y0,
                          const int w,
                          const int h,
                          const int r)
{
    uint32_t n = 0;
    for (int sy = 0; sy < 2; sy++)
    {
        for (int sx = 0; sx < 2; sx++)
        {
            const int px = (2 * x) + sx;
            const int py = (2 * y) + sy;

            const int X0 = (2 * x0);
            const int Y0 = 2 * y0;

            const int X1 = 2 * (x0 + w);
            const int Y1 = 2 * (y0 + h);

            const int R = 2 * r;
            if (px < X0 || px >= X1 || py < Y0 || py >= Y1)
            {
                continue;
            }

            const int cx = px < X0 + R ? X0 + R : (px >= X1 - R ? X1 - R : px);
            const int cy = py < Y0 + R ? Y0 + R : (py >= Y1 - R ? Y1 - R : py);
            const int dx = px - cx;
            const int dy = py - cy;
            if ((dx * dx) + (dy * dy) <= R * R)
            {
                n++;
            }
        }
    }
    return n;
}

static int cov_circle(const int x, const int y, const int cx, const int cy, const int r)
{
    return cov_rrect(x, y, cx - r, cy - r, 2 * r, 2 * r, r);
}

static void fill_rect(uint32_t*      px,
                      const int      W,
                      const int      H,
                      const int      x0,
                      const int      y0,
                      const int      w,
                      const int      h,
                      const int      r,
                      const uint32_t rgb,
                      const int      alpha)
{
    for (int y = y0; y < y0 + h; y++)
    {
        for (int x = x0; x < x0 + w; x++)
        {
            if (x < 0 || y < 0 || x >= W || y >= H)
            {
                continue;
            }

            const int c = cov_rrect(x, y, x0, y0, w, h, r);
            color_blend(rgb, &px[((size_t) y * W) + x], alpha * c / 4);
        }
    }
}

static void ring_rect(uint32_t*      px,
                      const int      W,
                      const int      H,
                      const int      x0,
                      const int      y0,
                      const int      w,
                      const int      h,
                      const int      r,
                      const int      t,
                      const uint32_t rgb)
{
    for (int y = y0; y < y0 + h; y++)
    {
        for (int x = x0; x < x0 + w; x++)
        {
            if (x < 0 || y < 0 || x >= W || y >= H)
            {
                continue;
            }

            const int co = cov_rrect(x, y, x0, y0, w, h, r);
            const int ci =
                cov_rrect(x, y, x0 + t, y0 + t, w - (2 * t), h - (2 * t), r > t ? r - t : 0);
            const int c = co - ci;
            if (c > 0)
            {
                color_blend(rgb, &px[((size_t) y * W) + x], 255 * c / 4);
            }
        }
    }
}

static void fill_circle(uint32_t*      px,
                        const int      W,
                        const int      H,
                        const int      cx,
                        const int      cy,
                        const int      r,
                        const uint32_t rgb)
{
    for (int y = cy - r; y <= cy + r; y++)
    {
        for (int x = cx - r; x <= cx + r; x++)
        {
            if (x < 0 || y < 0 || x >= W || y >= H)
            {
                continue;
            }
            int c = cov_circle(x, y, cx, cy, r);
            color_blend(rgb, &px[((size_t) y * W) + x], 255 * c / 4);
        }
    }
}

static void screen_locker_draw_pixels(const screen_locker* sl,
                                      uint32_t*            px,
                                      const int32_t        w,
                                      const int32_t        h)
{
    static const uint32_t borders[] = {
        [SL_UI_IDLE]     = 0x6b6f76, //
        [SL_UI_TYPING]   = 0x4c8bf5, //
        [SL_UI_VERIFY]   = 0xf5c542, //
        [SL_UI_FAIL]     = 0xe5484d, //
        [SL_UI_UNLOCKED] = 0x000000, //
    };

    static const uint32_t background_col = 0x101114;
    static const uint32_t no_pw_color    = 0x40444b;
    static const uint32_t pw_color       = 0xf2f3f5;

    static const int radius = 7, pitch = 26;
    static const int char_width = 24, default_char_count = 20;

    const int pw_size    = password_count_chars(&sl->pw);
    const int char_count = ((pw_size > default_char_count) ? pw_size : default_char_count) + 4;

    const int bw = char_count * char_width, bh = 64;

    const int fit = (bw - char_width) / pitch;
    const int k   = pw_size < fit ? pw_size : fit;

    const int x0 = (w - bw) / 2, y0 = (h - bh) / 2;

    fill_rect(px, w, h, x0, y0, bw, bh, 0, background_col, 0xF1);
    ring_rect(px, w, h, x0, y0, bw, bh, 0, 2, borders[sl->ui.state]);

    if (k == 0)
    {
        const int width = 30, height = 5;

        fill_rect(px,
                  w,
                  h,
                  (w / 2) - width,
                  y0 + (bh / 2) - height,
                  width * 2,
                  height * 2,
                  4,
                  no_pw_color,
                  0xFF);
        return;
    }

    const int width  = ((k - 1) * pitch) + (2 * radius);
    const int char_x = x0 + ((bw - width) / 2) + radius;
    const int char_y = y0 + (bh / 2);
    for (int i = 0; i < k; i++)
    {
        fill_circle(px, w, h, char_x + (i * pitch), char_y, radius, pw_color);
    }
}

static void buffer_release(void* user_data, struct wl_buffer* b)
{
    wl_buffer_destroy(b);
}

void screen_locker_output_fill_bg(screen_locker_output* output)
{
    sl_assert(output, "session output is NULL");

    free(output->bg);
    output->bg = (uint32_t*) malloc(sizeof(uint32_t) * output->w * output->h);
    if (!output->bg)
    {
        return;
    }

    lockscreen_args* args = &output->parent->ui.args;
    switch (args->type)
    {
    case LOCKSCREEN_ARG_FLAT:
    {
        lockscreen_args_flat* flat = &args->flat;

        for (uint64_t i = 0; i < (uint64_t) output->w * output->h; i++)
        {
            output->bg[i] = flat->color;
        }
        break;
    }
    case LOCKSCREEN_ARG_IMG:
    {
        lockscreen_args_image* image = &args->image;

        const int64_t w  = output->w;
        const int64_t h  = output->h;
        int64_t       iw = image->width, ih = image->height;
        int64_t       ox = 0, oy = 0;

        // wide screen
        if (w * ih > h * iw)
        {
            ih = (int64_t) image->width * h / w;
            oy = (image->height - ih) / 2;
        }
        else
        {
            iw = (int64_t) image->height * w / h;
            ox = (image->width - ih) / 2;
        }

        for (int64_t y = 0; y < h; y++)
        {
            int64_t sy = oy + (y * ih / h);
            for (int64_t x = 0; x < w; x++)
            {
                int64_t              sx = ox + (x * iw / w);
                const unsigned char* p  = image->data + (4 * ((sy * image->width) + sx));
                output->bg[(y * w) + x] = (uint32_t) p[0] << 16u | (uint32_t) p[1] << 8u | p[2];
            }
        }
        break;
    }
    }
}

void screen_locker_redraw(const struct screen_locker_output* output)
{
    if (!output->w || !output->h || !output->bg)
    {
        return;
    }

    const screen_locker* sl = output->parent;

    sl_assert(sl, "session locker is NULL");
    sl_assert(sl->shared_memory, "shared memory is NULL");

    const uint32_t stride = output->w * sizeof(uint32_t);
    const uint32_t size   = stride * output->h;

    const int fd     = sl_memory_memfile_create(SCREENLOCK_FD_MEM_NAME, size);
    uint32_t* pixels = (uint32_t*) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (pixels == MAP_FAILED)
    {
        sl_memory_memfile_close(fd);
        return;
    }

    memcpy(pixels, output->bg, size);
    screen_locker_draw_pixels(sl, pixels, output->w, output->h);
    munmap(pixels, size);

    struct wl_shm_pool* pool = wl_shm_create_pool(sl->shared_memory, fd, size);
    struct wl_buffer*   buffer =
        wl_shm_pool_create_buffer(pool, 0, output->w, output->h, stride, WL_SHM_FORMAT_XRGB8888);

    wl_shm_pool_destroy(pool);
    sl_memory_memfile_close(fd);

    static const struct wl_buffer_listener listeners = { .release = buffer_release };
    wl_buffer_add_listener(buffer, &listeners, NULL);

    wl_surface_attach(output->surface, buffer, 0, 0);
    wl_surface_damage_buffer(output->surface, 0, 0, output->w, output->h);
    wl_surface_commit(output->surface);
}

void screen_locker_redraw_all(screen_locker* sl)
{
    sl_assert(sl, "session locker is NULL");
    sl_assert(sl->display, "display is NULL");

    for (const screen_locker_output* output = sl->ui.outputs; output; output = output->next)
    {
        if (output->surface)
        {
            screen_locker_redraw(output);
        }
    }
    wl_display_flush(sl->display);
}
