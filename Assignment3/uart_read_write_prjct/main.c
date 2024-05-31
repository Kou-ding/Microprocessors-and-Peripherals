#include "platform.h"
#include <stdio.h>
#include <stdint.h>
#include "uart.h"
#include <string.h>
#include "queue.h"
#include "gpio.h"
#include "leds.h"
#include "timer.h"
#include "delay.h"

#define BUFF_SIZE 128 //read buffer length

Queue rx_queue; // Queue for storing received characters

//Public variables
int touch_pressed = 0;
int touch_counter = 0; 
int moist_samp = 0;
char my_printer[10]; //we create a printer array to be able to print the last digit later
int freq;
int counter;

uint16_t SUM, rh, temp;
uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;

float Temperature = 0;
float Humidity = 0;
uint8_t Presence = 0;

// Interrupt Service Routine for UART receive
void uart_rx_isr(uint8_t rx) {
	// Check if the received character is a printable ASCII character
	if (rx >= 0x0 && rx <= 0x20 || rx >= 0x30 && rx <= 0x39 || rx==0x7F) { //modified the range of characters allowed. Now it accepts only numbers, backspace and enter(cr: carriage return)
		// Store the received character
		queue_enqueue(&rx_queue, rx);
	}
}

void touch_isr() {
	touch_pressed = 1;
	//uart_print("pressed \r\n");
}

//inverts the led's state and prints the new state
void toggle_led(){
	gpio_toggle(P_LED_R);
}

//start signal from mcu according to dht11 datasheet
void dht11_init(){
	// dht11 receives the start signal from the mcu
	gpio_set_mode(PA_0, Output); // pin mode -> output 
	gpio_set(PA_0, 0); //the mcu sends a start signal to the dht11 
	delay_ms(18); // sets the pin's voltage level low for 18ms
	gpio_set(PA_0, 1);
	delay_us(20); // sets the pin's voltage high for 20-40 us
	gpio_set_mode(PA_0, Input); // pin mode -> input
}

//now that the pin is set to input we have to check if we are getting a correct response
uint8_t dht11_check_response(){
	// dht11 sends a response signal to the mcu
	uint8_t response=0;
	delay_us(40); //wait 40us to reach the middle of the 80us constant low voltage period
	if(gpio_get(PA_0)==0){ //check if the pin is 0
		delay_us(80); //wait 80us to reach the middle of the 80us constant high voltage period
		if(gpio_get(PA_0)==1){ //check if the pin is 1
			response=1; //we successfully have a response since we passed all checks
		}else{
			response=-1; //we don't have a response
		}
	}
	while(gpio_get(PA_0)==1); //wait for pin to go low, that is the time the data begins flowing to the mcu
	return response;
}

//after enabling the sensor and checking for a response we get the data out of it
uint8_t dht11_read(){
	int i,j;
	//traverse the 8 bits one by one 
	for(j=0;j<8;j++){ //read 8 bits = 1 byte
		while(!gpio_get(PA_0)); //wait for the pin to go high(waiting for the 50us constant low voltage to pass)
		delay_us(40); //70us of high voltage is 1, 26-28us of high voltage is 0 according to the datasheet
		if(gpio_get(PA_0)==0){ 
			i&= ~(1<<(7-j)); //configures the (j+1)'th bit to be 0
		}else{
			i|= (1<<(7-j)); //configures the (j+1)'th bit to be 1
		}
		while(gpio_get(PA_0)); //wait for the pin to go low
	}
	return i;
}

void temp_sampling(){
	if(counter!=freq-1){
		counter++;
	}else{
		counter=0;
		//sample temperature
		dht11_init();
		Presence = dht11_check_response();
		Rh_byte1 = dht11_read();
		Rh_byte2 = dht11_read();
		Temp_byte1 = dht11_read();
		Temp_byte2 = dht11_read();
		SUM = dht11_read();

		temp = Temp_byte1;
		rh = Rh_byte1;
		Temperature = (float) temp;
		Humidity = (float) rh;
		
		//print temperature
		sprintf(my_printer, "%.2f",Temperature); //int to char conversion
		uart_print("Temperature: ");
		uart_print(my_printer); //uart_print takes char as a parameter thus we need to move the result to a printer char array
		uart_print("C\r\n");
		
		//print sampling rate
		sprintf(my_printer, "%d", freq); //int to char conversion
		uart_print("Sampling rate: ");
		uart_print(my_printer); //uart_print takes char as a parameter thus we need to move the result to a printer char array
		uart_print("s\r\n");
		
		if( moist_samp == 1){
			//print humidity
			sprintf(my_printer, "%.2f", Humidity); //int to char conversion
			uart_print("Humidity: ");
			uart_print(my_printer); //uart_print takes char as a parameter thus we need to move the result to a printer char array
			uart_print("%\r\n");
		}
	}
}

int main() {
	//////////////our-code///////////////
	int sum_last_two_digits = 0;
	
	//initialize the led
	leds_init();
	
	//initialize the button's parameters
	gpio_set_mode(P_SW, Input);
	gpio_set_mode(P_SW, PullUp);
	gpio_set_trigger(P_SW, Rising);
	
	gpio_set_callback(P_SW, touch_isr);
	////////////////////////////////////
	
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
		uart_print("Enter your AEM: ");
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
		
		///////////////////////our-code/////////////////////////
		if(strcmp(buff, "\0") == 0){ //check if buffer is empty by comparing it with the null terminator which is always the last character
			uart_print("Wrong format.");
			uart_print("\r\n");// Print newline
		}
		else{
			char* aem = buff ; //Inside the buffer there is the aem
			int my_number=atoi(aem); //we convert the aem into an integer
			int sum_last_two_digits=(my_number%10)+((my_number/10)%10); //the sum of the two last digits of the aem
			
			//print the sum of the last two digits
			sprintf(my_printer, "%d", sum_last_two_digits); //int to char conversion
			uart_print("The AEM's last-two-digit sum is: ");
			uart_print(my_printer); //uart_print takes char as a parameter thus we need to move the result to a printer char array
			uart_print("\r\n");
			
			//change the sum to correctly affect the frequency
			if(sum_last_two_digits==2){
				sum_last_two_digits=4;
			}
			
			// Set callback to toggle LED
			timer_set_callback(temp_sampling); //this interrupt is activated whenever the timer is enabled
			timer_init(1000000); //initialization of the timer to 1.000.000us or 1s. This means it ticks every 1s.
			freq=2;
			timer_enable();
			
			while(1){
				// temperature-wise changes of the LED
				if(temp>25){
					gpio_set(P_LED_R, 1);
				}
				else if(temp<20){
					gpio_set(P_LED_R, 0);
				}
				else{
					toggle_led();
					delay_ms(1000);
				}
				
				//if the touch sensor is pressed
				if (touch_pressed == 1) {
					touch_counter ++;
					if (touch_counter < 2){
						freq = sum_last_two_digits;
					}else{
						//if touched odd times
						if (touch_counter % 2 != 0){
							freq = 3;
							moist_samp = 0;							
						}
						//if touched even times
						else{
							freq = sum_last_two_digits;
							moist_samp = 1;				
						}
					}
					counter = 0;
					touch_pressed = 0;
				}
			}
		}
	}
}
