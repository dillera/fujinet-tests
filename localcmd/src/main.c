#include <cmoc.h>
#include <coco.h>
#include "fujinet-fuji.h"
#include "fujinet-network.h"

#define CHUNK_SIZE 256
#define PORT "6573"
#define SOCKET "N1:TCP://:" PORT
#define CLIENT_SOCKET "N2:TCP://localhost:" PORT


void write_file_data(byte *src, size_t total_len)
{
    size_t offset = 0;
    byte err;

    while (offset < total_len) {
        size_t chunk_len = total_len - offset;
        if (chunk_len > CHUNK_SIZE)
            chunk_len = CHUNK_SIZE;

        err = network_write(SOCKET, &src[offset], chunk_len);
        if (err != FN_ERR_OK) {
            printf("NETWORK WRITE FAIL: %u\n", err);
            exit(1);
        }   

        offset += chunk_len;
    }
}

void get_json_data(void)
{
    char command[50];
    char device[50];
    char prefix[50];
    char expected[50];
    bool errorExpected;
    byte host_slot;
    byte device_slot;
    char data[50];
    byte replyLength;
    byte mode;
    byte algorithm;
    char as_hex[50];
    bool warnOnly;

    int16_t bytesread;
    int count = 0;
    char query[256];

    while (true)
    {

        sprintf(query, "/%d/command", count);
        bytesread = network_json_query(CLIENT_SOCKET, query, command);
        if (!bytesread) // If we didn't get any more objects with "command" in them, we're done.
        {
            break;
        }

        printf("command: %s\n", command);

        count++;
        waitkey(1);
    }

    printf("\n%d JSON objects read.\n", count);
}

int main(void)
{
    // Placeholder main function for JSON reading utility
    byte file_buffer[2048];
    byte work_buffer[256];
    size_t bytes_read = 0;
    int16_t rlen, pos;
    uint8_t err, status;
    uint16_t avail;
    size_t length;
    byte retval = 0;

    cls(1);

    // Returns:
    //   0 = success;
    //   1 = read error when reading directory sector(s);
    //   2 = file not found;
    //   3 = read error when reading the FAT;
    //   4 = read error when reading a file sector.
    memset(file_buffer, 0, sizeof(file_buffer));

    retval = readDECBFileWithDECB(file_buffer, 0, "TESTS   JSN", work_buffer, &bytes_read);

    if (retval != 0)
    {
        // Error reading file
        switch (retval)
        {
            case 1:
                printf("Error: Read error reading directory sectors.\n");
                break;
            case 2:
                printf("Error: File not found.\n");
                break;
            case 3:
                printf("Error: Read error reading the FAT.\n");
                break;
                case 4:
                printf("Error: Read error reading a file sector.\n");
            default:
                printf("Error: Unknown error code %d.\n", retval);
                break;
        }
        return 1;
    }

    printf("Read %u bytes from TESTS.TXT\n", bytes_read);
    printf("Parsing JSON from file contents...\n");

    // Open server socket
    printf("Opening server socket (%s)...\n", SOCKET);
    err = network_open(SOCKET, OPEN_MODE_RW, 0);

    if (err)
    {
        printf("SERVER SOCKET OPEN FAIL: %u\n", err);
        exit(1);
    }

    printf("Opening client socket (%s)...\n", CLIENT_SOCKET);
    err = network_open(CLIENT_SOCKET, OPEN_MODE_READ, 0);

    if (err)
    {
        printf("CLIENT SOCKET OPEN FAIL: %u\n", err);
        exit(1);
    }

    printf("Waiting for connection...\n");

    for (;;)
    {
        err = network_status(SOCKET, &avail, &status, &err);
        // printf("AVAIL: %u  STATUS: %u  ERR: %u\n", avail, status, err);
        if (err)
        {
            printf("SERVER SOCKET STATUS FAIL: %u\n", err);
            exit(1);
        }
        if (status == 1)
            break;
    }

    printf("Accepting connection...\n");
    err = network_accept(SOCKET);

    if (err)
    {
        printf("ACCEPT FAIL: %u\n", err);
        exit(1);
    }

    printf("Connected.\n");
    printf("Sending file data...\n");
    write_file_data(file_buffer, bytes_read);
    network_close(SOCKET);
    printf("Parsing json from file data...\n");
    network_json_parse(CLIENT_SOCKET);
    printf("Getting json data\n");
    get_json_data();
    network_close(CLIENT_SOCKET);

    return 0;
}

