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

//

static void screen_locker_draw_pixels(const screen_locker* sl, uint32_t* px, int32_t w, int32_t h)
{
}

static void buffer_release(void* data, struct wl_buffer* b)
{
    wl_buffer_destroy(b);
}

void screen_locker_output_fill_bg(screen_locker_output* output)
{
    sl_assert(output, "session output is NULL");

    free(output->bg);
    // rgba8 x w x h
    output->bg = (uint32_t*) malloc(4ull * output->w * output->h);
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

        int64_t w = output->w, h = output->h;
        int64_t iw = image->width, ih = image->height;
        int64_t ox = 0, oy = 0;

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
                output->bg[(y * w) + x] = (uint32_t) p[0] << 16 | (uint32_t) p[1] << 8 | p[2];
            }
        }
        break;
    }
    }
}

void screen_locker_redraw(struct screen_locker_output* output)
{
    if (!output->w || !output->h || !output->bg)
    {
        return;
    }

    const screen_locker* sl = output->parent;

    sl_assert(sl, "session locker is NULL");
    sl_assert(sl->shared_memory, "shared memory is NULL");

    const int32_t stride = (int32_t) output->w * 4;
    const int32_t size   = (int32_t) (stride * output->h);

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

    for (screen_locker_output* output = sl->ui.outputs; output; output = output->next)
    {
        if (output->surface)
        {
            screen_locker_redraw(output);
        }
    }
    wl_display_flush(sl->display);
}
