#include <stdio.h>

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

int v_left = 0;
int v_right = 0;
int num = 16;
#define NP 16
void move(int step, int direction);
int findPid();
void drawBW();

int main()
{
    init(0);
    bool white;
    int thresh = 100;
    // connect camera to the screen
    open_screen_stream();
    // set all digital outputs to +5V
    //for (i = 0; i < 8; i++)
    //{
      // set all digital channels as outputs
     // select_IO(i,0);
     // write_digital(i,0);
    //int wh[NP];
    //for (int g = 0; g < 20; g++)
    //{
       	//int avg = 0;
       	for(int i=0; i < num ; i++)
       	{
        	//wh[i]= get_pixel(100+i  , 120 ,3);
          	//avg += wh[i];
         	 //printf("%d ",wh[i]);
       	//}
      	//if(avg/NP >thresh) {
        	//printf("true");
     	//}else {
     	//printf("false");
     	//}
      	//printf("\n");
       	drawBW();
	// display picture
       	//update_screen();
	// check motors
       	//v_left = 127;
       	//v_right = 195;
       	//set_motor(2,v_right);
       	//set_motor(1,v_left);
       	//move();
	//Sleep(1,0);
	v_left = 0;
       	v_right = 0;
       	//set_motor(1,v_right);
       	//set_motor(2,v_left);
       	//Sleep(1,0);
      	// for (i = 0 ; i < 8; i++)
      	// {
      	//  int av = read_analog(i);
      	//  printf("ai=%d av=%d\n",i,av);
      	}
     	//}
// terminate hardware
close_screen_stream();
set_motor(1,0);
set_motor(2,0);
return 0;
}

void drawBW()
{
	take_picture();
	for (int x = 150; x < 230; x++){
		//for (int y = 1; y < 240; y++){
			update_screen();
			int y = 120;
		int average = get_pixel(x,y,3);
		if (average > 127){
			set_pixel(x,y,255,255,255);
		} else {
			set_pixel(x,y,0,0,0);
		}
		//}
	}
}

void move(int step, int direction)
{
	set_motor(2,(159*step)+direction);//right
	set_motor(1,(127*step)+direction);//left
}

// We will need to set this up soon.
int findPid()
{
	//code to find PID
	return 0;
}
