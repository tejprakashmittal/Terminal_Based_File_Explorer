#include<iostream>
#include<termios.h>
#include<unistd.h>
#include <dirent.h>
#include<bits/stdc++.h>
#include <cstring>
#include <pwd.h>
#include <grp.h>
#include <sys/wait.h>
#include <sys/stat.h>
using namespace std;

void goto_parent(string);
void normal_mode_start(string);

stack<string> left_stac;
stack<string> right_stac;
vector<string> file_name_list;
vector<long> file_size_list;
vector<string> ownnership_list;
vector<string> last_modified_list;
vector<string> permision_list;

int x=0,y=0,cursor_track=0;
string home_dir="";

void cursor_point(int x,int y)    
{
    //printf("%c[%d;%df",0x1B,x,y);
    printf("\033[%d;%dH",x,y);
    fflush(stdout);
}

void terminal_resize(){
  cout << "\e[8;80;150t";
  fflush(stdout);
}

void moveCursor(char key) {
  switch (key) {
    case 'w':
      if(x>0) {x--;cursor_track--;}
      break;
    case 's':
      x++;
      cursor_track++;
      break;
    // case 'a':
    //   if(y>0) y--;
    //   break;
    // case 'd':
    //   y++;
    //   break;
  }
}

struct termios org;

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &org);
}

void enableit(){
   tcgetattr(STDIN_FILENO,&org);
   atexit(disableRawMode);
   struct termios raw=org;
   raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
   raw.c_iflag &= ~(BRKINT);
   tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void clear_scr()
{
  //cout << "\033[H\033[2J\033[3J";
  //printf("\033[3J\033[H\033[2J");
  cout<<"\033[3J\033[H\033[2J";
  fflush(stdout);
}

string permissions(string file){
    struct stat st;
    char modeval[10];
    if(stat(file.c_str(), &st) == 0){
        mode_t perm = st.st_mode;
        modeval[0] = (perm & S_IRUSR) ? 'r' : '-';
        modeval[1] = (perm & S_IWUSR) ? 'w' : '-';
        modeval[2] = (perm & S_IXUSR) ? 'x' : '-';
        modeval[3] = (perm & S_IRGRP) ? 'r' : '-';
        modeval[4] = (perm & S_IWGRP) ? 'w' : '-';
        modeval[5] = (perm & S_IXGRP) ? 'x' : '-';
        modeval[6] = (perm & S_IROTH) ? 'r' : '-';
        modeval[7] = (perm & S_IWOTH) ? 'w' : '-';
        modeval[8] = (perm & S_IXOTH) ? 'x' : '-';
        modeval[9] = '\0';
        return string(modeval);     
    }
    else{
        return strerror(errno);
    }   
}

string last_modification(string filename){
  struct stat result;
  if(stat(filename.c_str(), &result)==0)
  {
      auto mod_time = result.st_mtime;
      return ctime(&mod_time);
  }
}

long GetFileSize(string filename)   // In bytes
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

string getOwnership(string filename){
  struct stat info;
  string result="";
  stat(filename.c_str(), &info);  // Error check omitted
  struct passwd *pw = getpwuid(info.st_uid);
  struct group  *gr = getgrgid(info.st_gid);
  if(pw != 0) {
      result+=string(pw->pw_name);
      result+=" ";
    }
  if(gr != 0) result+=string(gr->gr_name);
  return result;
}

void getCurDirFiles(string str)
{
  //cout<<str<<endl;
  DIR* dir; dirent* pdir;
  const char *ch=str.c_str();
  string abs_path="";
  dir=opendir(ch);  
  if(dir == NULL){
        perror("opendir_error");
        return;
    }   
  //abs_path=str.substr(0, str.find_last_of("/"));
  abs_path=str+"/";
  while (pdir=readdir(dir)) {
    //cout<<abs_path+string(pdir->d_name)<<endl;
      file_name_list.push_back(string(pdir->d_name));
      permision_list.push_back(permissions(abs_path+string(pdir->d_name)));
      string temp_str=last_modification(abs_path+string(pdir->d_name));
      last_modified_list.push_back(temp_str.substr(0,temp_str.size()-1));
      file_size_list.push_back(GetFileSize(abs_path+string(pdir->d_name)));
      ownnership_list.push_back(getOwnership(abs_path+string(pdir->d_name)));
      //cout<<pdir->d_name<<endl;
  }
  closedir(dir);
}

void display_cur_dir_files(){
  int len=file_size_list.size();

  for(int i=0;i<len;i++){
    string temp_str=file_name_list[i];
    if(temp_str.length()>10){
      temp_str=temp_str.substr(0,10);
      temp_str+="...";
    }
    else{
      int temp=13;
      while(temp-- > temp_str.length()) temp_str+=' ';
    }
    cout<<temp_str<<"\t\t"<<file_size_list[i]<<"\t\t"<<ownnership_list[i]<<"\t\t"<<permision_list[i]<<"\t\t"<<last_modified_list[i]<<endl;
    fflush(stdout);
  }
}

void clear_meta_vectors(){
  file_name_list.clear();
  file_size_list.clear();
  ownnership_list.clear();
  ownnership_list.clear();
  last_modified_list.clear();
}

int isDirectory(string path) {
   struct stat statbuf;
   //cout<<cursor_track<<endl;
   //fflush(stdout);
   if (stat(path.c_str(), &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

void goto_parent(string str){
  int pos = str.find_last_of('/');
       if(pos!=string::npos){
         string tmp_str=str.substr(0,pos);
         if(tmp_str.size() >= home_dir.size())
         {
            left_stac.push(tmp_str);
            //right_stac.push(str);
            normal_mode_start(tmp_str);
         }
    }
}

void normal_mode_start(string str)
{
  //terminal_resize();
  clear_scr();
  cursor_point(0,0);
  clear_meta_vectors();
  //initEditor();
  getCurDirFiles(str);
  display_cur_dir_files();
  cursor_point(0,0);
  x=0;y=0;
  cursor_track=-1;
  char c;
  while(read(STDIN_FILENO,&c,1)!=0){
    //fflush(stdin);
     moveCursor(c);
     cursor_point(x,y);
     if(c==10){    //Enter key detect
        string current_dir=".";
        string parent_dir="..";
        if(file_name_list[cursor_track]==current_dir){}
        else{
            if(str==home_dir && file_name_list[cursor_track]==parent_dir){}
            else{
              // cout<<str<<endl;
              // fflush(stdout);
              if(file_name_list[cursor_track]==parent_dir) goto_parent(str);
              else{
                  string temp_str=str+'/'+file_name_list[cursor_track];
                  if(isDirectory(temp_str)){
                    left_stac.push(temp_str);
                    normal_mode_start(temp_str);
                  }
                  else{
                    pid_t pid = fork();
                    if (pid == 0) {
                        if(execl("/usr/bin/xdg-open","xdg-open",temp_str.c_str(),NULL)==-1) perror("Error in Exec");
                        // char* arguments[3] = { "vi", (char *)temp_str.c_str(), NULL };
                        // execvp("vi", arguments);
                        exit(1);
                      }
                    //else wait(0);
                  }
              }
            }
          }
     }
     else if(c=='a' && left_stac.empty()==false){    
       string temp_s=left_stac.top();
       left_stac.pop();
       right_stac.push(temp_s);
       normal_mode_start(temp_s);
     }
     else if(c=='h'){      //Go to home
       left_stac.push(home_dir);
       right_stac.push(str);
       normal_mode_start(home_dir);
     }
     else if(c=='d' && right_stac.empty()==false){
       string temp_s=right_stac.top();
       right_stac.pop();
       left_stac.push(temp_s);
       normal_mode_start(temp_s);
     }
     else if(c==127){
       goto_parent(str);
     }
     else if(c=='q'){     //Exit
       write(STDOUT_FILENO, "\x1b[2J", 4);
       cursor_point(0,0);
       exit(1);
     }
     fflush(0);
  }
}

int main(){
  enableit();
  clear_scr();
  terminal_resize();
  cursor_point(0,0);
  char ch[256];
  getcwd(ch,256);
  home_dir=string(ch);
  left_stac.push(string(ch));
  normal_mode_start(left_stac.top());
  //cout<<content_list.size();
	clear_scr();
  return 0;
}
