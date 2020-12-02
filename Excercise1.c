__asm int my_strchck(const char *str)
{	
		
string_length
		LDRB r2,[r0,r5]		//Load byte into r2 from memory pointed to by r0 + r5
		ADDS r5,#1		//Increment pointer offset
		CMP r2,#0		//Is the byte 0? 
		BNE string_length		//If not, repeat the loop
	
		SUBS r5,#2		//Substracting 2 from r5 gives us the string length
		B for_loop		//Branch to palindrome checking routine

main_body
		LDRB r1, [r0,r3]		//Load byte into r1 from memory pointed to by r0 + r3
		LDRB r2, [r0,r5]		//Load byte into r2 from memory pointed to by r0 + r5
		SUB r5,#1		//Decrement string-end pointer
		ADDS r3,#1		//Increment string-start pointer 
		CMP r1,r2		//Are the characters the same?
		BNE not_palindrome		//If not, then our work here is done
		B for_loop		//Else continue checking
	

for_loop	
		CMP r3,r5		//Compare the memory offsets
		BGE terminate		//Brances to 'terminate' when i have checked the whole string succesfully 
		B main_body		//Else, if not finished, jump to the main body
	
not_palindrome
		MOVS r0, #0		//Save the value 0 in r0 for 'not palindrome string'
		BX lr		//Return from subroutine 
		
terminate
		MOVS r0, #1		//Save the value 1 in r0 for 'palindrome string'
		BX lr		//Return from subroutine
	}
		
int __main(void)
{
	const char str[] = "aababbabaa";
	int flag = my_strchck(str);

	while (1);
}