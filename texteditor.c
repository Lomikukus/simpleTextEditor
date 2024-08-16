#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h> // meant to Turn the Console into raw mode 
#include <unistd.h> 


struct termios orig_termios;

//disable Raw Mode 
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

//enables Raw mode 
void enableRawMode(){
                                                    //-> READS attributes from Terminal
    tcgetattr(STDIN_FILENO, &orig_termios);         //tcgetattr sets the terminals attributes and reads them into a struct  
    atexit(disableRawMode);                         //registers when programm is exited and disables raw mode 

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(ICRNL |IXON);                         //turns of ctrl q and s, used for dataflow/transmission controll XON XOFF - I -> Input flag
    raw.c_lflag &= ~ (ECHO | ICANON | IEXTEN | ISIG);        //<- ECHO makes every key being repeated into the Terminal - Turned Off for ECHO to not get in the way 
                                                    //-> WRITES/applys everything to the Terminal 
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);       // modified struct pased here  

}




//read() and STDIN_FILENO -> from unistd.h 
int main() {
    enableRawMode(); 

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q'){         //read() -> reads 1 byte and puts it into the variable c
        if (iscntrl(c)){                                        //-> iscntrl() tests if character is a control character -> non printable characters, but asc 0-31 & 127
            printf("%d\n", c);                                  // in this case the ASCII code is returned  | %d -> decimal number 
        } else {
            printf("%d ('%c')\n", c, c);                        // printable 32-126 | -> %c write byte as a character 
        }
    }  
    return 0;                                                   // while makes it do so until there is nothing to read anymore 
}