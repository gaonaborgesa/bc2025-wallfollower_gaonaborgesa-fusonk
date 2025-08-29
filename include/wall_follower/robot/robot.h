#ifndef WALL_FOLLOWER_ROBOT_ROBOT_H
#define WALL_FOLLOWER_ROBOT_ROBOT_H

#include <vector>
#include <string>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <lcm/lcm-cpp.hpp>
#include <mbot_lcm_msgs/lidar_t.hpp>
#include <mbot_lcm_msgs/twist2D_t.hpp>

#include "../common/utils.h"

#define MULTICAST_URL "udpm://239.255.76.67:7667?ttl=2"
#define MBOT_MOTOR_COMMAND_CHANNEL "MBOT_VEL_CMD"
#define LIDAR_CHANNEL "LIDAR"
#define TIMEOUT 5000

static lcm::LCM lcmInstance(MULTICAST_URL);
static bool lidar_received_;
static mbot_lcm_msgs::lidar_t lidar_scan_;
lcm::LCM::HandlerFunction<mbot_lcm_msgs::lidar_t> lidarCallback;

static void drive(const float vx, const float vy, const float wz)
{
    mbot_lcm_msgs::twist2D_t cmd;
    cmd.utime = getTimeMicro();
    cmd.vx = vx;
    cmd.vy = vy;
    cmd.wz = wz;

    lcmInstance.publish(MBOT_MOTOR_COMMAND_CHANNEL, &cmd);
};

static void handle()
{
    if (lcmInstance.handleTimeout(TIMEOUT) == 0)
    {
        std::cout << "WARNING: No Lidar data received.\n";
        lidar_received_ = false;
    }
}

static void initLidarListener()
{
    lidar_received_ = false;
    lidarCallback = [](const lcm::ReceiveBuffer* rbuf, const std::string& channel,
                       const mbot_lcm_msgs::lidar_t* msg)
    {
        lidar_scan_ = *msg;
        lidar_received_ = true;
    };
    lcmInstance.subscribe(LIDAR_CHANNEL, lidarCallback);
    handle();
}

static void getLidarScan(std::vector<float>& ranges, std::vector<float>& thetas)
{
    // Clear any old values in the data.
    ranges.clear();
    thetas.clear();

    if (lidar_received_)
    {
        // Copy over most recent scan data.
        ranges = lidar_scan_.ranges;
        thetas = lidar_scan_.thetas;
    }
};

static bool hasLidar()
{
    return lidar_received_;
};

static bool isReady(const double timeout = 0)
{
    fd_set rfds;
    int lcmCheck = lcmInstance.getFileno();

    FD_ZERO(&rfds);
    FD_SET(lcmCheck, &rfds);

    struct timeval tv;
    tv.tv_sec = static_cast<int>(timeout);
    tv.tv_usec = static_cast<int>((timeout - tv.tv_sec) * 1e6);

    int retval = select(lcmCheck + 1, &rfds, NULL, NULL, &tv);

    if (retval == -1)
    {
        return false;
    }
    else if (retval)
    {
        return true;
    }

    return false;
}

static bool handleOnce(const double timeout = 0)
{
    if (isReady(timeout))
    {
        return (lcmInstance.handle() == 0);
    }

    return true;
}

#endif  // WALL_FOLLOWER_ROBOT_ROBOT_H