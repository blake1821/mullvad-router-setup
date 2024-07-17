
// monitor of iplookup.c

extern "C"{
#include "../../common/debug.h"
}
#include <iostream>

using namespace std;

int main(){
    init_debug(false);

    cout << "Debug shared memory initialized. " << endl;
    while(true){
        cout << "Press any key to print the debug stack: ";
        
        char c;
        cin >> c;

        print_debug_stack();
    }

}