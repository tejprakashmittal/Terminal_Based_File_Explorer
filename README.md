# File Explorer -

This application will work in two modes :

1. `Normal Mode` : used to explore the current directory and navigate the filesystem.

2. `Command Mode` : used to enter and execute shell commands.

The root of the application is the directory where the application will start.

 
## How to execute

Open terminal and enter following commands :

1. For Compilation : g++ main.cpp
2. For Execute : ./a.out

## How to switch between Two Modes

Press `:` to switch from Normal Mode to Command Mode. Press `Esc` to switch from Command Mode to Normal Mode and `q` to exit from the application.

## Command Support

1. `Copy` - copy <source_file(s)> <destination_directory>

2. `Move` - move <source_file(s)> <destination_directory>

3. `Rename` - rename <old_filename> <new_filename>

4. `Create File` - create_file <file_name> <destination_path>

5. `Create Directory` - create_dir <dir_name> <destination_path>

6. `Delete File` - delete_file <file_path>

7. `Delete Directory` - delete_dir <dir_path>

8. `Goto` - goto <location>

9. `Search` - search <file_name> or search <directory_name>


## Assumptions
#### Source or Destination path assumptions
1. `~/` is path from root(where the application is started).
2. `./` represents the path from current directory.
3. `/` represents the absolute path (or whole path from home directory)