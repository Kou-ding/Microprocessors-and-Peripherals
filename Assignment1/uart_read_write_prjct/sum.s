.global sum_of_natural_numbers
	
sum_of_natural_numbers:
    mov r1, #0      // Initialize sum to 0
    mov r2, #0      // Initialize counter to 0
	
loop:
    add r1, r1, r2  // Add counter to sum
    add r2, r2, #1  // Increment counter
    cmp r2, r0      // Compare counter with input number
    ble loop        // If counter <= input number, loop again
    mov r0, r1      // Move result to r0 (return register)
    bx lr           // Return from function