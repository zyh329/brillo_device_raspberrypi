/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "wifi_hal_bcm43438"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <hardware_brillo/wifi_driver_hal.h>

const char kStationDeviceName[] = "wlan0";

/* Our HAL needs to set the AP/Station mode prior to actually initializing
 * the wifi. We use a dummy function for the initialize.
 */
static wifi_driver_error wifi_driver_initialize_bcm43438(void) {
    return WIFI_SUCCESS;
}

static wifi_driver_error wifi_driver_initialize_bcm43438_internal(void) {
    struct ifreq req;
    int rc, socketfd;

    socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (socketfd < 0) {
        ALOGE("%s: unable to open control socket", __func__);
        return WIFI_ERROR_UNKNOWN;
    }

    strcpy (req.ifr_name, kStationDeviceName);
    rc = ioctl(socketfd, SIOCGIFFLAGS, &req);
    if (rc < 0) {
        ALOGE("%s: unable to query interface wlan0", __func__);
        return WIFI_ERROR_UNKNOWN;
    }

    req.ifr_flags &= ~(IFF_UP|IFF_RUNNING);
    rc = ioctl(socketfd, SIOCSIFFLAGS, &req);
    if (rc < 0) {
        ALOGE("%s: unable to down interface wlan0", __func__);
        return WIFI_ERROR_UNKNOWN;
    }

    req.ifr_flags |= IFF_UP|IFF_RUNNING;
    rc = ioctl(socketfd, SIOCSIFFLAGS, &req);
    if (rc < 0) {
        ALOGE("%s: unable to up interface wlan0", __func__);
        return WIFI_ERROR_UNKNOWN;
    }

    return WIFI_SUCCESS;
}

static wifi_driver_error wifi_driver_set_mode_bcm43438(
    wifi_driver_mode mode,
    char* wifi_device_name,
    size_t wifi_device_name_size) {

    switch (mode) {
    case WIFI_MODE_AP:
        ALOGI("Setting mode WIFI_MODE_AP !");
        break;
    case WIFI_MODE_STATION:
        ALOGI("Setting mode WIFI_MODE_STATION !");
        break;
#ifdef WIFI_MODE_P2P
    case WIFI_MODE_P2P:
        ALOGI("Setting mode WIFI_MODE_P2P !");
        break;
#endif
    default:
        ALOGE("Unknown WiFi driver mode %d", mode);
        return WIFI_ERROR_INVALID_ARGS;
    }

    strlcpy(wifi_device_name, kStationDeviceName, wifi_device_name_size);
    return wifi_driver_initialize_bcm43438_internal();
}

static int
close_bcm43438_driver(struct hw_device_t *device) {
    wifi_driver_device_t *dev = (wifi_driver_device_t *)device;
    if (dev)
        free(dev);
    return 0;
}

static int
open_bcm43438_driver(const struct hw_module_t* module, const char*, struct hw_device_t** device) {
    wifi_driver_device_t* dev = (wifi_driver_device_t *)calloc(1, sizeof(wifi_driver_device_t));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = WIFI_DRIVER_DEVICE_API_VERSION_0_1;

    // We're forced into this cast by the existing API.  This pattern is
    // common among users of the HAL.
    dev->common.module = (hw_module_t *)module;

    dev->common.close = close_bcm43438_driver;
    dev->wifi_driver_initialize = wifi_driver_initialize_bcm43438;
    dev->wifi_driver_set_mode = wifi_driver_set_mode_bcm43438;

    *device = &dev->common;

    return 0;
}

static struct hw_module_methods_t bcm43438_driver_module_methods = {
    .open = open_bcm43438_driver
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag           = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id            = WIFI_DRIVER_HARDWARE_MODULE_ID,
    .name          = "BCM43438 module",
    .author        = "Intel",
    .methods       = &bcm43438_driver_module_methods,
    .dso           = NULL,
    .reserved      = {0},
};
