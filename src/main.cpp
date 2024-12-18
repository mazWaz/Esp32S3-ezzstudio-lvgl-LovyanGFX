#include <Arduino.h>
#include <lvgl.h>
#include <ui/ui.h>

#include "lovyanGfxSetup.h"

#define TFT_HOR_RES SCREEN_WIDTH
#define TFT_VER_RES SCREEN_HEIGHT

/* LVGL draws into this buffer, 1/10 screen size usually works well. The size is
 * in bytes. */
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))

uint32_t draw_buf[DRAW_BUF_SIZE / 4];

LGFX tft;

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf)
{
    LV_UNUSED(level);
    Serial.println(buf);
    Serial.flush();
}
#endif

/* LVGL calls it when a rendered image needs to copied to the display. */
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.writePixels((lgfx::rgb565_t *)px_map, w * h);
    tft.endWrite();

    /* Call it to tell LVGL you are ready. */
    lv_disp_flush_ready(disp);
}

/* Read the touchpad. */
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    uint16_t touchX, touchY;
    bool touched = tft.getTouch(&touchX, &touchY);

    if (!touched)
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
    else
    {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = touchX;
        data->point.y = touchY;
    }
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);

    tft.begin();
    tft.setRotation(0);
    tft.setBrightness(255);

    lv_init();

    lv_tick_set_cb((lv_tick_get_cb_t)millis);

#if LV_USE_LOG != 0
    lv_log_register_print_cb(my_print);
#endif

    /* Create a display. */
    lv_display_t *disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    /* Initialize the (dummy) input device driver. */
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(
        indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
    lv_indev_set_read_cb(indev, my_touchpad_read);

    Serial.println("Setup done");

    ui_init();
}

void loop()
{
    lv_timer_handler(); /* let the GUI do its work */
    ui_tick();
    delay(5);
}
