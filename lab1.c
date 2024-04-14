#include <stdio.h>

int hash_function(char str[]);
int sum_of_natural_numbers(int hash);

int main() {
    printf("Input a string(max 100 characters):\n");
    char str[100];
    scanf("%s", str);
    // printf("The string you entered is: %s\n", str); //for debugging

    // The hash of the input string
    printf("The hash is: %d\n", hash_function(str));

    // The sum of all the hash's previous numbers: 1+2+...+hash
    printf("The result is: %d\n", sum_of_natural_numbers(5));
    return 0;
}

int hash_function(char str[]){ //string into single-digit hash
    // Your code here
    int hash=0;
    return hash;
}

int sum_of_natural_numbers(int hash){ //single-digit hash into sum_of_natural_numbers()
    /*
    int res=0;
    for (int i=0; i<=hash; i++){
        res=res+i;
    }
    */
    asm (
        "mov %[res], #0\n" // Initialize sum to 0
        "mov r2, #1\n"     // Initialize counter to 1
        "1:\n"
        "add %[res], %[res], r2\n" // Add counter to sum
        "add r2, r2, #1\n"         // Increment counter
        "cmp r2, %[hash]\n"        // Compare counter with input number
        "ble 1b\n"                 // If counter <= input number, loop again
        : [res] "+r" (res)
        : [hash] "r" (hash)
        : "r2"
    );
    return res;
}
/*
.global sum_of_natural_numbers
sum_of_natural_numbers:
    mov r1, #0      // Initialize sum to 0
    mov r2, #1      // Initialize counter to 1
loop:
    add r1, r1, r2  // Add counter to sum
    add r2, r2, #1  // Increment counter
    cmp r2, r0      // Compare counter with input number
    ble loop        // If counter <= input number, loop again
    mov r0, r1      // Move result to r0 (return register)
    bx lr           // Return from function
*/