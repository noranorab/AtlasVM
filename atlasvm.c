#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <openssl/sha.h>
#include <stddef.h>
#include "pow.h"

/* Memory */
#define MEMORY_MAX (1 << 8)
uint8_t memory[MEMORY_MAX]; /* 256 locations */

/* Network */
int sockfd;

/* Registers */
enum {
    R_PC,  /* Program counter */
    R_ACC, /* Accumulator */
    R_COUNT /* Total number of registers */
};

uint8_t reg[R_COUNT];

/* Function Declarations */
uint8_t mem_read(uint16_t address);

/* Instruction set */
enum {
    OP_ADD = 0x00,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_LOAD,
    OP_STORE,
    OP_JUMP,
    OP_JZ,
    OP_JNZ,
    OP_IN,
    OP_OUT,
    OP_HALT,
    OP_NET_SEND,  // New opcode for sending data
    OP_NET_RECV,  // New opcode for receiving data
    OP_POW = 0x11, // Example: Unique opcode for POW
};

int read_image(const char* image_path) {
    FILE* file = fopen(image_path, "rb");
    if (!file) {
        perror("Failed to open image file");
        return 0;
    }

    fread(memory, sizeof(uint8_t), MEMORY_MAX, file);
    fclose(file);

    return 1;
}

uint8_t mem_read(uint16_t address) {
    return memory[address];
}

void ADD(uint8_t operand) {
    uint8_t value = mem_read(operand);
    reg[R_ACC] += value;
}

void SUB(uint8_t operand) {
    uint8_t value = mem_read(operand);
    reg[R_ACC] -= value;
}

void MUL(uint8_t operand) {
    uint8_t value = mem_read(operand);
    reg[R_ACC] *= value;
}

void DIV(uint8_t operand) {
    uint8_t value = mem_read(operand);
    if (value != 0) {
        reg[R_ACC] /= value;
    }
}

void AND(uint8_t operand) {
    uint8_t value = mem_read(operand);
    reg[R_ACC] &= value;
}

void OR(uint8_t operand) {
    uint8_t value = mem_read(operand);
    reg[R_ACC] |= value;
}

void XOR(uint8_t operand) {
    uint8_t value = mem_read(operand);
    reg[R_ACC] ^= value;
}

void LOAD(uint8_t operand) {
    reg[R_ACC] = mem_read(operand);
}

void STORE(uint8_t operand) {
    memory[operand] = reg[R_ACC];
}

void JUMP(uint8_t operand) {
    reg[R_PC] = operand;
}

void JZ() {
    if (reg[R_ACC] == 0) {
        uint8_t offset = mem_read(reg[R_PC]);
        reg[R_PC] = (uint8_t)(reg[R_PC] + (int8_t)offset);
    } else {
        reg[R_PC]++;
    }
}

void JNZ() {
    if (reg[R_ACC] != 0) {
        uint8_t offset = mem_read(reg[R_PC]);
        reg[R_PC] = (uint8_t)(reg[R_PC] + (int8_t)offset);
    } else {
        reg[R_PC]++;
    }
}

void IN() {
    int num;
    scanf("%d", &num);
    reg[R_ACC] = (uint8_t) num;
}

void OUT() {
    printf("%d\n", reg[R_ACC]);
}

int HALT() {
    return 0;
}

void NET_INIT() {
    struct sockaddr_in servaddr;
    
    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    // Define server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);  // Example port
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Example IP

    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        perror("Connection to server failed");
        close(sockfd);
        exit(1);
    }
}

void NET_SEND(uint8_t operand) {
    uint8_t value = memory[operand];
    if (send(sockfd, &value, sizeof(value), 0) == -1) {
        perror("Send failed");
        close(sockfd);
        exit(1);
    }
    printf("Sent to server: %d\n", value);
}

void NET_RECV(uint8_t operand) {
    uint8_t buffer;
    if (recv(sockfd, &buffer, sizeof(buffer), 0) == -1) {
        perror("Receive failed");
        close(sockfd);
        exit(1);
    }
    memory[operand] = buffer;
}

/* POW algo */
void POW(uint8_t operand, uint32_t difficulty) {
    uint8_t block[MEMORY_MAX + sizeof(uint32_t)];
    memcpy(block, memory, MEMORY_MAX);
    int nonce = compute_pow(block, MEMORY_MAX, difficulty);
    memcpy(memory + operand, &nonce, sizeof(nonce));
    printf("Computed nonce: %d\n", nonce);
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        printf("atlasvm [image-file1] ...\n");
        exit(2);
    }

    for (int j = 1; j < argc; ++j) {
        if (!read_image(argv[j])) {
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }

    enum { PC_START = 0x00 };
    reg[R_PC] = PC_START;

    NET_INIT();

    int running = 1;
    while (running) {
        uint8_t instr = mem_read(reg[R_PC]++);
        uint8_t op = instr >> 4;
        uint8_t operand = instr & 0x0F;

        printf("PC: 0x%02X, Instruction: 0x%02X, Op: 0x%X, Operand: 0x%X, ACC: 0x%02X\n", reg[R_PC], instr, op, operand, reg[R_ACC]);

        switch(op) {
            case OP_ADD:
                ADD(operand);
                break;
            case OP_SUB:
                SUB(operand);
                break;
            case OP_MUL:
                MUL(operand);
                break;
            case OP_DIV:
                DIV(operand);
                break;
            case OP_AND:
                AND(operand);
                break;
            case OP_OR:
                OR(operand);
                break;
            case OP_XOR:
                XOR(operand);
                break;
            case OP_LOAD:
                LOAD(operand);
                break;
            case OP_STORE:
                STORE(operand);
                break;
            case OP_JUMP:
                JUMP(operand);
                break;
            case OP_JZ:
                JZ();
                break;
            case OP_JNZ:
                JNZ();
                break;
            case OP_IN:
                IN();
                break;
            case OP_OUT:
                OUT();
                break;
            case OP_HALT:
                running = HALT();
                break;
            case OP_NET_SEND:
                NET_SEND(operand);
                break;
            case OP_NET_RECV:
                NET_RECV(operand);
                break;
            case OP_POW:
                POW(operand, 4);  // Set difficulty to 4 for testing
                break;
            default:
                printf("Unknown instruction: 0x%02X\n", instr);
                running = 0;
                break;
        }
    }
    close(sockfd);

    return 0;
}
