//#include "../../drivers/keyboard/keyboard.h"
#include "../../lib/stdlib/strings.h"
#include "../../drivers/vga/vga.h"

void terminal_execute(const char* command)
{
    if(comp("clear", command) != 0) {
                  //Reset();
        }
        else if(comp("exit", command) != 0) {
            print("\t exit");
        }
        else if(comp("info", command) != 0) {
            print("\tbasic commands exit, clear, info");
        }
        else if(comp("ls", command) != 0) {
            print("\n");
            //printfs();
        }
        else{
            //const char* out = split(text);
            //print(out);
            print("\ncommand not found");
        }
        print("\nNurOs-> ");
        //clear();
}