#include <locale.h>
#include "include/cb.h"

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    init_cb(argc, argv);
    return 0;
}