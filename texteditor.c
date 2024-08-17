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
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP |IXON );                      //turns of ctrl q and s, used for dataflow/transmission controll XON XOFF - I -> Input flag
    raw.c_oflag &= ~(OPOST);                            // turns of the output processing 
    raw.c_cflag &= (CS8);                               // Sets the Character size to 8 Bits per Byte ~ usally on by default  
    raw.c_lflag &= ~ (ECHO | ICANON | IEXTEN | ISIG);   //<- ECHO makes every key being repeated into the Terminal - Turned Off for ECHO to not get in the way 

    raw.c_cc[VMIN] = 0;                                 //c_cc -> Controll charcters // VMIN -> minimum number of bytes for read() to return... 0->as soon as there is anything 
    raw.c_cc[VTIME] = 1;                                //VTIME max time to wait before read() returns 

                                                        //-> WRITES/applys everything to the Terminal 
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);           // modified struct pased here  

}




//read() and STDIN_FILENO -> from unistd.h 
int main() {
    enableRawMode(); 

    
    while (1) {         
        char c = '\0';
        read(STDIN_FILENO, &c, 1);                              //read() -> reads 1 byte and puts it into the variable c
        if (iscntrl(c)){                                        //-> iscntrl() tests if character is a control character -> non printable characters, but asc 0-31 & 127
            printf("%d\r\n", c);                                  // in this case the ASCII code is returned  | %d -> decimal number 
        } else {                                                  // \r need to "return" the Cariage like tipewrigther               
            printf("%d ('%c')\r\n", c, c);                        // printable 32-126 | -> %c write byte as a character 
        }
        if(c == 'q') break;
    }  
    return 0;                                                   // while makes it do so until there is nothing to read anymore 
}