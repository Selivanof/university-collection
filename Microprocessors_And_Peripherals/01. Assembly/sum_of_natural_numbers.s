// Function that calculates the sum of natural numbers
// Author: Georgios Selivanof
// Description: For Lab1: MP

.global sum_of_natural_numbers
.p2align 2

.section .text
.type sum_of_natural_numbers, %function


sum_of_natural_numbers:
	push {lr}
	                    // Using registers:
	                    // r0: Input Argument, Return Value
	                    // r1: Sum/Result
				
	mov r1, #0          // Initialize the sum (r1) to 0
 	cmp r0, #0          // If the current input (r0) is 0, return
	ble end
loop:
	add r1, r1, r0      // Add the current input (r0) to the sum (r1)
	subs r0, r0, #1      // Remove 1 from the current input (r0)
	bne loop              // Repeat
end:
	mov r0, r1          // Move the sum (r1) to the return register (r0)
	pop {pc}
