#include<unistd.h>
#include<iostream>
#include<bits/stdc++.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include<ctime>
using namespace std;

string permissions(char *file){
    struct stat st;
    char modeval[10];
    if(stat(file, &st) == 0){
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

long GetFileSize(string filename)
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

int isDirectory(string path) {
   struct stat statbuf;
   if (stat(path.c_str(), &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

int main()
{
  string str="";
  str+='.';
  cout<<str.length();
  // stack<string> left;
  // char ch[256];
  // getcwd(ch,256);
  // //cout<<permissions(ch);
  // cout<<getOwnership(string(ch));
  // cout<<"\t\t"<<"\033[1;32m"<<"Hello";
  // cout<<"world";
  // left.push(string(ch));
  // cout<<left.top();
  //cout<<isDirectory("/home/os-class/Desktop/aos/clear.cpp");
  // char c;
  // while(read(STDIN_FILENO,&c,1)!=0 && c!='q'){
  //   if(c==10) cout<<"yes";
  // }
  
  return 0;
}
