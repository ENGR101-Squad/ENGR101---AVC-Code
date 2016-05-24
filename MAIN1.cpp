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
#define THRESH 80
#define BASE_SPEED 40
#define BASE_BACK_SPEED -50

void move(int left, int right);

int main() {
    init(0);
	printf("test");
	int dsum = 1;
	double errorSum = 0;
	double kp = 0.5;
	double kd = 0.5;
	double proportional_signal;
    open_screen_stream();
    while(dsum != 0) {
	take_picture();
	//CODE FOR PROPORTIONAL
		for (int i=0; i < 320; i++){
			if(get_pixel(i, 120, 3) > THRESH) {
				errorSum += (i-160);
			}
		}
		errorSum/=160;
		proportional_signal = errorSum * kp;
		printf("%f\n", proportional_signal);
		if (proportional_signal < - 0.5 && proportional_signal > -0.6){
			move(BASE_BACK_SPEED, BASE_BACK_SPEED);
		} else {
			move(BASE_SPEED + proportional_signal, BASE_SPEED - proportional_signal);
		}
		//Sleep(0,100000);
		dsum = get_pixel(10,10,3);
    }
    move(0,0);
    close_screen_stream();
}

void move(int left, int right){
	set_motor(1, left);
	set_motor(2, right);
}
