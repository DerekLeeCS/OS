.globl _start


	.text
_start:
	mov $1, %rax		# write syscall
	mov $1, %rdx		# stdout
	mov $str, %rsi		# "Hello World!\n"
	mov $13, %rdx		# 13 chars
	syscall			# kernel

	mov $60, %rax		# _exit
	mov $0, %rdi		# EXIT_SUCCESS
	syscall			# kernel


	.data
str: .ascii "Hello World!\n"
