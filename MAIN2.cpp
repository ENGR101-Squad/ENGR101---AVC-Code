#include <stdio.h>
#include <stdlib.h>
//#include <pthread.h>
#include <time.h>
#include <string.h>

// sudo gcc -Wall
extern "C" int init_hardware();
extern "C" int init(int d_lev);

extern "C" int take_picture();
extern "C" char get_pixel(int row, int col, int color);
extern "C" void set_pixel(int col, int row, char red,char green,char blue);

extern "C" int open_screen_stream();
extern "C" int close_screen_stream();
extern "C" int update_screen();
//extern "C" void GetLine(int row,int threshold);
extern "C" int display_picture(int delay_sec,int delay_usec);
extern "C" int save_picture(char filename[5]);

extern "C" int set_motor(int motor,int speed);

extern "C" int read_analog(int ch_adc);
extern "C" int Sleep(int sec, int usec);
extern "C" int select_IO(int chan, int direct);
extern "C" int write_digital(int chan,char level);
extern "C" int read_digital(int chan);
extern "C" int set_PWM(int chan, int value);

extern "C" int connect_to_server( char server_addr[15],int port);
extern "C" int send_to_server(char message[24]);
extern "C" int receive_from_server(char message[24]);

void move(int left, int right);

int main() {
    int dsum = 1;
	double thresh;
	double black_avg = 0;
	double white_avg = 0;
	double left_sensor;
	double right_sensor;
	int left_velocity;
	int right_velocity;
    init(0);
    open_screen_stream();
	for(int i = 0; i < 20; i++) {
		take_picture();
		black_avg += get_pixel(0,120,3);
		white_avg += get_pixel(160,120,3);
	}
	black_avg/=20;
	white_avg/=20;
	thresh = white_avg + (black_avg - white_avg)/2;
    while(dsum != 0) {
		take_picture();
		left_sensor = get_pixel(120,120,3);
		right_sensor = get_pixel(120,200,3);
		if(left_sensor > thresh) {
			left_velocity = 0;
		} else {
			left_velocity = 50;
		}
		if(right_sensor > thresh) {
			right_velocity = 0;
		} else {
			right_velocity = 50;
		}
		printf("Left %d", left_velocity);
		printf("Right %d", right_velocity);
		move(left_velocity,right_velocity);
		dsum = get_pixel(10,10,3);
    }
    move(0,0);
    close_screen_stream();
}

void move(int left, int right){
	set_motor(1, left);
	set_motor(2, right);
}
