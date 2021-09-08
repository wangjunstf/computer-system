#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv){
    int ret = access("t.txt",F_OK);
    return 0;
}