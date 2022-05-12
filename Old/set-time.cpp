#include <stdlib.h>
#include <time.h>

int main(int argc, const char* argv[]) {
    struct timespec ts;
    ts.tv_sec = atoi(argv[1]);
    ts.tv_nsec = 0;
    return !clock_settime(CLOCK_REALTIME, &ts)?EXIT_SUCCESS:EXIT_FAILURE;
}
