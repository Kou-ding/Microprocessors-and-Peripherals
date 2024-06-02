#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "platform.h"
#include "uart.h"
#include "queue.h"
#include "platform.h"
#include "gpio.h"
#include "leds.h"
#include "timer.h"

#define BUFF_SIZE 128 //read buffer length

Queue rx_queue; // Queue for storing received characters

int button_count = 0;	//ADC Counter for the number of times the board button has been pressed

// Interrupt Service Routine for UART receive
void uart_rx_isr(uint8_t rx) {
	// Check if the received character is a printable ASCII character
	if (rx >= 0x30 && rx <= 0x39 || rx==0x7F || rx==0x0D) { //modified the range of characters allowed. Now it accepts only numbers, backspace and enter(cr: carriage return)
		// Store the received character
		queue_enqueue(&rx_queue, rx);
	}
}

// This function prints the number of times that the board button has been pressed
void print_button()	{
	
	char to_print[10];
	sprintf(to_print, "%d", button_count);
	
	uart_print("The button has been pressed ");
	uart_print(to_print);
	uart_print(" times\r\n");
}

// Print the current led state: "LED ON" when the led is on and "LED OFF" when the led is off
// by checking whether the P_LED_R pin, which is the led pin, is 0 (led off) or 1( led on)
void print_led_state() {	
	gpio_get(P_LED_R) != 0 ? uart_print("LED ON") : uart_print("LED OFF");
	uart_print("\r\n");
}

//inverts the led's state and prints the new state
void toggle_led(){
	gpio_toggle(P_LED_R);
	print_led_state();
	}

//Increase counter that tracks the number that the button is pressed 
//inverts led 
//prints counter
void button_press_isr(){
	button_count ++;
	toggle_led();
	print_button();
}

// Callback for odd AEM: if aem's last digit is odd, toggle the led every half a second
void odd_aem(){
    // Enable timer and toggle LED every half a second
    timer_enable();
}

// Callback for even AEM: if aem's last digit is even, disable the timer and keep the led's state
void even_aem(){
    // Disable timer and keep LED state
    timer_disable();
}

int main() {
	/*********************our-code***********************/
	//initialize the led
	leds_init();

	//initializes the button and sets it to pullup and it's trigger to rising
	gpio_set_mode(P_SW, PullUp);
	gpio_set_trigger(P_SW, Rising);
	//callback when the putton is pressed
	gpio_set_callback(P_SW, button_press_isr);

	/***************************************************/

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

		// Prompt the user to enter their AEM
		uart_print("Enter your AEM:");
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

	// Check if buffer overflow occurred
			if (buff_index > BUFF_SIZE) {
				uart_print("Stop trying to overflow my buffer! I resent that!\r\n");
			}
			
			/////////original code /////////////
			if(strcmp(buff, "\0") == 0){ //check if buffer is empty by comparing it with the null terminator which is always the last character
				uart_print("Wrong format.");
				uart_print("\r\n");// Print newline
			}
			else{
				char* aem = buff ; //Inside the buffer there is the aem
				char my_printer[10]; //we create a printer array to be able to print the last digit later
				
				int my_number=atoi(aem); //we convert the aem into an integer
				int last_digit=my_number%10; //we find out the last digit of the aem
				
				//print the aem's last digit
				sprintf(my_printer, "%d", last_digit); //int to char conversion
				uart_print("The AEM's last digit is:");
				uart_print(my_printer); //uart_print takes char as a parameter thus we need to move the result to a printer char array
				uart_print("\r\n");
				
				// Set callback to toggle LED
				timer_set_callback(toggle_led); //this interrupt is activated whenever the timer is enabled
				timer_init(500000); //initialization of the timer to 500.000us or 0,5s. This means it ticks every 0,5s.
				
			// Check if AEM's last digit is even or odd
				if (my_number % 2 == 0) {
						// Even AEM: Disables the timer and turns off the LED
						even_aem();
				} else {
						// Odd AEM: Enables the timer and toggles the LED every half a second
						odd_aem();
				}
			}
			/////////////////////////////////////
		}
}