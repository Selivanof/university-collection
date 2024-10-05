// Function that calculates the digital root of an integer
// Author: Georgios Selivanof
// Description: For Lab1: MP

// More information about the calucation of the reciprocal: 
// Division by Invariant Integers using Multiplication by Torbjorn Granlund, Peter L. Montgomery (https://gmplib.org/%7Etege/divcnst-pldi94.pdf)

.global digital_root
.p2align 2

.section .text
.type digital_root, %function


digital_root:
	.fnstart
	push {lr}
	                        // Using registers:
	                        // r0: Input Argument, Return Value
	                        // r1: Reciprocal
	                        // r2: Quotient of division by 9
	
	ldr r1,  reciprocal     // Load the reciprocal into r1
	mov r2, #0              // Initialize the quotient (r2) to 0
	
	cmp r0, #0            	// === Special cases ===
	beq.N end               // If the input (r0) is 0, return 0
	it lt                   // If the input (r0) is negative
	rsblt r0, r0, #0        // Convert negative input (r0) to positive
	                        // === Division By 9 ===
	umull r3, r2, r1, r0    // Multiply the input (r0) with the reciprocal (r1)
	lsr r2, r2, #1          // Right shift on the lower 32 bits of the result (r2). r2 is now the quotient of r0 / 9
	                        // === Find the Remainder ===
	add r2, r2, r2, lsl #3  // Multiply the quotient (r2) by 9
	sub r0, r0, r2          // Remove the quotient * 9 (r2) from the input (r0). r0 is now the remainder of r0 / 9
	
	cmp.N r0, #0            
	it eq                   // If the remainder (r0) is 0
	moveq r0, #9            // Change the result (r0) to 9
end:
	pop {pc}
	.fnend
	
reciprocal:
	.word 954437177         // = (2^33) / 9
