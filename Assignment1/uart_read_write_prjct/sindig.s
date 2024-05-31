.global single_digit

single_digit:

add_digits_loop:        
    MOV r1, r0			// Store the input hash inside r1
	MOV r0, #0          // Reset r0 to accumulate the sum of digits

add_digits:
    MOV r2, #10         // Load 10 into r2 to use as a divisor
    UDIV r3, r1, r2     // Divide r1 by r2=10, quotient in r3
	
	MUL r2, r3, r2      // Multiply the quotient with the divisor
	SUB r2, r1, r2      // Subtract from the dividend to get the remainder
    ADD r0, r0, r2      // Add the remainder to r0
	
	MOV r1, r3
    CMP r3, #0          // Check if quotient is zero
    BEQ end_add_digits  // If quotient is zero then there are no more digits to add so exit the loop

    B add_digits        // Otherwise, continue adding digits

end_add_digits:
    CMP r0, #9          // Check if the sum is a single digit
    BHI add_digits_loop // If sum is not a single digit, repeat the process

    BX lr