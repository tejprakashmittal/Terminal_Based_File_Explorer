#include <iostream>
#include <dirent.h>
#include <cstring>

using namespace std;

int main() {
   DIR* dir; dirent* pdir;
   //From my workspace
   dir=opendir(".");     
   while (pdir=readdir(dir)) {
      cout<<pdir->d_name<<endl;
       }
   closedir(dir);
   return 0;
}
