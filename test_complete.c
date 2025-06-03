#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Include your refactored code here (or compile separately and link)
// For this test, I'm assuming the refactored code is in "error_detection.h"
// You can either copy the entire refactored code above this line,
// or save it as error_detection.c and create a header file

// Maximum supported sizes
#define MAX_BLOCKS 100
#define MAX_BLOCK_SIZE 64
#define MAX_DATA_SIZE 1000

// Error codes
typedef enum {
    EDC_SUCCESS = 0,
    EDC_ERROR_INVALID_INPUT = -1,
    EDC_ERROR_MEMORY_ALLOCATION = -2,
    EDC_ERROR_SIZE_EXCEEDED = -3,
    EDC_ERROR_NULL_POINTER = -4
} edc_error_t;

// Result structures
typedef struct {
    uint8_t checksum[MAX_BLOCK_SIZE];
    uint8_t data_with_checksum[MAX_BLOCKS + 1][MAX_BLOCK_SIZE];
    int num_blocks;
    int block_size;
    int error_detected;
    edc_error_t status;
} checksum_result_t;

typedef struct {
    uint8_t parity_block[MAX_BLOCK_SIZE];
    uint8_t data_with_parity[MAX_BLOCKS + 1][MAX_BLOCK_SIZE];
    int num_blocks;
    int block_size;
    int error_detected;
    edc_error_t status;
} lrc_result_t;

typedef struct {
    uint8_t parity_bits[MAX_BLOCKS];
    uint8_t data_with_parity[MAX_BLOCKS][MAX_BLOCK_SIZE + 1];
    int num_blocks;
    int block_size;
    int error_detected;
    edc_error_t status;
} vrc_result_t;

typedef struct {
    uint8_t remainder[MAX_BLOCK_SIZE];
    uint8_t codeword[MAX_DATA_SIZE];
    int dataword_size;
    int key_size;
    int codeword_size;
    int error_detected;
    edc_error_t status;
} crc_result_t;

// Input validation functions
static int validate_checksum_input(uint8_t data[][MAX_BLOCK_SIZE], int num_blocks, int block_size) {
    if (!data || num_blocks <= 0 || block_size <= 0) return 0;
    if (num_blocks > MAX_BLOCKS || block_size > MAX_BLOCK_SIZE) return 0;
    return 1;
}

static int validate_binary_data(uint8_t data[][MAX_BLOCK_SIZE], int num_blocks, int block_size) {
    for (int i = 0; i < num_blocks; i++) {
        for (int j = 0; j < block_size; j++) {
            if (data[i][j] != 0 && data[i][j] != 1) return 0;
        }
    }
    return 1;
}

// Checksum implementation
checksum_result_t calculate_checksum(uint8_t data[][MAX_BLOCK_SIZE], int num_blocks, int block_size) {
    checksum_result_t result = {0};
    
    if (!validate_checksum_input(data, num_blocks, block_size)) {
        result.status = EDC_ERROR_INVALID_INPUT;
        return result;
    }
    
    if (!validate_binary_data(data, num_blocks, block_size)) {
        result.status = EDC_ERROR_INVALID_INPUT;
        return result;
    }
    
    result.num_blocks = num_blocks;
    result.block_size = block_size;
    
    // Copy input data
    for (int i = 0; i < num_blocks; i++) {
        for (int j = 0; j < block_size; j++) {
            result.data_with_checksum[i][j] = data[i][j];
        }
    }
    
    uint8_t sum[MAX_BLOCK_SIZE] = {0};
    uint8_t carry = 0;
    
    // Calculate sum of all blocks
    for (int i = 0; i < num_blocks - 1; i++) {
        if (i == 0) {
            for (int j = block_size - 1; j >= 0; j--) {
                sum[j] = data[i][j] ^ data[i+1][j] ^ carry;
                carry = (data[i][j] & data[i+1][j]) | (data[i+1][j] & carry) | (carry & data[i][j]);
            }
        } else {
            for (int j = block_size - 1; j >= 0; j--) {
                uint8_t prevsum = sum[j];
                sum[j] = sum[j] ^ data[i+1][j] ^ carry;
                carry = (prevsum & data[i+1][j]) | (data[i+1][j] & carry) | (carry & prevsum);
            }
        }
        
        // Handle wrap-around carry
        if (carry == 1) {
            carry = 0;
            uint8_t temp[MAX_BLOCK_SIZE] = {0};
            temp[block_size - 1] = 1;
            
            for (int j = block_size - 1; j >= 0; j--) {
                uint8_t prevsum = sum[j];
                sum[j] = sum[j] ^ temp[j] ^ carry;
                carry = (prevsum & temp[j]) | (temp[j] & carry) | (carry & prevsum);
            }
        }
    }
    
    // One's complement for checksum
    for (int i = 0; i < block_size; i++) {
        result.checksum[i] = !sum[i];
        result.data_with_checksum[num_blocks][i] = result.checksum[i];
    }
    
    result.status = EDC_SUCCESS;
    return result;
}

checksum_result_t verify_checksum(uint8_t received_data[][MAX_BLOCK_SIZE], int num_blocks, int block_size) {
    checksum_result_t result = {0};
    
    if (!validate_checksum_input(received_data, num_blocks + 1, block_size)) {
        result.status = EDC_ERROR_INVALID_INPUT;
        return result;
    }
    
    result.num_blocks = num_blocks;
    result.block_size = block_size;
    
    uint8_t sum[MAX_BLOCK_SIZE] = {0};
    uint8_t carry = 0;
    
    // Calculate sum including checksum
    for (int i = 0; i < num_blocks; i++) {
        if (i == 0) {
            for (int j = block_size - 1; j >= 0; j--) {
                sum[j] = received_data[i][j] ^ received_data[i+1][j] ^ carry;
                carry = (received_data[i][j] & received_data[i+1][j]) | 
                       (received_data[i+1][j] & carry) | (carry & received_data[i][j]);
            }
        } else {
            for (int j = block_size - 1; j >= 0; j--) {
                uint8_t prevsum = sum[j];
                sum[j] = sum[j] ^ received_data[i+1][j] ^ carry;
                carry = (prevsum & received_data[i+1][j]) | 
                       (received_data[i+1][j] & carry) | (carry & prevsum);
            }
        }
        
        if (carry == 1) {
            carry = 0;
            uint8_t temp[MAX_BLOCK_SIZE] = {0};
            temp[block_size - 1] = 1;
            
            for (int j = block_size - 1; j >= 0; j--) {
                uint8_t prevsum = sum[j];
                sum[j] = sum[j] ^ temp[j] ^ carry;
                carry = (prevsum & temp[j]) | (temp[j] & carry) | (carry & prevsum);
            }
        }
    }
    
    // One's complement and check for errors
    result.error_detected = 0;
    for (int i = 0; i < block_size; i++) {
        result.checksum[i] = !sum[i];
        if (result.checksum[i] != 0) {
            result.error_detected = 1;
        }
    }
    
    result.status = EDC_SUCCESS;
    return result;
}

// LRC implementation
lrc_result_t calculate_lrc(uint8_t data[][MAX_BLOCK_SIZE], int num_blocks, int block_size) {
    lrc_result_t result = {0};
    
    if (!validate_checksum_input(data, num_blocks, block_size)) {
        result.status = EDC_ERROR_INVALID_INPUT;
        return result;
    }
    
    if (!validate_binary_data(data, num_blocks, block_size)) {
        result.status = EDC_ERROR_INVALID_INPUT;
        return result;
    }
    
    result.num_blocks = num_blocks;
    result.block_size = block_size;
    
    // Copy input data
    for (int i = 0; i < num_blocks; i++) {
        for (int j = 0; j < block_size; j++) {
            result.data_with_parity[i][j] = data[i][j];
        }
    }
    
    // Calculate LRC (column-wise XOR)
    for (int j = 0; j < block_size; j++) {
        result.parity_block[j] = 0;
        for (int i = 0; i < num_blocks; i++) {
            result.parity_block[j] ^= data[i][j];
        }
        result.data_with_parity[num_blocks][j] = result.parity_block[j];
    }
    
    result.status = EDC_SUCCESS;
    return result;
}

lrc_result_t verify_lrc(uint8_t received_data[][MAX_BLOCK_SIZE], int num_blocks, int block_size) {
    lrc_result_t result = {0};
    
    if (!validate_checksum_input(received_data, num_blocks + 1, block_size)) {
        result.status = EDC_ERROR_INVALID_INPUT;
        return result;
    }
    
    result.num_blocks = num_blocks;
    result.block_size = block_size;
    
    // Calculate LRC from received data (excluding parity block)
    for (int j = 0; j < block_size; j++) {
        result.parity_block[j] = 0;
        for (int i = 0; i < num_blocks; i++) {
            result.parity_block[j] ^= received_data[i][j];
        }
    }
    
    // Compare with received parity block
    result.error_detected = 0;
    for (int j = 0; j < block_size; j++) {
        if (result.parity_block[j] != received_data[num_blocks][j]) {
            result.error_detected = 1;
            break;
        }
    }
    
    result.status = EDC_SUCCESS;
    return result;
}

// VRC implementation
vrc_result_t calculate_vrc(uint8_t data[][MAX_BLOCK_SIZE], int num_blocks, int block_size) {
    vrc_result_t result = {0};
    
    if (!validate_checksum_input(data, num_blocks, block_size)) {
        result.status = EDC_ERROR_INVALID_INPUT;
        return result;
    }
    
    if (!validate_binary_data(data, num_blocks, block_size)) {
        result.status = EDC_ERROR_INVALID_INPUT;
        return result;
    }
    
    result.num_blocks = num_blocks;
    result.block_size = block_size;
    
    // Copy input data and calculate parity bits
    for (int i = 0; i < num_blocks; i++) {
        result.parity_bits[i] = 0;
        for (int j = 0; j < block_size; j++) {
            result.data_with_parity[i][j] = data[i][j];
            result.parity_bits[i] ^= data[i][j];
        }
        result.data_with_parity[i][block_size] = result.parity_bits[i];
    }
    
    result.status = EDC_SUCCESS;
    return result;
}

vrc_result_t verify_vrc(uint8_t received_data[][MAX_BLOCK_SIZE + 1], int num_blocks, int block_size) {
    vrc_result_t result = {0};
    
    if (!received_data || num_blocks <= 0 || block_size <= 0) {
        result.status = EDC_ERROR_INVALID_INPUT;
        return result;
    }
    
    if (num_blocks > MAX_BLOCKS || block_size > MAX_BLOCK_SIZE) {
        result.status = EDC_ERROR_INVALID_INPUT;
        return result;
    }
    
    result.num_blocks = num_blocks;
    result.block_size = block_size;
    result.error_detected = 0;
    
    // Check parity for each block
    for (int i = 0; i < num_blocks; i++) {
        int count = 0;
        for (int j = 0; j < block_size + 1; j++) {
            if (received_data[i][j] == 1) {
                count++;
            }
        }
        if (count % 2 != 0) {
            result.error_detected = 1;
            break;
        }
    }
    
    result.status = EDC_SUCCESS;
    return result;
}

// CRC implementation
crc_result_t calculate_crc(uint8_t *dataword, int dataword_size, uint8_t *key, int key_size) {
    crc_result_t result = {0};
    
    if (!dataword || !key || dataword_size <= 0 || key_size <= 0) {
        result.status = EDC_ERROR_NULL_POINTER;
        return result;
    }
    
    if (dataword_size + key_size - 1 > MAX_DATA_SIZE) {
        result.status = EDC_ERROR_SIZE_EXCEEDED;
        return result;
    }
    
    // Validate binary data
    for (int i = 0; i < dataword_size; i++) {
        if (dataword[i] != 0 && dataword[i] != 1) {
            result.status = EDC_ERROR_INVALID_INPUT;
            return result;
        }
    }
    
    for (int i = 0; i < key_size; i++) {
        if (key[i] != 0 && key[i] != 1) {
            result.status = EDC_ERROR_INVALID_INPUT;
            return result;
        }
    }
    
    result.dataword_size = dataword_size;
    result.key_size = key_size;
    result.codeword_size = dataword_size + key_size - 1;
    
    // Initialize codeword with dataword + zeros
    for (int i = 0; i < result.codeword_size; i++) {
        if (i < dataword_size) {
            result.codeword[i] = dataword[i];
        } else {
            result.codeword[i] = 0;
        }
    }
    
    uint8_t temp[MAX_BLOCK_SIZE];
    
    // Perform polynomial division
    for (int i = 0; i < dataword_size; i++) {
        if (i == 0) {
            if (result.codeword[i] == 0) {
                for (int j = 0; j < key_size; j++) {
                    temp[j] = result.codeword[j] ^ 0;
                }
            } else {
                for (int j = 0; j < key_size; j++) {
                    temp[j] = result.codeword[j] ^ key[j];
                }
            }
        } else {
            // Shift temp and bring in next bit
            for (int k = 0; k < key_size - 1; k++) {
                temp[k] = temp[k + 1];
            }
            temp[key_size - 1] = result.codeword[i + key_size - 1];
            
            if (temp[0] == 0) {
                for (int j = 0; j < key_size; j++) {
                    temp[j] = temp[j] ^ 0;
                }
            } else {
                for (int j = 0; j < key_size; j++) {
                    temp[j] = temp[j] ^ key[j];
                }
            }
        }
    }
    
    // Extract remainder and append to codeword
    for (int i = 0; i < key_size - 1; i++) {
        result.remainder[i] = temp[i + 1];
        result.codeword[dataword_size + i] = result.remainder[i];
    }
    
    result.status = EDC_SUCCESS;
    return result;
}

crc_result_t verify_crc(uint8_t *received_codeword, int codeword_size, uint8_t *key, int key_size) {
    crc_result_t result = {0};
    
    if (!received_codeword || !key || codeword_size <= 0 || key_size <= 0) {
        result.status = EDC_ERROR_NULL_POINTER;
        return result;
    }
    
    if (codeword_size > MAX_DATA_SIZE) {
        result.status = EDC_ERROR_SIZE_EXCEEDED;
        return result;
    }
    
    result.codeword_size = codeword_size;
    result.key_size = key_size;
    result.dataword_size = codeword_size - key_size + 1;
    
    // Copy received codeword
    for (int i = 0; i < codeword_size; i++) {
        result.codeword[i] = received_codeword[i];
    }
    
    uint8_t temp[MAX_BLOCK_SIZE];
    
    // Perform polynomial division on received codeword
    for (int i = 0; i < result.dataword_size; i++) {
        if (i == 0) {
            if (received_codeword[i] == 0) {
                for (int j = 0; j < key_size; j++) {
                    temp[j] = received_codeword[j] ^ 0;
                }
            } else {
                for (int j = 0; j < key_size; j++) {
                    temp[j] = received_codeword[j] ^ key[j];
                }
            }
        } else {
            for (int k = 0; k < key_size - 1; k++) {
                temp[k] = temp[k + 1];
            }
            temp[key_size - 1] = received_codeword[i + key_size - 1];
            
            if (temp[0] == 0) {
                for (int j = 0; j < key_size; j++) {
                    temp[j] = temp[j] ^ 0;
                }
            } else {
                for (int j = 0; j < key_size; j++) {
                    temp[j] = temp[j] ^ key[j];
                }
            }
        }
    }
    
    // Check remainder for errors
    result.error_detected = 0;
    for (int i = 0; i < key_size - 1; i++) {
        result.remainder[i] = temp[i + 1];
        if (result.remainder[i] != 0) {
            result.error_detected = 1;
        }
    }
    
    result.status = EDC_SUCCESS;
    return result;
}

// High-level convenience functions

// Convert string to binary array (for CRC)
int string_to_binary(const char *str, uint8_t *binary, int max_size) {
    int len = strlen(str);
    if (len > max_size) return -1;
    
    for (int i = 0; i < len; i++) {
        if (str[i] != '0' && str[i] != '1') return -1;
        binary[i] = str[i] - '0';
    }
    return len;
}

// Convert binary array to string
void binary_to_string(uint8_t *binary, int size, char *str, int str_size) {
    if (size + 1 > str_size) return;
    
    for (int i = 0; i < size; i++) {
        str[i] = binary[i] + '0';
    }
    str[size] = '\0';
}

// Convenience function for checksum with string input/output
int checksum_string(const char *input_blocks[], int num_blocks, int block_size, 
                   char *checksum_str, char *output_data[], int max_output_size) {
    if (!input_blocks || !checksum_str || !output_data || num_blocks <= 0 || block_size <= 0) {
        return EDC_ERROR_INVALID_INPUT;
    }
    
    uint8_t data[MAX_BLOCKS][MAX_BLOCK_SIZE];
    
    // Convert string input to binary
    for (int i = 0; i < num_blocks; i++) {
        if (string_to_binary(input_blocks[i], data[i], block_size) != block_size) {
            return EDC_ERROR_INVALID_INPUT;
        }
    }
    
    checksum_result_t result = calculate_checksum(data, num_blocks, block_size);
    if (result.status != EDC_SUCCESS) {
        return result.status;
    }
    
    // Convert results to strings
    binary_to_string(result.checksum, block_size, checksum_str, max_output_size);
    
    for (int i = 0; i < num_blocks + 1; i++) {
        binary_to_string(result.data_with_checksum[i], block_size, output_data[i], max_output_size);
    }
    
    return EDC_SUCCESS;
}

// Utility functions for testing
void print_binary_array(uint8_t *arr, int size, const char *label) {
    printf("%s: ", label);
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
    }
    printf("\n");
}

void print_2d_binary_array(uint8_t arr[][MAX_BLOCK_SIZE], int rows, int cols, const char *label) {
    printf("%s:\n", label);
    for (int i = 0; i < rows; i++) {
        printf("  Block %d: ", i + 1);
        for (int j = 0; j < cols; j++) {
            printf("%d", arr[i][j]);
        }
        printf("\n");
    }
}

void print_error_status(edc_error_t status) {
    switch (status) {
        case EDC_SUCCESS:
            printf("Status: SUCCESS\n");
            break;
        case EDC_ERROR_INVALID_INPUT:
            printf("Status: INVALID INPUT\n");
            break;
        case EDC_ERROR_MEMORY_ALLOCATION:
            printf("Status: MEMORY ALLOCATION ERROR\n");
            break;
        case EDC_ERROR_SIZE_EXCEEDED:
            printf("Status: SIZE EXCEEDED\n");
            break;
        case EDC_ERROR_NULL_POINTER:
            printf("Status: NULL POINTER\n");
            break;
        default:
            printf("Status: UNKNOWN ERROR\n");
    }
}

// Test functions
void test_checksum() {
    printf("=== TESTING CHECKSUM ===\n");
    
    // Test data: 3 blocks of 4 bits each
    uint8_t test_data[3][MAX_BLOCK_SIZE] = {
        {1, 0, 1, 0},  // Block 1: 1010
        {1, 1, 0, 1},  // Block 2: 1101
        {0, 1, 1, 0}   // Block 3: 0110
    };
    
    int num_blocks = 3;
    int block_size = 4;
    
    printf("\nTesting checksum calculation:\n");
    print_2d_binary_array(test_data, num_blocks, block_size, "Input data");
    
    checksum_result_t result = calculate_checksum(test_data, num_blocks, block_size);
    print_error_status(result.status);
    
    if (result.status == EDC_SUCCESS) {
        print_binary_array(result.checksum, block_size, "Calculated checksum");
        print_2d_binary_array(result.data_with_checksum, num_blocks + 1, block_size, "Data with checksum");
        
        // Test verification with correct data
        printf("\nTesting checksum verification (no errors):\n");
        checksum_result_t verify_result = verify_checksum(result.data_with_checksum, num_blocks, block_size);
        print_error_status(verify_result.status);
        printf("Error detected: %s\n", verify_result.error_detected ? "YES" : "NO");
        print_binary_array(verify_result.checksum, block_size, "Verification checksum");
        
        // Test verification with corrupted data
        printf("\nTesting checksum verification (with error):\n");
        uint8_t corrupted_data[4][MAX_BLOCK_SIZE];
        memcpy(corrupted_data, result.data_with_checksum, sizeof(corrupted_data));
        corrupted_data[0][0] = !corrupted_data[0][0]; // Flip one bit
        
        checksum_result_t error_result = verify_checksum(corrupted_data, num_blocks, block_size);
        print_error_status(error_result.status);
        printf("Error detected: %s\n", error_result.error_detected ? "YES" : "NO");
    }
}

void test_lrc() {
    printf("\n=== TESTING LRC ===\n");
    
    // Test data: 3 blocks of 4 bits each
    uint8_t test_data[3][MAX_BLOCK_SIZE] = {
        {1, 0, 1, 0},  // Block 1
        {1, 1, 0, 1},  // Block 2
        {0, 1, 1, 0}   // Block 3
    };
    
    int num_blocks = 3;
    int block_size = 4;
    
    printf("\nTesting LRC calculation:\n");
    print_2d_binary_array(test_data, num_blocks, block_size, "Input data");
    
    lrc_result_t result = calculate_lrc(test_data, num_blocks, block_size);
    print_error_status(result.status);
    
    if (result.status == EDC_SUCCESS) {
        print_binary_array(result.parity_block, block_size, "LRC parity block");
        print_2d_binary_array(result.data_with_parity, num_blocks + 1, block_size, "Data with LRC");
        
        // Test verification
        printf("\nTesting LRC verification (no errors):\n");
        lrc_result_t verify_result = verify_lrc(result.data_with_parity, num_blocks, block_size);
        print_error_status(verify_result.status);
        printf("Error detected: %s\n", verify_result.error_detected ? "YES" : "NO");
        
        // Test with error
        printf("\nTesting LRC verification (with error):\n");
        uint8_t corrupted_data[4][MAX_BLOCK_SIZE];
        memcpy(corrupted_data, result.data_with_parity, sizeof(corrupted_data));
        corrupted_data[1][2] = !corrupted_data[1][2]; // Flip one bit
        
        lrc_result_t error_result = verify_lrc(corrupted_data, num_blocks, block_size);
        print_error_status(error_result.status);
        printf("Error detected: %s\n", error_result.error_detected ? "YES" : "NO");
    }
}

void test_vrc() {
    printf("\n=== TESTING VRC ===\n");
    
    // Test data: 3 blocks of 4 bits each
    uint8_t test_data[3][MAX_BLOCK_SIZE] = {
        {1, 0, 1, 0},  // Block 1 (even parity)
        {1, 1, 0, 1},  // Block 2 (odd parity, will add 1)
        {0, 1, 1, 0}   // Block 3 (even parity)
    };
    
    int num_blocks = 3;
    int block_size = 4;
    
    printf("\nTesting VRC calculation:\n");
    print_2d_binary_array(test_data, num_blocks, block_size, "Input data");
    
    vrc_result_t result = calculate_vrc(test_data, num_blocks, block_size);
    print_error_status(result.status);
    
    if (result.status == EDC_SUCCESS) {
        print_binary_array(result.parity_bits, num_blocks, "VRC parity bits");
        
        printf("Data with VRC parity:\n");
        for (int i = 0; i < num_blocks; i++) {
            printf("  Block %d: ", i + 1);
            for (int j = 0; j < block_size + 1; j++) {
                printf("%d", result.data_with_parity[i][j]);
            }
            printf("\n");
        }
        
        // Test verification
        printf("\nTesting VRC verification (no errors):\n");
        vrc_result_t verify_result = verify_vrc(result.data_with_parity, num_blocks, block_size);
        print_error_status(verify_result.status);
        printf("Error detected: %s\n", verify_result.error_detected ? "YES" : "NO");
        
        // Test with error
        printf("\nTesting VRC verification (with error):\n");
        uint8_t corrupted_data[3][MAX_BLOCK_SIZE + 1];
        memcpy(corrupted_data, result.data_with_parity, sizeof(corrupted_data));
        corrupted_data[1][1] = !corrupted_data[1][1]; // Flip one bit
        
        vrc_result_t error_result = verify_vrc(corrupted_data, num_blocks, block_size);
        print_error_status(error_result.status);
        printf("Error detected: %s\n", error_result.error_detected ? "YES" : "NO");
    }
}

void test_crc() {
    printf("\n=== TESTING CRC ===\n");
    
    // Test data
    uint8_t dataword[] = {1, 0, 1, 1, 0, 1};  // 101101
    uint8_t key[] = {1, 0, 1, 1};              // 1011
    int dataword_size = 6;
    int key_size = 4;
    
    printf("\nTesting CRC calculation:\n");
    print_binary_array(dataword, dataword_size, "Dataword");
    print_binary_array(key, key_size, "Key (generator polynomial)");
    
    crc_result_t result = calculate_crc(dataword, dataword_size, key, key_size);
    print_error_status(result.status);
    
    if (result.status == EDC_SUCCESS) {
        print_binary_array(result.remainder, key_size - 1, "CRC remainder");
        print_binary_array(result.codeword, result.codeword_size, "Generated codeword");
        
        // Test verification
        printf("\nTesting CRC verification (no errors):\n");
        crc_result_t verify_result = verify_crc(result.codeword, result.codeword_size, key, key_size);
        print_error_status(verify_result.status);
        printf("Error detected: %s\n", verify_result.error_detected ? "YES" : "NO");
        print_binary_array(verify_result.remainder, key_size - 1, "Verification remainder");
        
        // Test with error
        printf("\nTesting CRC verification (with error):\n");
        uint8_t corrupted_codeword[MAX_DATA_SIZE];
        memcpy(corrupted_codeword, result.codeword, result.codeword_size);
        corrupted_codeword[2] = !corrupted_codeword[2]; // Flip one bit
        
        crc_result_t error_result = verify_crc(corrupted_codeword, result.codeword_size, key, key_size);
        print_error_status(error_result.status);
        printf("Error detected: %s\n", error_result.error_detected ? "YES" : "NO");
        print_binary_array(error_result.remainder, key_size - 1, "Error remainder");
    }
}

void test_string_functions() {
    printf("\n=== TESTING STRING CONVENIENCE FUNCTIONS ===\n");
    
    // Test string to binary conversion
    const char *test_strings[] = {"1010", "1101", "0110"};
    char checksum_str[20];
    char *output_data[4];
    char output_buffers[4][20];
    
    // Allocate output buffers
    for (int i = 0; i < 4; i++) {
        output_data[i] = output_buffers[i];
    }
    
    printf("Testing string-based checksum function:\n");
    for (int i = 0; i < 3; i++) {
        printf("Input block %d: %s\n", i + 1, test_strings[i]);
    }
    
    int result = checksum_string(test_strings, 3, 4, checksum_str, output_data, 20);
    
    if (result == EDC_SUCCESS) {
        printf("Checksum: %s\n", checksum_str);
        printf("Data with checksum:\n");
        for (int i = 0; i < 4; i++) {
            printf("  Block %d: %s\n", i + 1, output_data[i]);
        }
    } else {
        printf("Error in string checksum function: %d\n", result);
    }
}

void test_error_conditions() {
    printf("\n=== TESTING ERROR CONDITIONS ===\n");
    
    // Test null pointer
    printf("Testing null pointer handling:\n");
    checksum_result_t result1 = calculate_checksum(NULL, 3, 4);
    print_error_status(result1.status);
    
    // Test invalid block size
    printf("Testing invalid block size:\n");
    uint8_t dummy_data[1][MAX_BLOCK_SIZE];
    checksum_result_t result2 = calculate_checksum(dummy_data, 3, 0);
    print_error_status(result2.status);
    
    // Test size exceeded
    printf("Testing size exceeded:\n");
    checksum_result_t result3 = calculate_checksum(dummy_data, MAX_BLOCKS + 1, 4);
    print_error_status(result3.status);
    
    // Test invalid binary data
    printf("Testing invalid binary data:\n");
    uint8_t invalid_data[2][MAX_BLOCK_SIZE] = {{1, 0, 2, 0}, {1, 1, 0, 1}}; // Contains '2'
    checksum_result_t result4 = calculate_checksum(invalid_data, 2, 4);
    print_error_status(result4.status);
}

int main() {
    printf("TESTING REFACTORED ERROR DETECTION CODES\n");
    printf("==========================================\n");
    
    test_checksum();
    test_lrc();
    test_vrc();
    test_crc();
    test_string_functions();
    test_error_conditions();
    
    printf("\n==========================================\n");
    printf("ALL TESTS COMPLETED\n");
    
    return 0;
}
