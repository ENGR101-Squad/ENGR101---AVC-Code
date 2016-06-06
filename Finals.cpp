#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>

extern "C" int init_hardware();
extern "C" int init(int d_lev);

extern "C" int take_picture();
extern "C" char get_pixel(int row, int col, int color);
extern "C" void set_pixel(int col, int row, char red,char green,char blue);

extern "C" int open_screen_stream();
extern "C" int close_screen_stream();
extern "C" int update_screen();

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

#define THRESH 120
#define BASE_BACK_SPEED -75
previous_signal = -2;

//Simple method to set motors.
void move(int left, int right){
    set_motor(1, (int)left);
    set_motor(2, (int)right);
}

//Turns depending on the direction put
//	it should turn forever until a line is detected
void turn(int dir) {
    move(40,40);
    Sleep(0,300000);
    int left;
    int right;
    if (dir == 1){
        left = -100;
        right = 100;
        printf("Moving left\n");
    } else if (dir == 2){
        left = 100;
        right = -100;
        printf("Moving right\n");
    }
    int pixel = 0;
    while (pixel < THRESH){
        take_picture();
        pixel = get_pixel(160,120,3);
        move(left,right);
    }
}

//CODE for just Proportional Signal
// - This code takes multiple parameters
// - line is which line to read (robot only moves if line is 160)
// - kp is just the proportional constant that can be changed (As quadrant three has sharp turns, this number needs to change often)
// - base_speed is just to balance with the kp changes
// - This method returns range
bool pd(int line, double kp, double kd, int base_speed, int quadrant) {
    double max = 0;
    double min = 999;
    for (int i=0; i < 320; i++){
        if (get_pixel(i,line,3) > THRESH){
            if (i > max)max = i;
            if (i < min)min = i;
        }
    }
    bool white = false;
    for (int i=0; i < 320; i++){
        if (get_pixel(i,40,3) > THRESH){
            white = true;
        }
    }
    double range = max-min; //(MAX RANGE SHOULD BE 320, MIN SHOULD BE 0)
    double error_signal = (min+(range*0.5))/160 - 1; //error signal goes from -1 to 1 where 0 is the middle.
    double proportional_signal = error_signal * kp;
    double left_speed = base_speed;
    double right_speed = base_speed;
    if (previous_signal != -2){
        double derivative_signal = abs(previous_signal - error_signal) * kd;
        if (proportional_signal > 0)right_speed = right_speed - proportional_signal + derivative_signal;
        if (proportional_signal < 0)left_speed = left_speed + proportional_signal + derivative_signal;
    }
    switch (quadrant) {
        case 3:
            if (range == 0){
                turn(1)
            } else if (white){
                move(left_speed, right_speed);   
            } else if (range < 300){
                if (proportional_signal > 0)turn(2);
                if (proportional_signal < 0)turn(1);
            } else {
                turn(1);
            }
            break;
        case 2:
            if (range == 0)back();
            move(left_speed, right_speed);
    }
    
    previous_signal = error_signal;
    
    return range;
}

void back() {
    move(BASE_BACK_SPEED,BASE_BACK_SPEED);
    Sleep(0,300000);
}
//This method we got off the web, it stops the program when ctrl+c is pressed.
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
    move(70,70);
    Sleep(0,200000);
    int same_count = 0;
    double prev_signal = 0;
    int count = 0;
    int prevTotal = 0;
    while (1){
        int left_signal = read_analog(1);
        int right_signal = read_analog(0);
        int base_speed = 45;
        double constant = 0.075;
        /*printf("Left %d\n",left_signal);
        printf("Right %d\n",right_signal);*/
        double actual_signal = left_signal - right_signal;
        actual_signal *= constant;
        actual_signal += 4.8;
        printf("Actual %f\n",actual_signal);
        if (actual_signal > 20 || actual_signal < -20){
            base_speed = 35;
        } else {
            base_speed = 45;
        }
        move(base_speed + actual_signal, (base_speed - actual_signal)*1.3);
    }
}







