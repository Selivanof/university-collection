// Hashing Function ARM Implementation
// Author: Lamprinos Chatziioannou
// Description: For Lab1: MP

	.global hashfunc
	.p2align 2
	
	.section .data
values:
	.byte 10,42,12,21,7,5,67,48,69,2,36,3,19,1,14,51,71,8,26,54,75,15,6,59,13,25
	
	.section .text
	.type hashfunc,%function	
	
	
// In this asm file I perform extensive usage of labels, mainly for readability.
// By no means could this be considered optimized nor best practice

hashfunc: 
	.fnstart
	push {r4,lr}  // freeing r4 to use later on
	// using registers:
	// r0: address of input string
	// r1: current character
	// r2: --- no usage - should make it the hash
	// r3: memory address of  values
	// r4: hash
	//add r0, r0, r1
	mov r4, #0 // Setting initial value of 0 for the hash
	ldr r3, =values
	
loop:
	// Load the latest value and increment r0 to point to next char
	ldrb r1, [r0], #1
	cmp r1, #0
	beq end  // break from loop if the null terminator is found
	
	// If less than A, possibly a digit
	cmp r1, #'A'
	blt check_digit
	
	// If more than Z, possibly a lowercase
	cmp r1, #'Z'
	bgt check_lowercase
	
	// Convert to index
	sub r1, r1, #'A'
	ldrb r2, [r3, r1]
	add r4,r4,r2
	
	b loop
	
check_lowercase:
	// Ensuring that we're in the proper range
	cmp r1, #'a'
	blt loop
	cmp r1, #'z'
	bgt loop
	
	// Convert to index
	sub r1, r1, #'a'
	ldrb r2, [r3, r1]
	sub r4, r4,r2
	
	b loop
	
check_digit:
	// Ensuring proper range
	cmp r1, #'0'
	blt loop
	cmp r1, #'9'
	bgt loop
	
	// Convert to index
	sub r1, r1, #'0'
	add r4, r4, r1	
	b loop
end:
	mov r0, r4
	pop {r4,pc}
	.fnend

