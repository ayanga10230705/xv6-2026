#include "types.h"
#include "user.h"

int main() {
    printf(1, "[USER] calling write\n");  // 第一层
    write(1, "hello xv6\n", 10);
    exit();
}