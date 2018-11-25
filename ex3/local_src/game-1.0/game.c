#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <asm-generic/fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include "game.h" //Contains game parameters

FILE* gamepad;
int buttons;

unsigned int p_one_pos = (HEIGHT-PADDLE_LEN)/2; //The initial position of player one (left)
unsigned int p_two_pos = (HEIGHT-PADDLE_LEN)/2; //The initial position of player two (right)
unsigned int ball_x = WIDTH/2; //Place the ball in the middle of the board
unsigned int ball_y = 0;
signed int ball_x_dir = -1; //The initial x-direction of the ball (left)
signed int ball_y_dir = 1; //The initial y-direction of the ball (down)

int screenDriver;
unsigned short* screenBuffer;

unsigned int p_one_pts = 0;
unsigned int p_two_pts = 0;

void clear_screen(){
	struct fb_copyarea area;
	area.dx = 0;
	area.dy = 0;
	area.width = 320;
	area.height = 240;

	unsigned int i;
	for(i=0; i<2*38400; i++){
		screenBuffer[i] = 0x0000;
	}
	ioctl(screenDriver,0x4680,&area);
}

void move_player(int c){
	switch(c){
		case 2: //UP (Player 1)
			if(p_one_pos<PADDLE_STEP){
				p_one_pos=0;
			}
			else{
				p_one_pos-=PADDLE_STEP;
			}
			break;
		case 32: //UP (Player 2)
			if(p_two_pos<PADDLE_STEP){
				p_two_pos=0;
			}
			else{
				p_two_pos-=PADDLE_STEP;
			}
			break;
		case 8: //DOWN (Player 1)
			if((p_one_pos+PADDLE_STEP)>(HEIGHT-PADDLE_LEN)){
				p_one_pos=HEIGHT-PADDLE_LEN;
			}
			else{
				p_one_pos+=PADDLE_STEP;
			}
			break;
		case 128: //DOWN (Player 2)
			if((p_two_pos+PADDLE_STEP)>(HEIGHT-PADDLE_LEN)){
				p_two_pos=HEIGHT-PADDLE_LEN;
			}
			else{
				p_two_pos+=PADDLE_STEP;
			}
			break;
		default:
			break;
	}
}

void move_ball(){
	if(ball_x==PADDLE_WIDTH){ //The ball is close to the left side
		if(p_one_pos<=ball_y+BALL_SIZE-1 && p_one_pos+PADDLE_LEN>ball_y){ //The ball hits the left paddle
			ball_x_dir *= -1;
		}
	}
	if(ball_x==WIDTH-BALL_SIZE-PADDLE_WIDTH){ //The ball is close to the right side
		if(p_two_pos<=ball_y+BALL_SIZE-1 && p_two_pos+PADDLE_LEN>ball_y){ //The ball hits the right paddle
			ball_x_dir *= -1;
		}
	}
	if(ball_y==0 || ball_y+BALL_SIZE==HEIGHT){ //The ball hits the floor or ceiling
		ball_y_dir *= -1;
	}
	if(ball_x==WIDTH-1){ //The ball hits the right wall
		p_one_pts+=1; //Give player one a point
		ball_x=WIDTH/2; //Place the ball in the middle of the board...
		ball_y=rand()%HEIGHT; //...at a random height...
		if(rand()%2){ //...and random x-direction (left/right)
			ball_x_dir = -1;
		}
		else{
			ball_x_dir = 1;
		}
		return;
	}
	if(ball_x==0){ //The ball hits the left wall
		p_two_pts+=1; //Give player two a point
		ball_x=WIDTH/2; //Place the ball in the middle of the board...
		ball_y=rand()%HEIGHT; //...at a random height...
		if(rand()%2){ //...and random x-direction (left/right)
			ball_x_dir = -1;
		}
		else{
			ball_x_dir = 1;
		}
		return;
	}
	ball_x += ball_x_dir; //Move ball one pixel in x-direction
	ball_y += ball_y_dir; //Move ball one pixel in y-direction
}

void game_tick(){ //New game state, every tick
	remove_ball();
	move_ball();
	draw_game();
}

void draw_board(){
	struct fb_copyarea area;
	area.dx=WIDTH/2;
	area.dy=0;
	area.height = HEIGHT;
	area.width = 1;
	for(int i=0; i<HEIGHT; i++){ //The line in the middle of the board
		screenBuffer[i*320+WIDTH/2]=0xffff;
	}
	ioctl(screenDriver, 0x4680, &area);
}

void draw_paddles(){
	struct fb_copyarea area;
	area.dx=0;
	area.dy=0;
	area.height = HEIGHT;
	area.width = PADDLE_WIDTH;
	for(int i=0;i<HEIGHT;i++){ //Clear the left side
		for(int j=0; j<PADDLE_WIDTH; j++){
			screenBuffer[i*WIDTH+j]=0x0000;
		}
	}
	for(int i=0;i<PADDLE_LEN;i++){ //Draw the left paddle
		for(int j=0; j<PADDLE_WIDTH; j++){
			screenBuffer[(p_one_pos+i)*WIDTH+j]=0xf800;
		}
	}
	ioctl(screenDriver, 0x4680, &area);
	area.dx=WIDTH-PADDLE_WIDTH;
	for(int i=0;i<HEIGHT;i++){ //Clear the right side
		for(int j=0; j<PADDLE_WIDTH; j++){
			screenBuffer[i*WIDTH+WIDTH-j]=0x0000;
		}
	}
	for(int i=0;i<PADDLE_LEN;i++){ //Draw the right paddle
		for(int j=0; j<PADDLE_WIDTH; j++){
			screenBuffer[(p_two_pos+i)*WIDTH+WIDTH-j]=0xffff;
		}
	}
	ioctl(screenDriver, 0x4680, &area);
}

void remove_ball(){
	struct fb_copyarea area;
	area.dx=ball_x;
	area.dy=ball_y;
	area.height = BALL_SIZE;
	area.width = BALL_SIZE;
	for(int i=0;i<BALL_SIZE;i++){
		for(int j=0;j<BALL_SIZE;j++){
			screenBuffer[(ball_y+i)*320+ball_x+j]=0x0000;
		}
	}
	ioctl(screenDriver, 0x4680, &area);
}
void draw_ball(){
	struct fb_copyarea area;
	area.dx=ball_x;
	area.dy=ball_y;
	area.height = BALL_SIZE;
	area.width = BALL_SIZE;
	for(int i=0;i<BALL_SIZE;i++){ //Draw ball
		for(int j=0;j<BALL_SIZE;j++){
			screenBuffer[(ball_y+i)*320+ball_x+j]=0x04ff;
		}
	}
	ioctl(screenDriver, 0x4680, &area);
}

void draw_game(){
	draw_board();
	draw_paddles();
	draw_ball();
}


int receive_input(int signo) {
	read(gamepad,&buttons,sizeof(buttons)); //Read which button was pressed
	move_player(buttons); //Move players based on button pressed
	return 0;
}

void init_graphics(){
	screenDriver = open("/dev/fb0",O_RDWR);
	if(screenDriver == -1){
		printf("Error: cannot open framebuffer device");
		exit(1);
	}
	screenBuffer = (unsigned short*) mmap(NULL, 2*320*240, PROT_WRITE | PROT_READ, MAP_SHARED, screenDriver, 0);
	if((int) screenBuffer == -1){
		printf("Error: failed to map framebuffer device to memory");
		exit(4);
	}

	clear_screen();
}

int main(int argc, char *argv[]) {
	init_graphics();
	gamepad = open("/dev/gamepad", O_RDONLY);
	int fno = fileno(fopen("/dev/gamepad", "r"));

	//set up sigaction
	struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_handler = receive_input; // set function to run when signal is received
	action.sa_flags = 0; // no relevant flags
	sigaction(SIGIO, &action, NULL); // run interrupt on SIGIO

	// Set ownership of gamepad to this program.
	if (fcntl(fno, F_SETOWN, getpid()) == -1) {
		printf("Error in setting ownership of gamepad.");
		return EXIT_FAILURE;
	}
	if (fcntl(fno, F_SETFL, fcntl(fno, F_GETFL) | FASYNC) == -1) {
		printf("Error in setting FASYNC flag in gamepad.");
		return EXIT_FAILURE;
	}
	ball_y = rand() % HEIGHT; //Place ball at random height initially
	while (1) {
		game_tick();
		usleep(100000/SPEED);
	}
	return EXIT_SUCCESS;
}
