#include<iostream>
#include<termios.h>
#include<unistd.h>
#include <dirent.h>
#include<bits/stdc++.h>
#include <cstring>
#include <pwd.h>
#include <grp.h>
#include<fcntl.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include<sstream>
using namespace std;

void refresh_screen(int);
void goto_parent(string);
void createDir(string,string);
void copy_dir(string,string);
void move_dir(string,string);
void commandMode();
void normal_mode_start(string);
int filesToDisplay();

stack<string> left_stac;
stack<string> right_stac;
vector<string> file_name_list;
vector<string> file_size_list;
vector<string> ownnership_list;
vector<string> last_modified_list;
vector<string> permision_list;
vector<string> cmd_list_str;

string cmd_str="";

struct winsize terminalWindow;
struct termios org;

bool cmd_mode=false;
unsigned int x=0,y=0,start_ptr=0,end_ptr=0,cursor_track=0,totalFiles_cur_dir=0,row_num=0,col_num=0,window;
string home_dir="";
string current_directory="";

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
  x=1;
  fflush(stdout);
}

string permissions(string file){
    struct stat st;
    char modeval[11];
    if(stat(file.c_str(), &st) == 0){
        mode_t perm = st.st_mode;
        modeval[0] = S_ISDIR(st.st_mode) ? 'd' : '-';
        modeval[1] = (perm & S_IRUSR) ? 'r' : '-';
        modeval[2] = (perm & S_IWUSR) ? 'w' : '-';
        modeval[3] = (perm & S_IXUSR) ? 'x' : '-';
        modeval[4] = (perm & S_IRGRP) ? 'r' : '-';
        modeval[5] = (perm & S_IWGRP) ? 'w' : '-';
        modeval[6] = (perm & S_IXGRP) ? 'x' : '-';
        modeval[7] = (perm & S_IROTH) ? 'r' : '-';
        modeval[8] = (perm & S_IWOTH) ? 'w' : '-';
        modeval[9] = (perm & S_IXOTH) ? 'x' : '-';
        modeval[10] = '\0';
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

string GetFileSize(string filename)   // In bytes
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    if(rc ==0 ){
      long long siz=stat_buf.st_size;
      string converted_size="";
      if(siz >= 1073741824){
         siz/=1073741824;
         converted_size += to_string(siz) + "G";
      }
      else if(siz >= 1048576){
         siz/=1048576;
         converted_size += to_string(siz) + "M";
      }
      else if(siz >= 1024){
         siz/=1024;
         converted_size += to_string(siz) + "K";
      }
      else{
         converted_size += to_string(siz) + "";
      }
      return converted_size;
    }
    else return "-1";
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
  //int len=file_name_list.size();

  for(int i=start_ptr;i<=end_ptr;i++){
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
            //left_stac.push(tmp_str);
            left_stac.push(str);
            //right_stac.push(str);
            normal_mode_start(tmp_str);
         }
    }
}

void normal_mode_start(string str)
{
  //terminal_resize();
  current_directory=str;
  clear_scr();
  cursor_point(1,1);
  clear_meta_vectors();
  //initEditor();
  getCurDirFiles(str);
  totalFiles_cur_dir=file_name_list.size();
  start_ptr=0;end_ptr=filesToDisplay()-1;
  display_cur_dir_files();
  cursor_point(1,1);
  x=1;y=1;
  cursor_track=0;
  char c;
  while(read(STDIN_FILENO,&c,1)!=0){
    if (c == '\x1b') {
          char seq[3];
          if (read(STDIN_FILENO, &seq[0], 1) != 1);
          if (read(STDIN_FILENO, &seq[1], 1) != 1);
          if (seq[0] == '[') {
            switch (seq[1]) {         /*Arrow Keys Implementation*/
              case 'A': if(x>1) {x--;cursor_track--;};cursor_point(x,y);break;
              case 'B': if(x<filesToDisplay()) {x++;cursor_track++;cursor_point(x,y);}break;
              case 'C': if(right_stac.empty()==false){
                          string temp_s=right_stac.top();
                          right_stac.pop();
                          //left_stac.push(temp_s);
                          left_stac.push(str);
                          normal_mode_start(temp_s);
                        }
                        break;
              case 'D': if(left_stac.empty()==false){    
                          string temp_s=left_stac.top();
                          left_stac.pop();
                          // right_stac.push(temp_s);
                          right_stac.push(str);
                          normal_mode_start(temp_s);
                        }
                        break;
            }
          }
        } 
        else {
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
                          //left_stac.push(temp_str);
                          left_stac.push(str);
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
          else if(c=='h'){      //Go to home
            left_stac.push(home_dir);
            right_stac.push(str);
            normal_mode_start(home_dir);
          }
          else if(c==127){
            goto_parent(str);
          }
          else if(c=='q'){     //Exit
            write(STDOUT_FILENO, "\x1b[2J", 4);
            cursor_point(1,1);
            exit(1);
          }
          else if(c=='k'){
            if(start_ptr > 0){
              clear_scr();
              start_ptr--;
              end_ptr--;
              //cursor_point(0,0);
              cursor_track=start_ptr;
              display_cur_dir_files();
            }
          }
          else if(c=='l'){
            if(end_ptr+1 < totalFiles_cur_dir){
              clear_scr();
              start_ptr++;
              end_ptr++;
              //cursor_point(0,0);
              cursor_track=start_ptr;
              display_cur_dir_files();
            }
          }
          else if(c==':'){
            commandMode();
            cmd_mode=false;
            x=1;y=1;cursor_track=1;
            cursor_point(1,1);
            normal_mode_start(current_directory);
          }
          fflush(0);
    }
  }
}

int filesToDisplay()
{
    int count;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminalWindow);
    row_num= terminalWindow.ws_row - 2;
    col_num=terminalWindow.ws_col;
    if (totalFiles_cur_dir <= row_num)
    {
        count = totalFiles_cur_dir;
    }
    else
    {
        count = row_num;
    }
    return count;
}

/*Command Mode--------------------------------------------------------------------------------*/

void split_command(){
  istringstream ss(cmd_str);
  string word;
  while(ss >> word){
    cmd_list_str.push_back(word);
  }
}

string convert_abs_path(string str)
{
    string abs="";
    char firstchar = str[0];
    
    if(firstchar=='.')
    {
        abs = current_directory + str.substr(1,str.length());    
    }
    else if(firstchar=='~')
    {
        abs = home_dir + str.substr(1,str.length());
    }
    else if(firstchar =='/')
    {
        abs = home_dir + str;
    }
    else
    {
        abs= current_directory + "/" + str;
    }
    return abs;
}

void copy_command(string source_path,string dest_path){
  //cout<<"This is source path "<<source_path<<endl;
  //cout<<"This is dest path "<<dest_path<<endl;
  //string dest_path=convert_abs_path(cmd_list_str[cmd_list_str.size()-1]);
  char b[1024];
  int read_count,source,dest;
  string filename=source_path;
  if(filename.find_last_of("/")!=string::npos){
    filename=filename.substr(filename.find_last_of("/")+1);
  }
  string dest_full_path=dest_path+"/"+filename;
  // FILE* source = fopen(source_path.c_str(), "rb");
  // FILE* dest = fopen(dest_full_path.c_str(), "wb");

  source = open(source_path.c_str(), O_RDONLY);
  dest = open(dest_full_path.c_str(), O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);

  if (source == -1) {
    cout<<source_path<<endl;
        perror("Error_open_source_path");
        return;
    }
    if (dest == -1) {
      fflush(stdout);
      //cout<<dest_full_path<<endl;
        perror("Error_open_dest_path");
        return;
    }

  char c;
  // while((c=getc(source))!=EOF){
  //   putc(c,dest);
  // }

  while((read_count = read(source,b,sizeof(b)))>0){
		write(dest,b,read_count);
	}

  struct stat source_File_stat;
  stat(source_path.c_str(), &source_File_stat);
  chown(dest_full_path.c_str(), source_File_stat.st_uid, source_File_stat.st_gid);
  chmod(dest_full_path.c_str(), source_File_stat.st_mode);
  close(source);
  close(dest);
}

void copy_mutiple(){
  string dest=convert_abs_path(cmd_list_str[cmd_list_str.size()-1]);
  for(int i=1;i<cmd_list_str.size()-1;i++){
    if(isDirectory(convert_abs_path(cmd_list_str[i])))
    copy_dir(convert_abs_path(cmd_list_str[i]),dest);
    else
    copy_command(convert_abs_path(cmd_list_str[i]),dest);
  }
}

bool bfs_helper_copy(queue<pair<string,string>> &que,string source,string dest){
  string filename=source;
  if(filename.find_last_of("/")!=string::npos){
     filename=filename.substr(filename.find_last_of("/")+1);
   }
   createDir(filename,dest);
   dest=dest+"/"+filename;
  bool flag=true;
  for(int i=0;i<file_name_list.size();i++){
    if(file_name_list[i]=="." || file_name_list[i]=="..") continue;
    string temp_str=source+'/'+ file_name_list[i];
    // cout<<temp_str;
    // fflush(stdout);
    if(isDirectory(temp_str)){
      que.push({temp_str,dest});
      flag=false;
    }
    else{
      // cout<<"This is copy_command "<<temp_str<<endl;
      // fflush(stdout);
      copy_command(temp_str,dest);
    }
  }
  return flag;
}

void bfs_copy(string source,string dest){
  queue<pair<string,string>> que;
  getCurDirFiles(source);
  if(bfs_helper_copy(que,source,dest)) return;
  while(que.empty()==false){
    string temp=que.front().first;
    string temp2=que.front().second;
    que.pop();
    file_name_list.clear();
    getCurDirFiles(temp);
    current_directory=temp;
    bfs_helper_copy(que,temp,temp2);
  }
}

void copy_dir(string source,string dest){
  file_name_list.clear();
  string temp=current_directory;
  bfs_copy(source,dest);
  current_directory=temp;
}

void delete_command(string file_path){
  //string file_path=convert_abs_path(cmd_list_str[cmd_list_str.size()-1]);
  if( remove(file_path.c_str()) != 0 )
    perror( "Error deleting file" );
}

bool bfs_helper(queue<string> &que,string filename){
  for(int i=0;i<file_name_list.size();i++){
    if(file_name_list[i]=="." || file_name_list[i]=="..") continue;
    if(file_name_list[i]==filename) return true;
    string temp_str=current_directory+'/'+file_name_list[i];
    if(isDirectory(temp_str)){
      que.push(temp_str);
    }
  }
  return false;
}

bool bfs(string filename){
  queue<string> que;
  if(bfs_helper(que,filename)) return true;
  while(que.empty()==false){
    string temp=que.front();
    que.pop();
    getCurDirFiles(temp);
    current_directory=temp;
    if(bfs_helper(que,filename)) return true;
  }
 return false;
}

/*Delete the single directory-----------------------------*/

bool bfs_helper_del(queue<string> &que,string dirpath){
  bool flag=true;
  for(int i=0;i<file_name_list.size();i++){
    if(file_name_list[i]=="." || file_name_list[i]=="..") continue;
    string temp_str=dirpath+'/'+ file_name_list[i];
    // cout<<temp_str;
    // fflush(stdout);
    if(isDirectory(temp_str)){
      que.push(temp_str);
      flag=false;
    }
    else{
      if( remove(temp_str.c_str()) != 0 )
      perror( "Error deleting file" );
    }
  }
  return flag;
}

void bfs_del(string dirpath){
  queue<string> que;
  stack<string> nexted_dir_stac;
  getCurDirFiles(dirpath);
  if(bfs_helper_del(que,dirpath)){
    if( remove(dirpath.c_str()) != 0 )
      perror( "Error deleting directory first case" );
      return;
  }
  nexted_dir_stac.push(dirpath);
  while(que.empty()==false){
    string temp=que.front();
    nexted_dir_stac.push(temp);
    que.pop();
    file_name_list.clear();
    getCurDirFiles(temp);
    if(bfs_helper_del(que,temp)){
      if( remove(temp.c_str()) != 0 )
      perror( "Error deleting directory" );
      nexted_dir_stac.pop();
    }
  }
  while(nexted_dir_stac.empty()==false){
    remove((nexted_dir_stac.top()).c_str());
    nexted_dir_stac.pop();
  }
}

void delete_dir_command(string dir_path){
   file_name_list.clear();
   bfs_del(dir_path);
}

void createFile(string filename,string dest_path){
  dest_path+="/"+filename;
  int fd=open(dest_path.c_str(),O_RDONLY | O_CREAT,0644);
  if(fd==-1){
    cout<<endl;
    cout<<"Error in Creating File";
  }
  else close(fd);
}

void createDir(string dirname,string dest_path){
  dest_path+="/"+dirname;
  int status=mkdir(dest_path.c_str(),0777);
  if(status==-1){
    cout<<endl;
    cout<<"Error in Creating File";
  }
}

void renameFile(string file1,string file2){
  int status=rename(file1.c_str(),file2.c_str());
  if(status==-1){
    cout<<endl;
    cout<<"Error in renaming File";
  }
}

void move_single_command(string source,string dest){
  string filename=source;
  // if(filename.find_last_of("/")!=string::npos){
  //   filename=filename.substr(filename.find_last_of("/")+1);
  // }
  copy_command(filename,dest);
  delete_command(source);
}

void move_multiple(){
  string dest=convert_abs_path(cmd_list_str[cmd_list_str.size()-1]);
  for(int i=1;i<cmd_list_str.size()-1;i++){
    if(isDirectory(convert_abs_path(cmd_list_str[i])))
    move_dir(convert_abs_path(cmd_list_str[i]),dest);
    else
    move_single_command(convert_abs_path(cmd_list_str[i]),dest);
  }
}

void move_dir(string source_path,string dest_path){
  copy_dir(source_path,dest_path);
  delete_dir_command(source_path);
}

void commandMode(){
  cmd_mode=true;
  x=terminalWindow.ws_row - 1;
  y=1;
  cursor_point(x,y);
  printf(":");
  y++;
  cursor_point(x,y);
  fflush(stdout);
  char seq[3];
  memset(seq, 0, 3 * sizeof(seq[0]));
  for(;;)
  {
    if (read(STDIN_FILENO, seq, 3) == 0)
        continue;

    if(seq[0]==27 && seq[1]=='[' && (seq[2]=='A' || seq[2]=='B' || seq[2]=='C'|| seq[2]=='D')) continue;
    if(seq[0]==27 && seq[1]==0 && seq[2]==0){
      return;
    }
    if(seq[0]=='q' && seq[1]==0 && seq[2]==0 && cmd_str==""){
      write(STDOUT_FILENO, "\x1b[2J", 4);
      cursor_point(1,1);
      exit(1);
    }
    if(seq[0]==10){    //detecting Enter Key
      split_command();
      cmd_str="";
      if(cmd_list_str[0]=="copy"){
        if(cmd_list_str.size()==3)
        {
          if(isDirectory(convert_abs_path(cmd_list_str[1])))
          copy_dir(convert_abs_path(cmd_list_str[1]),convert_abs_path(cmd_list_str[cmd_list_str.size()-1]));
          else
          copy_command(convert_abs_path(cmd_list_str[1]),convert_abs_path(cmd_list_str[cmd_list_str.size()-1]));
        }
        else copy_mutiple();
        cmd_list_str.clear();
        printf("\x1b[2K");
        fflush(stdout);
        y=1;
        cursor_point(x,y);
        printf(":");
        y++;
        cursor_point(x,y);
      }
      else if(cmd_list_str[0]=="move"){
        if(cmd_list_str.size()==3)
        {
          if(isDirectory(convert_abs_path(cmd_list_str[1])))
          move_dir(convert_abs_path(cmd_list_str[1]),convert_abs_path(cmd_list_str[cmd_list_str.size()-1]));
          else
          move_single_command(convert_abs_path(cmd_list_str[1]),convert_abs_path(cmd_list_str[cmd_list_str.size()-1]));
        }
        else move_multiple();
        cmd_list_str.clear();
        printf("\x1b[2K");
        fflush(stdout);
        y=1;
        cursor_point(x,y);
        printf(":");
        y++;
        cursor_point(x,y);
      }
      else if(cmd_list_str[0]=="rename"){
        renameFile(convert_abs_path(cmd_list_str[1]),convert_abs_path(cmd_list_str[cmd_list_str.size()-1]));
        cmd_list_str.clear();
        cmd_list_str.clear();
        printf("\x1b[2K");
        fflush(stdout);
        y=1;
        cursor_point(x,y);
        printf(":");
        y++;
        cursor_point(x,y);
      }
      else if(cmd_list_str[0]=="create_file"){
        createFile(cmd_list_str[1],convert_abs_path(cmd_list_str[cmd_list_str.size()-1]));
        cmd_list_str.clear();
        printf("\x1b[2K");
        fflush(stdout);
        y=1;
        cursor_point(x,y);
        printf(":");
        y++;
        cursor_point(x,y);
      }
      else if(cmd_list_str[0]=="create_dir"){
        createDir(cmd_list_str[1],convert_abs_path(cmd_list_str[cmd_list_str.size()-1]));
        cmd_list_str.clear();
        printf("\x1b[2K");
        fflush(stdout);
        y=1;
        cursor_point(x,y);
        printf(":");
        y++;
        cursor_point(x,y);
      }
      else if(cmd_list_str[0]=="delete_file"){
        delete_command(convert_abs_path(cmd_list_str[cmd_list_str.size()-1]));
        cmd_list_str.clear();
        printf("\x1b[2K");
        fflush(stdout);
        y=1;
        cursor_point(x,y);
        printf(":");
        y++;
        cursor_point(x,y);
      }
      else if(cmd_list_str[0]=="delete_dir"){
        delete_dir_command(convert_abs_path(cmd_list_str[1]));
        cmd_list_str.clear();
        printf("\x1b[2K");
        fflush(stdout);
        y=1;
        cursor_point(x,y);
        printf(":");
        y++;
        cursor_point(x,y);
      }
      else if(cmd_list_str[0]=="goto"){
        current_directory=convert_abs_path(cmd_list_str[cmd_list_str.size()-1]);
        cmd_list_str.clear();
        printf("\x1b[2K");
        fflush(stdout);
        y=1;
        cursor_point(x,y);
        printf(":");
        y++;
        cursor_point(x,y);
      }
      else if(cmd_list_str[0]=="search"){
          string c_dir_temp=current_directory;
          string res="";
          if(bfs(cmd_list_str[1])){
            res="True";
          }
          else{
            res="False";
          }
          current_directory=c_dir_temp;
          // printf("\x1b[2K");
          // fflush(stdout);
          cmd_list_str.clear();
          printf("\x1b[2K");
          y=1;
          cursor_point(x,y);
          printf(":");
          cout<<res;
          // y=6;
          // cursor_point(x,y);
           fflush(stdout);
           y=6;
          cursor_point(x,y);
      }
    }
    else if(seq[0]==127){
      if(y>2){
        y--;
        cursor_point(x,y);
        printf("\x1b[0K");
        if(cmd_str!="")
        cmd_str.pop_back();
      }
    }
    else{
      cout<<seq[0];
      fflush(stdout);
      y++;
      cursor_point(x,y);
      cmd_str+=seq[0];
    }
    fflush(stdout);
    // x=1;y=1;cursor_track=1;
    // cursor_point(1,1);
    memset(seq, 0, 3 * sizeof(seq[0]));
  }
}

void refresh_screen(int signal)
{
  clear_scr();
  cursor_point(1,1);
  clear_meta_vectors();
  //initEditor();
  getCurDirFiles(current_directory);
  totalFiles_cur_dir=file_name_list.size();
  start_ptr=0;end_ptr=filesToDisplay()-1;
  display_cur_dir_files();
  cursor_point(1,1);
  x=1;y=1;
  cursor_track=0;

  if(cmd_mode==true) commandMode();

}

int main(){
  enableit();
  clear_scr();
  //terminal_resize();
  cursor_point(1,1);
  char ch[256];
  getcwd(ch,256);
  home_dir=string(ch);
  left_stac.push(string(ch));
  signal(SIGWINCH,refresh_screen);      //Handling terminal resize
  normal_mode_start(left_stac.top());
  //cout<<content_list.size();
	clear_scr();
  return 0;
}
