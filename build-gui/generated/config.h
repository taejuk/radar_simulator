#ifndef CONFIG_H
#define CONFIG_H

#define DEVICE_COUNT        6

/*
 * CMake의 ENABLE_GUI 값을 0 또는 1로 생성한다.
 */
#define ENABLE_GUI 1
#define PACKET_JSON_DIR "/home/taejuk/simulator/config/packets"


#define SERVER_IP           "127.0.0.1"

#define DEVICE_A_PORT       5001
#define DEVICE_B_PORT       5002
#define DEVICE_C_PORT       5003
#define DEVICE_D_PORT       5004
#define DEVICE_E_PORT       5005
#define DEVICE_F_PORT       5006

#define MAX_PAYLOAD_SIZE    4096
#define LISTEN_BACKLOG      1

#endif
