#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "lvgl.h"

lv_obj_t * SearchKeyboard, *SearchTextArea, *SearchResults, *PreviousButton, *RandomButton, *NextButton;

char rx_buffer[RT_SERIAL_RB_BUFSZ + 1];
static rt_device_t serial;
static struct rt_messagequeue rx_mq;

struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};

// Incoming UART text to mq
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    struct rx_msg msg;
    msg.dev = dev;
    msg.size = size;
    return rt_mq_send(&rx_mq, &msg, sizeof(msg));
}

// Display mq text
static void serial_thread_entry(void *parameter)
{
    struct rx_msg msg;
    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));
        if (rt_mq_recv(&rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER) == RT_EOK)
            rx_buffer[rt_device_read(msg.dev, 0, rx_buffer, msg.size)] = '\0';
        rt_thread_mdelay(100);
        lv_label_set_text(lv_textarea_get_label(SearchResults), rx_buffer);
    }
}

static void ButtonClicked(lv_event_t * e)
{
    const char * ThisButtonText = lv_label_get_text(lv_obj_get_child(lv_event_get_target(e), NULL));
    if (strcmp(ThisButtonText, "Previous") == 0) rt_device_write(serial, 0, "<", 1);
    else if (strcmp(ThisButtonText, "Random") == 0) rt_device_write(serial, 0, "_", 1);
    else if (strcmp(ThisButtonText, "Next") == 0) rt_device_write(serial, 0, ">", 1);
}

static void SearchTextAreaEvent(lv_event_t * e)
{
    const char * SearchTextAreaValue = lv_label_get_text(lv_textarea_get_label(SearchTextArea));
    switch (lv_event_get_code(e))
    {
    case LV_EVENT_CLICKED:
        lv_obj_move_foreground(SearchKeyboard);
        lv_obj_clear_flag(SearchKeyboard, LV_OBJ_FLAG_HIDDEN);
        break;
    case LV_EVENT_DEFOCUSED:
        lv_obj_move_foreground(SearchResults);
        lv_obj_move_foreground(PreviousButton);
        lv_obj_move_foreground(RandomButton);
        lv_obj_move_foreground(NextButton);
        lv_obj_add_flag(SearchKeyboard, LV_OBJ_FLAG_HIDDEN);
        break;
    case LV_EVENT_READY:
        // Send on UART
        rt_device_write(serial, 0, SearchTextAreaValue, sizeof(SearchTextAreaValue));
        lv_label_set_text(lv_textarea_get_label(SearchTextArea), "");
        lv_textarea_set_cursor_pos(SearchTextArea, 0);
        lv_obj_move_foreground(SearchResults);
        lv_obj_move_foreground(PreviousButton);
        lv_obj_move_foreground(RandomButton);
        lv_obj_move_foreground(NextButton);
        lv_obj_add_flag(SearchKeyboard, LV_OBJ_FLAG_HIDDEN);
        break;
    default:
        break;
    }
}

void lv_user_gui_init(void)
{
    static lv_style_t MainBackground, NavButtons;

    lv_style_init(&MainBackground);
    lv_style_set_bg_color(&MainBackground, lv_color_hex(0x0000ff));
    lv_style_set_bg_grad_color(&MainBackground, lv_color_hex(0x00ffff));
    lv_style_set_bg_grad_dir(&MainBackground, LV_GRAD_DIR_VER);
    lv_obj_add_style(lv_scr_act(), &MainBackground, 0);

    lv_style_init(&NavButtons);
    lv_style_set_bg_color(&NavButtons, lv_color_hex(0x0000ff));
    lv_style_set_text_color(&NavButtons, lv_color_hex(0x00ffff));

    SearchKeyboard = lv_keyboard_create(lv_scr_act());
    lv_keyboard_set_mode(SearchKeyboard, LV_KEYBOARD_MODE_SPECIAL);
    lv_obj_add_flag(SearchKeyboard, LV_OBJ_FLAG_HIDDEN);

    SearchTextArea = lv_textarea_create(lv_scr_act());
    lv_obj_align(SearchTextArea, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_size(SearchTextArea, 780, 70);
    lv_textarea_set_placeholder_text(SearchTextArea, "Engines of Our Ingenuity");
    lv_keyboard_set_textarea(SearchKeyboard, SearchTextArea);
    lv_obj_add_event_cb(SearchTextArea, SearchTextAreaEvent, LV_EVENT_ALL, SearchKeyboard);
    lv_obj_add_style(SearchTextArea, &MainBackground, 0);

    SearchResults = lv_textarea_create(lv_scr_act());
    lv_obj_align(SearchResults, LV_ALIGN_TOP_LEFT, 10, 90);
    lv_obj_set_size(SearchResults, 780, 380);
    lv_obj_move_foreground(SearchResults);
    lv_obj_add_state(SearchResults, LV_STATE_DISABLED);

    PreviousButton = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(PreviousButton, ButtonClicked, LV_EVENT_CLICKED, NULL);
    lv_obj_align(PreviousButton, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_set_width(PreviousButton, 220);
    lv_label_set_text(lv_label_create(PreviousButton), "Previous");
    lv_obj_align(lv_obj_get_child(PreviousButton, NULL), LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_style(PreviousButton, &NavButtons, 0);

    RandomButton = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(RandomButton, ButtonClicked, LV_EVENT_CLICKED, NULL);
    lv_obj_align(RandomButton, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_width(RandomButton, 220);
    lv_label_set_text(lv_label_create(RandomButton), "Random");
    lv_obj_align(lv_obj_get_child(RandomButton, NULL), LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(RandomButton, &NavButtons, 0);

    NextButton = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(NextButton, ButtonClicked, LV_EVENT_CLICKED, NULL);
    lv_obj_align(NextButton, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_set_width(NextButton, 220);
    lv_label_set_text(lv_label_create(NextButton), "Next");
    lv_obj_align(lv_obj_get_child(NextButton, NULL), LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_style(NextButton, &NavButtons, 0);
}

int main(void)
{
    static char msg_pool[256];
    serial = rt_device_find("uart6");
    rt_mq_init(&rx_mq, "rx_mq", msg_pool, sizeof(struct rx_msg), sizeof(msg_pool), RT_IPC_FLAG_FIFO);
    rt_device_open(serial, RT_DEVICE_FLAG_DMA_RX);
    rt_device_set_rx_indicate(serial, uart_input);
    rt_thread_startup(rt_thread_create("serial", serial_thread_entry, RT_NULL, 1024, 25, 10));
    while (1) rt_thread_mdelay(10);
}
