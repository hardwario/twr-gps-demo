#include <application.h>

bc_led_t led;
bc_led_t gps_led_r;
bc_led_t gps_led_g;

int top;

void screen_clear(void)
{
    bc_module_lcd_draw_string(2, 2, "BigClown GPS Module", true);
    bc_module_lcd_draw_line(0, 17, 127, 17, true);

    top = 25;
}

void screen_append(char *s)
{
    bc_log_info("APP: Append to screen: %s", s);

    bc_module_lcd_draw_string(2, top, s, true);

    top += 15;
}

void screen_update(void)
{
    bc_module_lcd_update();
}

void gps_module_event_handler(bc_module_gps_event_t event, void *event_param)
{
    if (event == BC_MODULE_GPS_EVENT_START)
    {
        bc_log_info("APP: Event BC_MODULE_GPS_EVENT_START");

        bc_led_set_mode(&gps_led_g, BC_LED_MODE_ON);
    }
    else if (event == BC_MODULE_GPS_EVENT_STOP)
    {
        bc_log_info("APP: Event BC_MODULE_GPS_EVENT_STOP");

        bc_led_set_mode(&gps_led_g, BC_LED_MODE_OFF);
    }
    else if (event == BC_MODULE_GPS_EVENT_UPDATE)
    {
        bc_led_pulse(&gps_led_r, 50);

        screen_clear();

        char buffer[64];

        bc_module_gps_time_t time;

        if (bc_module_gps_get_time(&time))
        {
            snprintf(buffer, sizeof(buffer),
                "Date: %04d-%02d-%02d",
                time.year, time.month, time.day);

            screen_append(buffer);

            snprintf(buffer, sizeof(buffer),
                "Time: %02d:%02d:%02d",
                time.hours, time.minutes, time.seconds);

            screen_append(buffer);
        }

        bc_module_gps_position_t position;

        if (bc_module_gps_get_position(&position))
        {
            snprintf(buffer, sizeof(buffer),
                "Lat: %03.5f", position.latitude);

            screen_append(buffer);

            snprintf(buffer, sizeof(buffer),
                "Lon: %03.5f", position.latitude);

            screen_append(buffer);
        }

        bc_module_gps_altitude_t altitude;

        if (bc_module_gps_get_altitude(&altitude))
        {
            snprintf(buffer, sizeof(buffer),
                "Altitude: %.1f %c",
                altitude.altitude, tolower(altitude.units));

            screen_append(buffer);
        }

        bc_module_gps_quality_t quality;

        if (bc_module_gps_get_quality(&quality))
        {
            snprintf(buffer, sizeof(buffer),
                "Fix quality: %d",
                quality.fix_quality);

            screen_append(buffer);

            snprintf(buffer, sizeof(buffer),
                "Satellites: %d",
                quality.satellites_tracked);

            screen_append(buffer);
        }

        screen_update();

        bc_module_gps_invalidate();
    }
    else if (event == BC_MODULE_GPS_EVENT_ERROR)
    {
        bc_log_info("APP: Event BC_MODULE_GPS_EVENT_ERROR");
    }
}

void application_init(void)
{
    bc_log_init(BC_LOG_LEVEL_DUMP, BC_LOG_TIMESTAMP_ABS);

    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_ON);

    bc_module_lcd_init();
    bc_module_lcd_set_font(&bc_font_ubuntu_13);

    if (!bc_module_gps_init())
    {
        bc_log_error("APP: GPS Module initialization failed");
    }
    else
    {
        bc_module_gps_set_event_handler(gps_module_event_handler, NULL);
        bc_module_gps_start();
    }

    bc_led_init_virtual(&gps_led_r, BC_MODULE_GPS_LED_RED, bc_module_gps_get_led_driver(), 0);
    bc_led_init_virtual(&gps_led_g, BC_MODULE_GPS_LED_GREEN, bc_module_gps_get_led_driver(), 0);
}
