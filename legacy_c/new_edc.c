#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
