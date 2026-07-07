/*
 * Copyright (c) 2020 PHYTEC Messtechnik GmbH
 * Copyright (c) 2021 @weteor  (JD79653 support)
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * JD79653 (GoodDisplay GDEW0154M09) EPD controller driver.
 *
 * Ported to the current Zephyr device/SPI/GPIO/display API (gpio_dt_spec /
 * spi_dt_spec, spi_write_dt, DEVICE_DT_INST_DEFINE with a const config) from
 * weteor's ZMK fork, which targeted the old Zephyr 2.x API (device_get_binding,
 * *_LABEL macros, spi_cs_control). The controller command sequence and the
 * partial-window DTM1(old)/DTM2(new) + AUTO refresh logic are carried over
 * verbatim; only the OS-facing plumbing was modernised (mirroring ZMK's in-tree
 * il0323 driver, the JD79653's ported sibling).
 *
 * Only the black/white panel (KW mode) is supported, first gate/source = 0.
 */

#define DT_DRV_COMPAT gooddisplay_jd79653

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/byteorder.h>

#include "jd79653_regs.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(jd79653, CONFIG_DISPLAY_LOG_LEVEL);

#define EPD_PANEL_WIDTH DT_INST_PROP(0, width)
#define EPD_PANEL_HEIGHT DT_INST_PROP(0, height)
#define JD79653_PIXELS_PER_BYTE 8U

/* Horizontally aligned page! */
#define JD79653_NUMOF_PAGES (EPD_PANEL_WIDTH / JD79653_PIXELS_PER_BYTE)
#define JD79653_BUFFER_SIZE (EPD_PANEL_HEIGHT * JD79653_NUMOF_PAGES)

struct jd79653_cfg {
    struct gpio_dt_spec reset;
    struct gpio_dt_spec dc;
    struct gpio_dt_spec busy;
    struct spi_dt_spec spi;
};

static uint8_t jd79653_pwr[] = DT_INST_PROP(0, pwr);

/* Shadow of the panel RAM, needed for the JD79653 partial-update flow (DTM1 gets
 * the previous frame, DTM2 the new one). */
static uint8_t old_buffer[JD79653_BUFFER_SIZE];
static bool blanking_on = true;

static inline int jd79653_write_cmd(const struct jd79653_cfg *cfg, uint8_t cmd, uint8_t *data,
                                    size_t len) {
    struct spi_buf buf = {.buf = &cmd, .len = sizeof(cmd)};
    struct spi_buf_set buf_set = {.buffers = &buf, .count = 1};

    gpio_pin_set_dt(&cfg->dc, 1);
    if (spi_write_dt(&cfg->spi, &buf_set)) {
        return -EIO;
    }

    if (data != NULL) {
        buf.buf = data;
        buf.len = len;
        gpio_pin_set_dt(&cfg->dc, 0);
        if (spi_write_dt(&cfg->spi, &buf_set)) {
            return -EIO;
        }
    }

    return 0;
}

static inline void jd79653_busy_wait(const struct jd79653_cfg *cfg) {
    int pin = gpio_pin_get_dt(&cfg->busy);

    while (pin > 0) {
        __ASSERT(pin >= 0, "Failed to get pin level");
        k_msleep(JD79653_BUSY_DELAY);
        pin = gpio_pin_get_dt(&cfg->busy);
    }
}

static int jd79653_write(const struct device *dev, const uint16_t x, const uint16_t y,
                         const struct display_buffer_descriptor *desc, const void *buf) {
    const struct jd79653_cfg *cfg = dev->config;
    uint16_t x_end_idx = x + desc->width - 1;
    uint16_t y_end_idx = y + desc->height - 1;
    uint8_t ptl[JD79653_PTL_REG_LENGTH] = {0};
    size_t buf_len;
    uint8_t i;
    uint8_t *fb = (uint8_t *)buf;

    LOG_DBG("x %u, y %u, height %u, width %u, pitch %u", x, y, desc->height, desc->width,
            desc->pitch);

    buf_len = MIN(desc->buf_size, desc->height * desc->width / JD79653_PIXELS_PER_BYTE);
    __ASSERT(desc->width <= desc->pitch, "Pitch is smaller then width");
    __ASSERT(buf != NULL, "Buffer is not available");
    __ASSERT(buf_len != 0U, "Buffer of length zero");
    __ASSERT(!(desc->width % JD79653_PIXELS_PER_BYTE), "Buffer width not multiple of %d",
             JD79653_PIXELS_PER_BYTE);

    if ((y_end_idx > (EPD_PANEL_HEIGHT - 1)) || (x_end_idx > (EPD_PANEL_WIDTH - 1))) {
        LOG_ERR("Position out of bounds");
        return -EINVAL;
    }

    /* Setup Partial Window and enable Partial Mode */
    ptl[JD79653_PTL_HRST_IDX] = x;
    ptl[JD79653_PTL_HRED_IDX] = x_end_idx;
    ptl[JD79653_PTL_HRESERVED] = 0x00;
    ptl[JD79653_PTL_VRST_IDX] = y;
    ptl[JD79653_PTL_VRESERVED] = 0x00;
    ptl[JD79653_PTL_VRED_IDX] = y_end_idx;
    ptl[sizeof(ptl) - 1] = JD79653_PTL_PT_SCAN;

    jd79653_busy_wait(cfg);
    if (jd79653_write_cmd(cfg, JD79653_CMD_PTIN, NULL, 0)) {
        return -EIO;
    }

    if (jd79653_write_cmd(cfg, JD79653_CMD_PTL, ptl, sizeof(ptl))) {
        return -EIO;
    }

    /* DTM1 = previous frame */
    if (jd79653_write_cmd(cfg, JD79653_CMD_DTM1, old_buffer, JD79653_BUFFER_SIZE)) {
        return -EIO;
    }

    /* fold the new region into the shadow buffer */
    for (i = 0; i < desc->height; ++i) {
        memcpy(&old_buffer[(x + (y + i) * EPD_PANEL_WIDTH) / JD79653_PIXELS_PER_BYTE],
               &fb[(i * desc->pitch) / JD79653_PIXELS_PER_BYTE],
               desc->width / JD79653_PIXELS_PER_BYTE);
    }

    /* DTM2 = new frame */
    if (jd79653_write_cmd(cfg, JD79653_CMD_DTM2, fb, buf_len)) {
        return -EIO;
    }

    /* Auto power-on / refresh / power-off */
    ptl[0] = JD79653_AUTO_PON_DRF_POF;
    if (jd79653_write_cmd(cfg, JD79653_CMD_AUTO, ptl, 1)) {
        return -EIO;
    }

    if (jd79653_write_cmd(cfg, JD79653_CMD_PTOUT, NULL, 0)) {
        return -EIO;
    }

    return 0;
}

static int jd79653_read(const struct device *dev, const uint16_t x, const uint16_t y,
                        const struct display_buffer_descriptor *desc, void *buf) {
    LOG_ERR("not supported");
    return -ENOTSUP;
}

static void *jd79653_get_framebuffer(const struct device *dev) {
    LOG_ERR("not supported");
    return NULL;
}

static int jd79653_blanking_off(const struct device *dev) {
    blanking_on = false;
    return 0;
}

static int jd79653_blanking_on(const struct device *dev) {
    blanking_on = true;
    return 0;
}

static int jd79653_set_brightness(const struct device *dev, const uint8_t brightness) {
    LOG_WRN("not supported");
    return -ENOTSUP;
}

static int jd79653_set_contrast(const struct device *dev, uint8_t contrast) {
    LOG_WRN("not supported");
    return -ENOTSUP;
}

static void jd79653_get_capabilities(const struct device *dev, struct display_capabilities *caps) {
    memset(caps, 0, sizeof(struct display_capabilities));
    caps->x_resolution = EPD_PANEL_WIDTH;
    caps->y_resolution = EPD_PANEL_HEIGHT;
    caps->supported_pixel_formats = PIXEL_FORMAT_MONO10;
    caps->current_pixel_format = PIXEL_FORMAT_MONO10;
    caps->screen_info = SCREEN_INFO_MONO_MSB_FIRST | SCREEN_INFO_EPD;
}

static int jd79653_set_orientation(const struct device *dev,
                                   const enum display_orientation orientation) {
    LOG_ERR("Unsupported");
    return -ENOTSUP;
}

static int jd79653_set_pixel_format(const struct device *dev, const enum display_pixel_format pf) {
    if (pf == PIXEL_FORMAT_MONO10) {
        return 0;
    }

    LOG_ERR("not supported");
    return -ENOTSUP;
}

static int jd79653_clear_and_write_buffer(const struct device *dev, uint8_t pattern, bool update) {
    const struct jd79653_cfg *cfg = dev->config;

    memset(old_buffer, ~pattern, JD79653_BUFFER_SIZE);
    if (jd79653_write_cmd(cfg, JD79653_CMD_DTM1, old_buffer, JD79653_BUFFER_SIZE)) {
        return -EIO;
    }
    memset(old_buffer, pattern, JD79653_BUFFER_SIZE);
    if (jd79653_write_cmd(cfg, JD79653_CMD_DTM2, old_buffer, JD79653_BUFFER_SIZE)) {
        return -EIO;
    }

    if (update == true) {
        uint8_t auto_cmd = JD79653_AUTO_PON_DRF_POF;
        if (jd79653_write_cmd(cfg, JD79653_CMD_AUTO, &auto_cmd, 1)) {
            return -EIO;
        }
    }

    return 0;
}

static int jd79653_controller_init(const struct device *dev) {
    const struct jd79653_cfg *cfg = dev->config;
    uint8_t tmp[JD79653_TRES_REG_LENGTH];

    gpio_pin_set_dt(&cfg->reset, 1);
    k_msleep(JD79653_RESET_DELAY);
    gpio_pin_set_dt(&cfg->reset, 0);
    k_msleep(JD79653_RESET_DELAY);
    jd79653_busy_wait(cfg);

    LOG_DBG("Initialize JD79653 controller");

    /* panel settings - 200x200 res, booster on, BW mode, temp sense */
    tmp[0] = 0xdf;
    tmp[1] = 0x0e;
    if (jd79653_write_cmd(cfg, JD79653_CMD_PSR, tmp, 2)) {
        return -EIO;
    }

    /* power settings */
    if (jd79653_write_cmd(cfg, JD79653_CMD_PWR, jd79653_pwr, sizeof(jd79653_pwr))) {
        return -EIO;
    }

    /* Fiti-internal init sequence (not in the datasheet, used by GoodDisplay) */
    tmp[0] = 0x55;
    if (jd79653_write_cmd(cfg, JD79653_CMD_FITIINT_4D, tmp, 1)) {
        return -EIO;
    }
    tmp[0] = 0x0f;
    if (jd79653_write_cmd(cfg, JD79653_CMD_FITIINT_AA, tmp, 1)) {
        return -EIO;
    }
    tmp[0] = 0x02;
    if (jd79653_write_cmd(cfg, JD79653_CMD_FITIINT_E9, tmp, 1)) {
        return -EIO;
    }
    tmp[0] = 0x11;
    if (jd79653_write_cmd(cfg, JD79653_CMD_FITIINT_B6, tmp, 1)) {
        return -EIO;
    }
    tmp[0] = 0x0a;
    if (jd79653_write_cmd(cfg, JD79653_CMD_FITIINT_F3, tmp, 1)) {
        return -EIO;
    }

    /* resolution settings */
    tmp[JD79653_TRES_HRES_IDX] = EPD_PANEL_WIDTH;
    tmp[1] = 0x00;
    tmp[JD79653_TRES_VRES_IDX] = EPD_PANEL_HEIGHT;
    if (jd79653_write_cmd(cfg, JD79653_CMD_TRES, tmp, JD79653_TRES_REG_LENGTH)) {
        return -EIO;
    }

    /* tcon settings */
    tmp[0] = DT_INST_PROP(0, tcon);
    if (jd79653_write_cmd(cfg, JD79653_CMD_TCON, tmp, 1)) {
        return -EIO;
    }

    /* vcom DC settings */
    tmp[0] = DT_INST_PROP(0, vcom_dc);
    if (jd79653_write_cmd(cfg, JD79653_CMD_VDCS, tmp, 1)) {
        return -EIO;
    }

    tmp[0] = 0x97;
    if (jd79653_write_cmd(cfg, JD79653_CMD_CDI, tmp, 1)) {
        return -EIO;
    }

    tmp[0] = 0x00;
    if (jd79653_write_cmd(cfg, JD79653_CMD_PWS, tmp, 1)) {
        return -EIO;
    }

    if (jd79653_clear_and_write_buffer(dev, 0x00, false)) {
        return -1;
    }

    /* Enable Auto Sequence (power on -> refresh -> power off) */
    tmp[0] = JD79653_AUTO_PON_DRF_POF;
    if (jd79653_write_cmd(cfg, JD79653_CMD_AUTO, tmp, 1)) {
        return -EIO;
    }

    return 0;
}

static int jd79653_init(const struct device *dev) {
    const struct jd79653_cfg *cfg = dev->config;

    if (!spi_is_ready_dt(&cfg->spi)) {
        LOG_ERR("SPI device not ready for JD79653");
        return -EIO;
    }

    if (!device_is_ready(cfg->reset.port)) {
        LOG_ERR("Could not get GPIO port for JD79653 reset");
        return -EIO;
    }
    gpio_pin_configure_dt(&cfg->reset, GPIO_OUTPUT_INACTIVE);

    if (!device_is_ready(cfg->dc.port)) {
        LOG_ERR("Could not get GPIO port for JD79653 DC signal");
        return -EIO;
    }
    gpio_pin_configure_dt(&cfg->dc, GPIO_OUTPUT_INACTIVE);

    if (!device_is_ready(cfg->busy.port)) {
        LOG_ERR("Could not get GPIO port for JD79653 busy signal");
        return -EIO;
    }
    gpio_pin_configure_dt(&cfg->busy, GPIO_INPUT);

    return jd79653_controller_init(dev);
}

static const struct jd79653_cfg jd79653_config = {
    .spi = SPI_DT_SPEC_INST_GET(0, SPI_OP_MODE_MASTER | SPI_WORD_SET(8), 0),
    .reset = GPIO_DT_SPEC_INST_GET(0, reset_gpios),
    .busy = GPIO_DT_SPEC_INST_GET(0, busy_gpios),
    .dc = GPIO_DT_SPEC_INST_GET(0, dc_gpios),
};

static const struct display_driver_api jd79653_driver_api = {
    .blanking_on = jd79653_blanking_on,
    .blanking_off = jd79653_blanking_off,
    .write = jd79653_write,
    .read = jd79653_read,
    .get_framebuffer = jd79653_get_framebuffer,
    .set_brightness = jd79653_set_brightness,
    .set_contrast = jd79653_set_contrast,
    .get_capabilities = jd79653_get_capabilities,
    .set_pixel_format = jd79653_set_pixel_format,
    .set_orientation = jd79653_set_orientation,
};

DEVICE_DT_INST_DEFINE(0, jd79653_init, NULL, NULL, &jd79653_config, POST_KERNEL,
                      CONFIG_APPLICATION_INIT_PRIORITY, &jd79653_driver_api);
