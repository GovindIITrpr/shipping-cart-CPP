#include <dragon/dragon.h>
#include "app.h"

int main(int argc, char *argv[]) {
    Dragon::initialize(argc, argv);
    
    App app;
    app.start();

    return 0;
}