/*
 * Widgets.c
 *
 *  Created on: 20 Aug 2024
 *      Author: jondurrant
 */

#include "lvgl.h"
#include "LVGL_example.h"
#include "hardware/uart.h"


#define UART_ID uart1
#define BAUD_RATE 115200
#define UART_TX_PIN 4
#define UART_RX_PIN 5

extern const lv_img_dsc_t Eye;
extern const lv_img_dsc_t mask;


int eyeX = 0;
int eyeY = 0;
int minSpeed = 1;
int maxSpeed = 2;
uint8_t step = 0;

int stopCount = 0;

int16_t steps[10][2] = {
		{0,0},
		{40,0},
		{-40,0},
		{0, 30},
		{40,0},
		{0,0},
};
//uint8_t numSteps =6;

int eyesSteps[10][5] = {
		{2000, 0, 0, 0, 0},
		{0500, 0, 0, 0, 0},
		{1000, 0, 20, 0, 20},
		{0700, 0, 20, 0, 20},
		{1000, 0, 0, 0, 0},
		{1000, 0, 0, 0, 0},
		{2000, 40, 0, 40, 0},
		{0500, 40, 0, 40, 0},
		{4000, -40, 0, -40, 0},
		{0500, -40, 0, -40, 0}
};
uint8_t numSteps =10;


uint32_t startTime=0;
float delX = 0.0;
float delY = 0.0;


static void setupUart(){
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
    gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));
}

static void startEyeMove(int ms, int targetX, int targetY){
	delX = (float)( targetX - eyeX) / (float)ms;
	delY = (float)( targetY - eyeY) / (float)ms;

}

static void deltaEyeMove(uint32_t sinceStart, lv_obj_t *eyeObj){
	int x = eyeX + (int)(delX * (float)sinceStart);
	int y = eyeY + (int)(delY * (float)sinceStart);

	lv_obj_align(eyeObj, LV_ALIGN_CENTER, x, y);
}

static void tellSubEyeMove(int ms, int targetX, int targetY){
	char buf[80];
	sprintf(buf, "%d,%d,%d\n", ms, targetX, targetY);
	uart_puts(uart1, buf);

	printf("Sub Move to: %s\n", buf);

	int tms, tx, ty;
	sscanf(buf,"%d,%d,%d", &tms, &tx, &ty);
	printf("Scanned %d, %d, %d\n", tms, tx, ty);

}

static void domEyePos(void * obj, int32_t v){
	lv_obj_t *eyeObj =(lv_obj_t *) obj;

	uint32_t now = to_ms_since_boot (get_absolute_time());
	uint32_t sinceStart = now - startTime;
	if (sinceStart > eyesSteps[step][0]){
		eyeX = eyesSteps[step][1];
		eyeY = eyesSteps[step][2];
		lv_obj_align(eyeObj, LV_ALIGN_CENTER, eyeX, eyeY);
		step++;
		if (step >= numSteps){
			step = 0;
		}
		startEyeMove(eyesSteps[step][0], eyesSteps[step][1], eyesSteps[step][2]);
		tellSubEyeMove(eyesSteps[step][0], eyesSteps[step][3], eyesSteps[step][4]);
		startTime = now;
	} else {
		  deltaEyeMove(sinceStart, eyeObj);
	}
}

int subMS = 0;
int subX = 0;
int subY = 0;

#define BUF_LEN 80
char buf[BUF_LEN];
uint bufI = 0;


static void subRead(){
	//printf("subRead: ");
	while (uart_is_readable (uart1)){
		char c = uart_getc (uart1);
		buf[bufI] = c;
		bufI++;
		if (bufI >= BUF_LEN){
			bufI=0;
		}
		if (c == '\n'){
			eyeX = subX;
			eyeY = subY;
			sscanf(buf,"%d,%d,%d", &subMS, &subX, &subY);
			printf("Scanned %d, %d, %d\n", subMS, subX, subY);
			bufI = 0;
			startEyeMove(subMS, subX, subY);
			startTime =  to_ms_since_boot (get_absolute_time());
			return;
		}
		//printf("%c", c);
	}
	//printf("Done\n");

}

static void subEyePos(void * obj, int32_t v){
	lv_obj_t *eyeObj =(lv_obj_t *) obj;

	subRead();

	uint32_t now = to_ms_since_boot (get_absolute_time());
	uint32_t sinceStart = now - startTime;
	if (sinceStart > subMS){
		eyeX = subX;
		eyeY = subY;
		lv_obj_align(eyeObj, LV_ALIGN_CENTER, eyeX, eyeY);
	} else {
		  deltaEyeMove(sinceStart, eyeObj);
	}
}



static void eyePos(void * obj, int32_t v){
	if ((eyeX == steps[step][0]) && (eyeY == steps[step][1])){
		stopCount ++;
		if (stopCount < 50){
			return;
		}
		stopCount = 0;
		step++;
		if (step >= numSteps){
			step = 0;
		}
	}

	int dif;

	dif = steps[step][0] - eyeX;
	if (dif != 0){
		if (dif > 0){
			if (dif > maxSpeed){
				eyeX +=  maxSpeed;
			} else {
				eyeX +=  minSpeed;
			}
		} else {
			if ((dif* -1) > maxSpeed){
				eyeX -=  maxSpeed;
			} else {
				eyeX -=  minSpeed;
			}
		}
	}

	dif = steps[step][1] - eyeY;
	if (dif != 0){
		if (dif > 0){
			if (dif > maxSpeed){
				eyeY +=  maxSpeed;
			} else {
				eyeY +=  minSpeed;
			}
		} else {
			if ((dif* -1) > maxSpeed){
				eyeY -=  maxSpeed;
			} else {
				eyeY -=  minSpeed;
			}
		}
	}

	lv_obj_t *img1 =(lv_obj_t *) obj;
	lv_obj_align(img1, LV_ALIGN_CENTER, eyeX, eyeY);
}


/********************************************************************************
function:	Initializes the layout of LVGL widgets
parameter:
********************************************************************************/
void Widgets_Init(void)
{
	 setupUart();

    // /*Style Config*/
    static lv_style_t style_base;
    lv_style_init(&style_base);
    lv_style_set_bg_color(&style_base, lv_palette_main(LV_PALETTE_LIGHT_GREEN));
    lv_style_set_border_color(&style_base, lv_palette_darken(LV_PALETTE_LIGHT_GREEN, 3));
    lv_style_set_border_width(&style_base, 2);
    lv_style_set_radius(&style_base, 10);
    lv_style_set_shadow_width(&style_base, 10);
    lv_style_set_shadow_ofs_y(&style_base, 5);
    lv_style_set_shadow_opa(&style_base, LV_OPA_50);
    lv_style_set_text_color(&style_base, lv_color_make(0, 0, 0xFF));
    lv_style_set_width(&style_base, 100);
    lv_style_set_height(&style_base, LV_SIZE_CONTENT);

    static lv_style_t style_press;
    lv_style_init(&style_press);
    lv_style_set_bg_color(&style_press, lv_palette_main(LV_PALETTE_GREEN));
    lv_style_set_border_color(&style_press, lv_palette_darken(LV_PALETTE_GREEN, 3));
    lv_style_set_border_width(&style_press, 2);
    lv_style_set_radius(&style_press, 10);
    lv_style_set_shadow_width(&style_press, 10);
    lv_style_set_shadow_ofs_y(&style_press, 5);
    lv_style_set_shadow_opa(&style_press, LV_OPA_50);
    lv_style_set_text_color(&style_press, lv_color_white());
    lv_style_set_width(&style_press, 100);
    lv_style_set_height(&style_press, LV_SIZE_CONTENT);

    static lv_style_t style_slider;
    lv_style_set_bg_color(&style_slider, lv_palette_main(LV_PALETTE_ORANGE));
    lv_style_set_border_color(&style_slider, lv_palette_darken(LV_PALETTE_ORANGE, 3));

    static lv_style_t style_indic;
    lv_style_init(&style_indic);
    lv_style_set_bg_color(&style_indic, lv_palette_lighten(LV_PALETTE_DEEP_ORANGE, 3));
    lv_style_set_bg_grad_color(&style_indic, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_HOR);

    static lv_style_t style_indic_pr;
    lv_style_init(&style_indic_pr);
    lv_style_set_shadow_color(&style_indic_pr, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_shadow_width(&style_indic_pr, 10);
    lv_style_set_shadow_spread(&style_indic_pr, 3);

    static lv_style_t style_sw;
    lv_style_init(&style_sw);
    lv_style_set_bg_opa(&style_sw, LV_OPA_COVER);
    lv_style_set_bg_color(&style_sw, lv_palette_lighten(LV_PALETTE_RED, 1));
    lv_style_set_shadow_width(&style_sw, 55);
    lv_style_set_shadow_color(&style_sw, lv_palette_main(LV_PALETTE_PINK));

    static lv_style_t style_roller;
    lv_style_init(&style_roller);
    lv_style_set_border_color(&style_roller, lv_palette_darken(LV_PALETTE_BLUE, 3));
    lv_style_set_shadow_width(&style_roller, 55);
    lv_style_set_shadow_color(&style_roller, lv_palette_main(LV_PALETTE_BLUE_GREY));
    static lv_style_t style_list;
    lv_style_set_shadow_width(&style_list, 55);
    lv_style_set_shadow_color(&style_list, lv_palette_main(LV_PALETTE_GREY));

    static lv_style_t style_imu_label;
    lv_style_init(&style_imu_label);
    lv_style_set_text_color(&style_imu_label,lv_palette_main(LV_PALETTE_PURPLE));;

    // /*Create tileview*/
    lv_obj_t *tv = lv_tileview_create(lv_scr_act());
    lv_obj_set_scrollbar_mode(tv,  LV_SCROLLBAR_MODE_OFF);
   // lv_group_add_obj(group, tv);





    //Jon test Tile

    lv_obj_t *tileJD = lv_tileview_add_tile(tv, 0, 0, LV_DIR_TOP|LV_DIR_BOTTOM);

	LV_IMG_DECLARE(Eye);
	lv_obj_t *img1 = lv_img_create(tileJD);
	lv_img_set_src(img1, &Eye);
	lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);

	lv_anim_t a;
	lv_anim_init(&a);
	//lv_anim_set_var(&a, arc);
	lv_anim_set_var(&a, img1);

	lv_anim_set_time(&a, 3000);
	lv_anim_set_repeat_delay(&a, 0);
	lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);    /*Just for the demo*/
	lv_anim_set_values(&a, 0, 100);
	//lv_anim_set_exec_cb(&a, eyePos);
#ifdef SUB_EYE
	lv_anim_set_exec_cb(&a, subEyePos);
#else
	lv_anim_set_exec_cb(&a, domEyePos);
#endif
	lv_anim_start(&a);


	LV_IMG_DECLARE(mask);
	lv_obj_t *mask1 = lv_img_create(tileJD);
	lv_img_set_src(mask1, &mask);
	lv_obj_align(mask1, LV_ALIGN_CENTER, 0, 0);





}



