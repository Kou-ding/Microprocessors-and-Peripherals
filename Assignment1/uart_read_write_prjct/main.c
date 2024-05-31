#include "platform.h"
#include <stdio.h>
#include <stdint.h>
#include "uart.h"
#include <string.h>
#include "queue.h"


#define BUFF_SIZE 128 //read buffer length

Queue rx_queue; // Queue for storing received characters

// Interrupt Service Routine for UART receive
void uart_rx_isr(uint8_t rx) {
	// Check if the received character is a printable ASCII character
	if (rx >= 0x0 && rx <= 0x7F ) {
		// Store the received character
		queue_enqueue(&rx_queue, rx);
	}
}

extern int hash(char*);
extern int single_digit(int hashRes);
extern int sum_of_natural_numbers(int hash);

int main() {
		
	// Variables to help with UART read
	uint8_t rx_char = 0;
	char buff[BUFF_SIZE]; // The UART read string will be stored here
	uint32_t buff_index;
	
	// Initialize the receive queue and UART
	queue_init(&rx_queue, 128);
	uart_init(115200);
	uart_set_rx_callback(uart_rx_isr); // Set the UART receive callback function
	uart_enable(); // Enable UART module
	
	__enable_irq(); // Enable interrupts
	
	uart_print("\r\n");// Print newline
	
	while(1) {

		// Prompt the user to enter their full name
		uart_print("Input a string(max 128 characters):");
		buff_index = 0; // Reset buffer index
		
		do {
			// Wait until a character is received in the queue
			while (!queue_dequeue(&rx_queue, &rx_char))
				__WFI(); // Wait for Interrupt

			if (rx_char == 0x7F) { // Handle backspace character
				if (buff_index > 0) {
					buff_index--; // Move buffer index back
					uart_tx(rx_char); // Send backspace character to erase on terminal
				}
			} else {
				// Store and echo the received character back
				buff[buff_index++] = (char)rx_char; // Store character in buffer
				uart_tx(rx_char); // Echo character back to terminal
			}
		} while (rx_char != '\r' && buff_index < BUFF_SIZE); // Continue until Enter key or buffer full
		
		// Replace the last character with null terminator to make it a valid C string
		buff[buff_index - 1] = '\0';
		uart_print("\r\n"); // Print newline
		
	/////////original code in uart/////////////
	char* randomWord = buff ;
	char my_printer[10];  //buffer to print the int results as strings
	
	// The hash of the input string
	int hashRes = hash(randomWord);
	//print hashRes
	sprintf(my_printer, "%d", hashRes);
	uart_print("The hash is:");
	uart_print(my_printer);
	uart_print("\r\n");
		
	// In case the hash is negative 
	if(hashRes<0){
		hashRes=-hashRes;
	}
	
	// Convert the hash into a single digit
	int singleHash = single_digit(hashRes);
	//print singleHash
	sprintf(my_printer, "%d", singleHash);
	uart_print("The single digit hash is:");
	uart_print(my_printer);
	uart_print("\r\n");
	
	// The sum of all the hash's previous numbers: 1+2+...+hash
	int result=sum_of_natural_numbers(singleHash);
	sprintf(my_printer, "%d", result);
	uart_print("The final result is:");
	uart_print(my_printer);
	uart_print("\r\n");
	///////////////////////////////////////////////////////////
	
		// Check if buffer overflow occurred
		if (buff_index > BUFF_SIZE) {
			uart_print("Stop trying to overflow my buffer! I resent that!\r\n");
		}
	}
}
