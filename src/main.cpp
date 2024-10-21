#include <glog/logging.h>
#include "twinviewer.h"

// ------------------------------------------
// main

int main(int argc, char *argv[]) {
    google::InitGoogleLogging(argv[0]);
    // parse cmd line args and window setup
    twin::TwinViewer twin(argc, argv);

    // run
    twin.run();
}
