#include <termios.h> // meant to Turn the Console into raw mode 
#include <unistd.h> 

//enables Raw mode 
void enableRawMode(){
    struct termios raw;

    tcgetattr(STDIN_FILENO, &raw); //tcgetattr sets the terminals attributes and reads them into a struct 

    raw.c_lflag &= ~ (ECHO); //<- ECHO makes every key being repeated into the Terminal - Turned Off for ECHO to not get in the way 

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); // modified struct pased here 

}




//read() and STDIN_FILENO -> from unistd.h 
int main() {
    enableRawMode(); 

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q'); //read() -> reads 1 byte and puts it into the variable c 
    return 0;                               // while makes it do so until there is nothing to read anymore 
}