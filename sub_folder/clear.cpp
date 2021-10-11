#include<iostream>
using namespace std;

void gotoxy(int x,int y)    
{
    printf("%c[%d;%df",0x1B,y,x);
}
int main(){
//cout << "\033[H\033[2J\033[3J";
gotoxy(0,0);
char c;
cin>>c;
}
