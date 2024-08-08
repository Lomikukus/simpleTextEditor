#include <unistd.h> 

//read() and STDIN_FILENO -> from unistd.h 
int main() {
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q'); //read() -> reads 1 byte and puts it into the variable c 
    return 0;                               // while makes it do so until there is nothing to read anymore 
}