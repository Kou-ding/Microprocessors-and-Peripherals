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
    printf("The result is: %d\n", sum_of_natural_numbers(hash_function(str)));
    return 0;
}

int hash_function(char str[]){ //string into single-digit hash
    // Your code here
    int hash=0;
    return hash;
}

int sum_of_natural_numbers(int hash){ //single-digit hash into sum_of_natural_numbers()
    
    int res=0;
    return res;
}