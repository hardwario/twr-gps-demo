#include <application.h>

twr_led_t led;
twr_led_t gps_led_r;
twr_led_t gps_led_g;

int top;

void screen_clear(void)
{
    twr_module_lcd_clear();
    twr_module_lcd_draw_string(2, 2, "BigClown GPS Module", true);
    twr_module_lcd_draw_line(0, 17, 127, 17, true);

    top = 25;
}

void screen_append(char *s)
{
    twr_log_info("APP: Append to screen: %s", s);

    twr_module_lcd_draw_string(2, top, s, true);

    top += 15;
}

void screen_update(void)
{
    twr_module_lcd_update();
}

void gps_module_event_handler(twr_module_gps_event_t event, void *event_param)
{
    if (event == TWR_MODULE_GPS_EVENT_START)
    {
        twr_log_info("APP: Event TWR_MODULE_GPS_EVENT_START");

        twr_led_set_mode(&gps_led_g, TWR_LED_MODE_ON);
    }
    else if (event == TWR_MODULE_GPS_EVENT_STOP)
    {
        twr_log_info("APP: Event TWR_MODULE_GPS_EVENT_STOP");

        twr_led_set_mode(&gps_led_g, TWR_LED_MODE_OFF);
    }
    else if (event == TWR_MODULE_GPS_EVENT_UPDATE)
    {
        twr_led_pulse(&gps_led_r, 50);

        screen_clear();

        char buffer[64];

        twr_module_gps_time_t time;

        if (twr_module_gps_get_time(&time))
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

        twr_module_gps_position_t position;

        if (twr_module_gps_get_position(&position))
        {
            snprintf(buffer, sizeof(buffer),
                "Lat: %03.5f", position.latitude);

            screen_append(buffer);

            snprintf(buffer, sizeof(buffer),
                "Lon: %03.5f", position.longitude);

            screen_append(buffer);
        }

        twr_module_gps_altitude_t altitude;

        if (twr_module_gps_get_altitude(&altitude))
        {
            snprintf(buffer, sizeof(buffer),
                "Altitude: %.1f %c",
                altitude.altitude, tolower(altitude.units));

            screen_append(buffer);
        }

        twr_module_gps_quality_t quality;

        if (twr_module_gps_get_quality(&quality))
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

        twr_module_gps_invalidate();
    }
    else if (event == TWR_MODULE_GPS_EVENT_ERROR)
    {
        twr_log_info("APP: Event TWR_MODULE_GPS_EVENT_ERROR");
    }
}

void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DUMP, TWR_LOG_TIMESTAMP_ABS);

    twr_led_init(&led, TWR_GPIO_LED, false, false);
    twr_led_set_mode(&led, TWR_LED_MODE_ON);

    twr_module_lcd_init();
    twr_module_lcd_set_font(&twr_font_ubuntu_13);

    screen_clear();
    screen_update();

    if (!twr_module_gps_init())
    {
        twr_log_error("APP: GPS Module initialization failed");
    }
    else
    {
        twr_module_gps_set_event_handler(gps_module_event_handler, NULL);
        twr_module_gps_start();
    }

    twr_led_init_virtual(&gps_led_r, TWR_MODULE_GPS_LED_RED, twr_module_gps_get_led_driver(), 0);
    twr_led_init_virtual(&gps_led_g, TWR_MODULE_GPS_LED_GREEN, twr_module_gps_get_led_driver(), 0);
}
