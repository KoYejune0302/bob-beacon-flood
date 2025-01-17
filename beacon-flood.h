#ifndef BEACON_FLOOD_H
#define BEACON_FLOOD_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <time.h>

// Radiotap header structure
typedef struct {
    uint8_t it_version;
    uint8_t it_pad;
    uint16_t it_len;
    uint32_t it_present;
} radiotap_header_t;

// IEEE 802.11 Beacon frame structure
typedef struct {
    uint16_t frame_control;
    uint16_t duration;
    uint8_t ra[6];
    uint8_t ta[6];
    uint8_t bssid[6];
    uint16_t seq_ctrl;
    uint64_t timestamp;
    uint16_t beacon_interval;
    uint16_t capability;
    uint8_t ssid_ie; // SSID IE tag
    uint8_t ssid_len;
    uint8_t ssid[32];
} beacon_frame_t;

// Function prototypes
void build_radiotap_header(radiotap_header_t *radiotap);
void build_beacon_frame(beacon_frame_t *beacon, const char *ssid);
int send_beacon_packet(int sock, const radiotap_header_t *radiotap, const beacon_frame_t *beacon, const char *interface);

#endif // BEACON_FLOOD_H