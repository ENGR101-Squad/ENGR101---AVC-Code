#include <stdio.h>
#include <stdlib.h>
//#include <pthread.h>
#include <time.h>
#include <string.h>
#include <signal.h>

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
#define BASE_SPEED_QUAD_1 80
#define BASE_BACK_SPEED -75

int dsum = 1;
double errorSum = 0;
double errorSum2 = 0;
double kp = 0.5;
double kd = 5;
double proportional_signal;
int quad_three = 0;
void move(int left, int right);
void turn90(int dir);
void PID(int base_speed);
void signal_callback_handler(int signum) {
	//We caught sig 2 (ctrl+c)
	printf("Caught signal %d\n",signum);
	//Kill motors
	set_motor(1,0);
	set_motor(2,0);
	// Terminate program
	exit(signum);
}

int main() {
	init(0);
	//Add a ctrl+c handler that stops the motors
	signal(2, signal_callback_handler);
	open_screen_stream();
	//turn90(1);
	while(dsum != 0) {
		PID(BASE_SPEED_QUAD_1);
		if(quad_three == 320)break;
		dsum = get_pixel(10,10,3);
	}
	move(0,0);
	dsum = 1;
	while(dsum != 0) {
		dsum = get_pixel(10,10,3);
	}
	move(0,0);
	close_screen_stream();
}

void move(int left, int right){
	set_motor(1, left);
	set_motor(2, right);
}

void turn90(int dir) {
	int left;
	int right;
	switch(dir) {
		case 1://Left
			move(-50,50);
			break;
		case 2://Right
			move(50,-50);
			break;
	}
	printf("%d\n", left);
	printf("%d\n", right);
	move(left, right);
	sleep(0,300000);
	move(0,0);
}

void PID(int base_speed) {
	quad_three = 0;
	take_picture();
	//CODE FOR PROPORTIONAL
	for (int i=0; i < 320; i++){
		if(get_pixel(i, 160, 3) > THRESH) {
			errorSum += (i-160);
			quad_three++;
		}
	}
	for (int i=0; i < 320; i++){
		if(get_pixel(i, 80, 3) > THRESH) {
			errorSum2 += (i-160);
		}
	}
	printf("%d\n", quad_three);
	errorSum/=160;
	errorSum2/=160;
	int derivative_signal = abs(errorSum-errorSum2)*kd;
	if(derivative_signal > 40) {
		derivative_signal = 40;
	}
	proportional_signal = errorSum * kp;
	int leftSpeed = base_speed + proportional_signal - derivative_signal;
	int rightSpeed = base_speed - proportional_signal - derivative_signal;
	if (proportional_signal < - 0.5 && proportional_signal > -0.6){
		for(int i = 0; i < 10; i++) {
			move(BASE_BACK_SPEED, BASE_BACK_SPEED);
		}
	} else {
		if(leftSpeed < 40)leftSpeed = base_speed + proportional_signal - 40;
		if(rightSpeed < 40)rightSpeed = base_speed - proportional_signal - 40;
		if(quad_three == 320)break;
		move(leftSpeed, rightSpeed);
	}
}
