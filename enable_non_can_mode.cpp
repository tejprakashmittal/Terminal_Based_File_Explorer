#include<iostream>
#include<termios.h>
#include<unistd.h>
using namespace std;

void gotoxy(int x,int y)    
{
    printf("%c[%d;%df",0x1B,y,x);
}

struct termios org;

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &org);
}

void enableit(){
   tcgetattr(STDIN_FILENO,&org);
   atexit(disableRawMode);
   struct termios raw=org;
   raw.c_lflag &= ~(ICANON);
   tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(){
  enableit();
  gotoxy(10,20);
  char c;
  while(read(STDIN_FILENO,&c,1)!=0 && c!='q')
{
char ch='\n';
write(STDOUT_FILENO,&ch,1);
write(STDOUT_FILENO,&c,1);
write(STDOUT_FILENO,&ch,1);
}
	
  return 0;
}
