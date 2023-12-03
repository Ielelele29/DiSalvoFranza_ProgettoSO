#include <stdio.h>
#include <unistd.h>

void met();

int main() {
    printf("Hello, World 4!\n");
    met();
    sleep(3);
    #define TEST 5
    return 0;
}

void met()
{
    printf("Provetta %i", TEST);
}
