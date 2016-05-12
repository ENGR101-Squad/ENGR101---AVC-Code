extern "C" int init_hardware();
extern "C" int init(int d_lev);

extern "C" int set_motor(int motor,int speed);

int main() {
    for(int i = 0; i < 20; i++) {
        set_motor(0, 127);
        set_motor(1,127);
    }
}