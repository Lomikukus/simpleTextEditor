/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h> //Input Output Control IOCtl
#include <termios.h> // meant to Turn the Console into raw mode 
#include <unistd.h> 


/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x1f)  //defines Controll Key 

/*** data ***/

struct editorConfig {
    int screenrows;
    int screencols;
    struct termios orig_termios;
};
 
 struct editorConfig E; //global variable containing editor state 





/*** terminal ***/

//prints error message and exits programm
void die(const char* s) {
    write(STDOUT_FILENO, "\x1b[2J",4);
    write(STDOUT_FILENO, "\x1b[H",3);

    perror(s);              //perror looks at the global error variable (errno) and prints a descriptive error message 
    exit(1);                // exit of 1 indicates error 
}

//disable Raw Mode 
void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) die("tcsetattr"); //check error 
}

//enables Raw mode 
void enableRawMode(){
                                                    //-> READS attributes from Terminal
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");         //tcgetattr sets the terminals attributes and reads them into a struct  
    atexit(disableRawMode);                         //registers when programm is exited and disables raw mode 

    struct termios raw = E.orig_termios;
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
// gets the current position of the cursor so that u can calculate the window size later on 
int getCursorPosition(int *rows, int *cols){
    char buf[32]; 
    unsigned int i = 0; 

    if (write(STDOUT_FILENO, "\x1b[6n", 4) !=4) return -1; 

    while (i < sizeof(buf) - 1){
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] =  '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

    return 0;
}


int getWindowSize(int *rows, int *cols){
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        if ( write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1; 
        return getCursorPosition(rows, cols); 
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}


/*** output ***/
// draws rows like in vim
void editorDrawRows(){
    int y;
    for (y = 0; y < E.screenrows; y++){
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}


void editorRefreshScreen(){
    write(STDOUT_FILENO, "\x1b[2J", 4); //-> write() -> write 4 bytes to terminal | \x1b is an escape character 27-> decimal | J -> Command -> clear screen | argument 2 means entire screen
    write(STDOUT_FILENO, "\x1b[H", 3);  //escape sequence written to terminal -> always start with 27 ^^^ followed by [ | H -command-> position cursor S
                                        // escape sequences instruct the terminal to do a variaty of txt formating (coloring, moving cursor, clearing, etc.)
    editorDrawRows();

    write(STDOUT_FILENO, "\x1b[H", 3); 
}                                           
                                        

/*** input ***/ //-> editor functionality mapping 
//waits for keypress and then handles it -> CTRL key functionality 
void editorProcessKeypress() {
    char c = editorReadKey();

    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J",4);
            write(STDOUT_FILENO, "\x1b[H",3);
            exit(0);
            break; 
    }
}

/*** init ***/

void initEditor() {
    if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize"); 
}

//read() and STDIN_FILENO -> from unistd.h 
int main() {
    enableRawMode(); 
    initEditor(); 

    while (1){
        editorRefreshScreen();
        editorProcessKeypress();
    }
    
    return 0;                                                   // while makes it do so until there is nothing to read anymore 
}