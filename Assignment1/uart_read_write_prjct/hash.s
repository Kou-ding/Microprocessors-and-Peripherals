.global hash
	
// lookup table for the hash 
.section .data 	//informs the comp that this goes to where data is saved 
table: 
		.word 10,42,12,21,7,5,67,48,69,2,36,3,19,1,14,51,71,8,26,54,75,15,6,59,13,25
myNumber: .word
			
.section .text
	
hash:
	PUSH {r4,lr}	//save r4, lr to stack

	MOV r1, #0  	//set r1=0. r1 is the result 
	MOV r2, #0		//r2 will be used to count the index in the input array 
	ldr r3, =table 	//r3 point to start of "table"
	
	
//this loop takes one character at a time and through consecutive compares of it's ASCII value 
//finds it's type: lowercase letter, Uppercase letter, number or a character that doesnt change the hash. 
//when the character's type is found i jusmp to a block of code that changes the result accordingly. 
//after that i loop again until the last character of the input string	
loop:
	ldrb r4, [r0]	//loads value found in r0 into r4. this value is the first character of input during the first loop
		
	cmp r4, #0		//if i read 0 it means end of input 
	beq end_loop
	
	cmp r4, #'z'+1 
	bcs nouln	// if the character is higher or equal than z+1 in ascii, hash in not changed
	
	cmp r4, #'a'
	bcs lowerCase	//if the character is between [97,122] it is a lower case letter and hash is changed 
		
	cmp r4, #'Z'+1
	bcs nouln 	//if the character is between [91,96] it is not a lower or upper case letter or a number so hash is unchanged 
	
	cmp r4, #'A' 
	bcs upperCase 	//if the caracter is between [65,90] it is upper case letter and the hash is changed 
	
	cmp r4, '9'+1 
	bcs nouln	//if the character is between [58,64] it is not a lower or upper case letter or a number so hash is unchanged 
	
	cmp r4, '0'
	bcs number	//if the caracter is between [48,57] it is upper case letter and the hash is changed
	
	b nouln		//if the caracter is not any of the above then the hash isn't changed 
	
lowerCase:
	SUB r2, r4, #'a'	//r2 becomes the index of the letter in the table | r4 = character | r3 points to table start 
	LSL r2, #2			//multiply index by 4 
	MOV r4, r3			//r4 points to the start of the table
	ADD r4, r2			//r4 points to the coresponding value of the letter i am reading 
	ldr r2, [r4]			//loads value that will be subed from the hash
	SUB r1, r2			//the corespoding value of the letter is subtracted by the hash 
	b nouln
	
upperCase:				//the current character is a lowercase letter and changes the hash accordingly 
	SUB r2, r4, #'A'	//r2 becomes the index of the letter in the table | r4 = character | r3 points to table start 
	LSL r2, #2			//multiply index by 4 since every index increment coresponds to 4 bytes
	MOV r4, r3			//r4 points to the start of the table
	ADD r4, r2			//r4 points to the coresponding value of the letter i am reading 
	ldr r2, [r4]		//loads value that will be subed from the hash
	ADD r1, r2			//the corespoding value of the letter is added to the hash 
	b nouln	
	
number:					//the current character is a number so it's value is added to the hash
	SUB r2, r4, #'0'	//r2 becomes the value i am adding | r4 = character | r3 points to table start 
	ADD r1, r2			//add the number to the hash 
	b nouln	
	
nouln:					//the current character dosn't affect the hash value
	ADD r0, #1			//go to the next character 
	B loop

end_loop:
	MOV r0, r1			//return result in r0 
    POP {r4, lr}		//return r4 to original value and pop lr to prepare to go back to caller
    BX lr				//ruturn from this function
