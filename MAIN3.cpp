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
#define BASE_SPEED 80
#define BASE_BACK_SPEED -75

void move(int left, int right);
void turn(int dir);
int p(int y);
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
//	connect_to_server("130.195.6.196",1024);
//	send_to_server("Please");
//	char message[24];
//	receive_from_server(message);
//	send_to_server(message);
	signal(2, signal_callback_handler);
	int stop = 0;
	double errorSum = 0;
	double errorSum2 = 0;
	double kp = 0.5;
	double kd = 5;
	double proportional_signal;
	open_screen_stream();
	int quad_three = 0;
	printf("QUADRANT ONE AND TWO\n");
	while(stop != 0) {
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
		//printf("%d\n", quad_three);
		errorSum/=160;
		errorSum2/=160;
		int derivative_signal = abs(errorSum-errorSum2)*kd;
		if(derivative_signal > 40) {
			derivative_signal = 40;
		}
		proportional_signal = errorSum * kp;
		int leftSpeed = BASE_SPEED + proportional_signal - derivative_signal;
		int rightSpeed = BASE_SPEED - proportional_signal - derivative_signal;
		if (proportional_signal < - 0.5 && proportional_signal > -0.6){
			for(int i = 0; i < 10; i++) {
				move(BASE_BACK_SPEED, BASE_BACK_SPEED);
			}
		} else {
			if(leftSpeed < 40)leftSpeed = BASE_SPEED + proportional_signal - 40;
			if(rightSpeed < 40)rightSpeed = BASE_SPEED - proportional_signal - 40;
			if(quad_three == 320)break;
			move(leftSpeed, rightSpeed);
		}
		stop = get_pixel(10,10,3);
	}
	printf("THIRD QUADRANT\n");
	Sleep(0,400000);
	stop = 1;
	while(stop != 0) {
		take_picture();
		stop = get_pixel(10,10,3);
		int p160 = p(160);
		int p80 = p(80);
		//printf("160 %d\n", p160);
		//printf("160 %d\n", p80);
		if (p160 > 310 && p80 < 80) {
			turn(1);
		}
	}
	//move(40,40);
	//Sleep(0,300000);
	turn(1);
	move(0,0);
	close_screen_stream();
}

void move(int left, int right){
	set_motor(1, left);
	set_motor(2, right);
}

void turn(int dir) {
	int left;
	int right;
	//move(50,50);
	//Sleep(0,500000);
	if (dir == 1){
		left = -70;
		right = 70;
		printf("Moving left\n");
	} else if (dir == 2){
		left = 70;
		right = -70;
		printf("Moving right\n");
	}
	move(0,0);
//	Sleep(0,300000);
	int whiteC = 0;
	while (whiteC < 40){//whiteC < 80
		take_picture();
		whiteC = 0;
		for (int i = 100; i < 220; i++){
			if (get_pixel(i,80,3) > 127){
				whiteC++;
			}
		}
		//printf("%d\n", whiteC);
		//if (whiteC > 80){
			printf("%d\n", whiteC);
		//}
		move(left,right);
	}
	move(0,0);
	Sleep(0,300000);
}

int p(int y){
	int p = 0;
	int pkp = 0.6;
	int base_speed = 50;
	int whiteC2 = 0;
	int whiteC = 0;
	take_picture();
	for (int i = 0; i < 320; i++){
		if (get_pixel(i,y,3)>THRESH){
			p += i-160;
			whiteC += 1;
		}
	}
	for (int i = 0; i <320; i++){
		if (get_pixel(i,80,3)>THRESH){
			whiteC2 += 1;
		}
	}
	p = p*pkp/160;
	printf("%d\n",p);
	if (whiteC2 > 80){
		move(base_speed+p,base_speed-p);
	} else if (p < -35){
		turn(1);
	} else if (p > 35){
		turn(2);
	}
	return whiteC;
}
