#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define HEIGHT 30
#define WIDTH  100
#define BALL_SIZE 2
#define PADDLE_LEN 6
#define PADDLE_WIDTH 2

unsigned int pone_pos = (HEIGHT-PADDLE_LEN)/2;
unsigned int ptwo_pos = (HEIGHT-PADDLE_LEN)/2;
unsigned int ball_x = WIDTH/2;
unsigned int ball_y = 0;
signed int ball_x_dir = -1;
signed int ball_y_dir = 1;
char board[HEIGHT][WIDTH];

signed int pone_pts = 0;
signed int ptwo_pts = 0;

void clean_board(){
    for(int i=0; i<HEIGHT; i++){
	for(int j=0; j<WIDTH; j++){
	    if(j==WIDTH/2){
		board[i][j]='|';
	    }
	    else{
		board[i][j]=' ';
	    }
	}
    }
}

void move_player(char c){
    switch(c){
	case 'q': //UP (Player 1)
	    if(pone_pos>0) pone_pos-=1;
	    break;
	case 'w': //UP (Player 2)
	    if(ptwo_pos>0) ptwo_pos-=1;
	    break;
	case 'a': //DOWN (Player 1)
	    if(pone_pos<HEIGHT-PADDLE_LEN) pone_pos+=1;
	    break;
	case 's': //DOWN (Player 2)
	    if(ptwo_pos<HEIGHT-PADDLE_LEN) ptwo_pos+=1;
	    break;
	default:
	    break;
    }
    for(int i=0;i<PADDLE_LEN;i++){
	for(int j=0; j<PADDLE_WIDTH; j++){
	    board[pone_pos+i][j]='x';
	}
    }
    for(int i=0;i<PADDLE_LEN;i++){
	for(int j=0; j<PADDLE_WIDTH; j++){
	    board[ptwo_pos+i][WIDTH-1-j]='y';
	}
    }
}

void move_ball(){
    if(ball_x==PADDLE_WIDTH){
	if(pone_pos<=ball_y+BALL_SIZE-1 && pone_pos+PADDLE_LEN>ball_y){
	    ball_x_dir *= -1;
	}
    }
    if(ball_x==WIDTH-BALL_SIZE-PADDLE_WIDTH){
	if(ptwo_pos<=ball_y+BALL_SIZE-1 && ptwo_pos+PADDLE_LEN>ball_y){
	    ball_x_dir *= -1;
	}
    }
    if(ball_y==0 || ball_y+BALL_SIZE==HEIGHT){
	ball_y_dir *= -1;
    }
    if(ball_x==WIDTH-1){
	pone_pts+=1;
	ball_x=WIDTH/2;
	ball_y=rand()%HEIGHT;
	if(rand()%2){
	    ball_x_dir = -1;
	}
	else{
	    ball_x_dir = 1;
	}
	return;
    }
    if(ball_x==0){
	ptwo_pts+=1;
	ball_x=WIDTH/2;
	ball_y=rand()%HEIGHT;
	if(rand()%2){
	    ball_x_dir = -1;
	}
	else{
	    ball_x_dir = 1;
	}
	return;
    }
    ball_x += ball_x_dir;
    ball_y += ball_y_dir;
    for(int i=0;i<BALL_SIZE;i++){
	for(int j=0;j<BALL_SIZE;j++){
	    board[ball_y+i][ball_x+j]='o';
	}
    }
}

void generate_board(char c){
    clean_board();
    move_player(c);
    move_ball();
}
void draw_board(int redraw){
    if(redraw) printf("\033[%dA\r",HEIGHT+2);
    printf("*P1: ");
    for(int i=0; i<pone_pts; i++){
	printf("|");
    }
    for(int i=0; i<(WIDTH/2)-3-pone_pts; i++){
	printf("*");
    }
    printf("P2: ");
    for(int i=0; i<ptwo_pts; i++){
	printf("|");
    }
    for(int i=0; i<(WIDTH/2)-2-ptwo_pts; i++){
	printf("*");
    }
    printf("\n");
    for(int i=0; i<HEIGHT; i++){
	printf("*");
	for(int j=0; j<WIDTH; j++){
	    printf("%c",board[i][j]);
	}
	printf("*\n");
    }
    for(int i=0; i<WIDTH+2; i++){
	printf("*");
    }
    printf("\n      \r>");
}

int main(int argc, char *argv[]){
    ball_y = rand() % HEIGHT;
    generate_board(' ');
    draw_board(0);
    while(1){
	char c = getchar();
	if(c!='\r' && c!='\n'){ //Not enter key
	    generate_board(c);
	    draw_board(0);
	}
    }
}
