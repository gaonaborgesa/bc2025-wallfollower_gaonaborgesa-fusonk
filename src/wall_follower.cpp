#include <iostream>
#include <cmath>

#include <signal.h>

#include <wall_follower/common/utils.h>
#include <wall_follower/robot/robot.h>

bool ctrl_c_pressed;
void ctrlc(int)
{
    ctrl_c_pressed = true;
}


int main(int argc, const char *argv[])
{
    signal(SIGINT, ctrlc);
    signal(SIGTERM, ctrlc);

    // Initialize the Lidar listener and create empty vectors to store the scan data.
    initLidarListener();
    std::vector<float> ranges;
    std::vector<float> thetas;

    /**
     * TODO: Declare any variables you need here.
     */

    while (true) {
        // These functions get the Lidar scan data.
        handle();
        getLidarScan(ranges, thetas);

        /**
         * TODO: (P1.2) Write code to follow the nearest wall here.
         *
         * HINT: You should use the functions crossProduct and findMinDist.
         */

        if (ctrl_c_pressed) break;
    }

    // Stop the robot.
    drive(0, 0, 0);
    return 0;
}
