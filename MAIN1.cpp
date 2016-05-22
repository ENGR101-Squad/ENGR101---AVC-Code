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
#define THRESH 20
#define MAX_ERR 13056000
//#define SPEED 100

void move(int left, int right);

int main() {
    int dsum = 1;
    init(0);
	double errorSum = 0;
	double currError;
	double kp = 0.5;
	double proportional_signal;
    open_screen_stream();
    while(dsum != 0) {
	take_picture();
	//CODE FOR PROPORTIONAL
		for (int i=0; i < 320; i++){
			currError = ((i-160)*get_pixel(i, 120, 3));
			errorSum += currError;
		}
		errorSum/=(MAX_ERR/2);
		proportional_signal = errorSum * kp;
		double temp = (proportional_signal)*255;
		printf("%f\n", temp);
		move(50 + temp, 50 - temp);
		Sleep(0,100000);
	/*ps = psum*kp-128;
	printf("%d\n",(int)ps);
	move(ps,100);
	if (psum <  0){
		move(0,kright);
		printf("move left\n");
	} else {
		move(kleft,0);
		printf("move right\n");
	}
	printf("%d -- ",psum);

	//CODE FOR DERIVATIVE
	int d = dsum - psum;
	printf("%d\n",p);

    	dsum = psum;
		*/dsum = get_pixel(10,10,3);
    }
    move(0,0);
    close_screen_stream();
}

void move(int left, int right){
	set_motor(1, left);
	set_motor(2, right);
}
