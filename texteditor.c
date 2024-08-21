/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h> // meant to Turn the Console into raw mode 
#include <unistd.h> 


/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x1f)  //defines Controll Key 

/*** data ***/

struct termios orig_termios;


/*** terminal ***/

//prints error message and exits programm
void die(const char* s) {
    perror(s);              //perror looks at the global error variable (errno) and prints a descriptive error message 
    exit(1);                // exit of 1 indicates error 
}

//disable Raw Mode 
void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcsetattr"); //check error 
}

//enables Raw mode 
void enableRawMode(){
                                                    //-> READS attributes from Terminal
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");         //tcgetattr sets the terminals attributes and reads them into a struct  
    atexit(disableRawMode);                         //registers when programm is exited and disables raw mode 

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP |IXON );                      //turns of ctrl q and s, used for dataflow/transmission controll XON XOFF - I -> Input flag
    raw.c_oflag &= ~(OPOST);                            // turns of the output processing 
    raw.c_cflag &= (CS8);                               // Sets the Character size to 8 Bits per Byte ~ usally on by default  
    raw.c_lflag &= ~ (ECHO | ICANON | IEXTEN | ISIG);   //<- ECHO makes every key being repeated into the Terminal - Turned Off for ECHO to not get in the way 

    raw.c_cc[VMIN] = 0;                                 //c_cc -> Controll charcters // VMIN -> minimum number of bytes for read() to return... 0->as soon as there is anything 
    raw.c_cc[VTIME] = 1;                                //VTIME max time to wait before read() returns 

                                                        //-> WRITES/applys everything to the Terminal 
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");           // modified struct pased here  

}

//wait for Keypresses and them Return them
char editorReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c , 1)) != 1) {
        if(nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}
/*** output ***/

void editorRefreshScreen(){
    write(STDOUT_FILENO, "\x1b[2J", 4); //-> write() -> write 4 bytes to terminal | \x1b is an escape character 27-> decimal | J -> Command -> clear screen | argument 2 means entire screen
}                                       //escape sequence written to terminal -> always start with 27 ^^^ followed by [
                                        // escape sequences instruct the terminal to do a variaty of txt formating (coloring, moving cursor, clearing, etc.)    

/*** input ***/ //-> editor functionality mapping 
//waits for keypress and then handles it -> CTRL key functionality 
void editorProcessKeypress() {
    char c = editorReadKey();

    switch (c) {
        case CTRL_KEY('q'):
            exit(0);
            break; 
    }
}

/*** init ***/

//read() and STDIN_FILENO -> from unistd.h 
int main() {
    enableRawMode(); 

    while (1){
        editorRefreshScreen();
        editorProcessKeypress();
    }
    
    return 0;                                                   // while makes it do so until there is nothing to read anymore 
}