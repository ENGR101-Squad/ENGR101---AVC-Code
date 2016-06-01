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

#define THRESH 100 //Changed from 80
#define BASE_SPEED 73 //Changed from 80
#define BASE_BACK_SPEED -75

void move(int left, int right);
void turn(int dir);
int p(int y, double pkp, int base_speed, int threshChange);
void signal_callback_handler(int signum)
{
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
	//Networking Code
	connect_to_server("130.195.6.196",1024);
	send_to_server("Please");
	char message[24];
	receive_from_server(message);
	send_to_server(message);
	//Signal Catcher code
	signal(2, signal_callback_handler);
	//Declaration of Constants for 2nd Quadrant
	double errorSum = 0;
	double errorSum2 = 0;
	double kp = 0.5;
	double kd = 5;
	double proportional_signal;
	open_screen_stream();
	int quad_three = 0;
	printf("QUADRANT ONE AND TWO\n");
	while(1) {
		quad_three = 0;
		take_picture();
		//CODE FOR PROPORTIONAL
		for (int i=0; i < 320; i++){
			if(get_pixel(i, 160, 3) > THRESH) {
				errorSum += (i-160);
				quad_three++;
			}
		}
		//CODE FOR FUTURE LINE ERROR
		for (int i=0; i < 320; i++){
			if(get_pixel(i, 80, 3) > THRESH) {
				errorSum2 += (i-160);
			}
		}
		errorSum/=160;
		errorSum2/=160;
		//THIS IS NOT ACTUALLY DERIVATIVE SIGNAL - THIS CODE:
		//Looks to see if there any curves in the line and slows down based on how curvy the curve is
		int derivative_signal = abs(errorSum-errorSum2)*kd;
		if(derivative_signal > 33) {
			derivative_signal = 33;
		}
		proportional_signal = errorSum * kp;
		int leftSpeed = BASE_SPEED + proportional_signal - derivative_signal;
		int rightSpeed = BASE_SPEED - proportional_signal - derivative_signal;
		if (proportional_signal < - 0.5 && proportional_signal > -0.6 && quad_three != 320){
			for(int i = 0; i < 10; i++) {
				move(BASE_BACK_SPEED, BASE_BACK_SPEED);
			}
		} else {
			if(quad_three == 320)break;
			move(leftSpeed, rightSpeed);
		}
	}
	printf("THIRD QUADRANT\n");
	move(50,50);
	Sleep(0,200000);
	while(1) {
		if (p(160,0.3,50,0) > 310 && p(40,0.3,50,0) < 80) {
			turn(1);
			break;
		}
	}
	int red = 0;
	int blue = 0;
	int green = 0;
	while(red < 200 || green > 100 || blue > 100) {
		take_picture();
		red = get_pixel(120,160,0);
		green = get_pixel(120,160,1);
		blue = get_pixel(120,160,2);
		int p160 = p(160,0.7,40,0);
		int p80 = p(80,0.7,40,0);
		if (p160 > 310 && p80 < 80) {
			turn(1);
		}
	}
	while (1){
		p(160,0.5,40,-40);//RANDOM CODE FOR QUADRANT 4
	}
	move(0,0);
	close_screen_stream();
}

//Simple method to set motors.
void move(int left, int right){
	set_motor(1, left);
	set_motor(2, right);
}

//Turns depending on the direction put
//	it should turn forever until a line is detected
void turn(int dir) {
	move(40,40);
	Sleep(0,300000);
	int left;
	int right;
	if (dir == 1){
		left = -60;
		right = 60;
		printf("Moving left\n");
	} else if (dir == 2){
		left = 60;
		right = -60;
		printf("Moving right\n");
	}
	int whiteC = 0;
	while (whiteC < 80){
		take_picture();
		whiteC = 0;
		for (int i = 100; i < 220; i++){
			if (get_pixel(i,80,3) > 127){
				whiteC++;
			}
		}
		move(left,right);
	}
	move(0,0);
	Sleep(0,300000);
}

//CODE for just Proportional Signal
// - This code takes multiple parameters
// - y is which line to read (robot only moves if y is 160)
// - pkp is just the proportional constant that can be changed (As quadrant three has sharp turns, this number needs to change often)
// - base_speed is just to balance with the pkp changes
// - threshChange is for quadrant four (we attempted to write code that can do quadrant 4 without IR Sensors hahaha)
// This method returns whiteC
int p(int y, double pkp, int base_speed, int threshChange){
	int p = 0;
	int whiteC2 = 0;
	int whiteC = 0;
	take_picture();
	for (int i = 0; i < 320; i++){
		if (get_pixel(i,y,3)>THRESH+threshChange){
			p += i-160;
			whiteC += 1;
		}
	}
	for (int i = 0; i <320; i++){
		if (get_pixel(i,80,3)>THRESH+threshChange){
			whiteC2 += 1;
		}
	}
	p = p*pkp/160;
	if (y == 160){
		if (whiteC == 0){
			move(BASE_BACK_SPEED,BASE_BACK_SPEED);
			Sleep(0,300000);
		} else {
		move(base_speed+p,base_speed-p);
		}
	}
	return whiteC;
}
