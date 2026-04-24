#include "ee14lib.h"
#include <stdio.h>


int _write(int file, char *data, int len) {
    serial_write(USART2,data, len);
    return len;
}

    int main() {
        host_serial_init(115200);
        char buff[1024];
        char Hello[4];

        serial_write(USART2, Hello, 4);
        int size = 0;
        for(int i = 0; i < 4; i++) {
            size += ((uint8_t) serial_read(USART2)) << (i * 8);
        }
        printf("%d\n", size);


        while (1) {
            for(int i = 0; i < size; i++) {
                const char c = serial_read(USART2);
                buff[i] = c;
            }
            serial_write(USART2, buff, size);           
        }
    }

