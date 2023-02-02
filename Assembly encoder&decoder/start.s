section .data
    encode_flag: db 0
    encoded_input: db 0
    buffer_stream: db 0
    input: dd 0
    output: dd 1

section .bss
    code resd 1          ;reserves 4 bytes one time
section .text
global main
main:
    push ebp
    mov ebp, esp
    mov esi, [ebp+12]
    mov edi, [ebp+8]

define_program_args:
    cmp edi, 1
    je no_args
    dec edi
    add esi, 4
    mov eax, [esi]
    
    cmp word[eax], '+e' ;2 bytes
    je get_code
    
    cmp word[eax], '-i'
    je get_input_file
    
    cmp word[eax], '-o'
    je get_output_file
   
invalid_input:
    pop ebp
    ret
    
get_input_file:
    mov eax, [esi]
    add eax, 2
    
    pushad
    
    mov ebx, eax   
    mov eax, 5      
    mov ecx, 0              
    int 0x80
    mov dword[input], eax
    
    popad
    
    jmp define_program_args

get_output_file:
    mov eax, [esi]
    add eax, 2
    
    pushad
    
    mov ebx, eax
    mov eax, 5 
    mov ecx, 0x42
    int 0x80
    mov dword[output], eax
    
    popad
    
    jmp define_program_args

   
get_code:
    mov byte[encode_flag], 1
    mov eax, [esi]
    add eax, 2
    mov dword[code], eax
    jmp define_program_args
    
encode_input:
    cmp byte[buffer_stream], 10 ; ;skip line
    je print_output
    
    mov bl, byte[edi]   
    sub bl, '0'            
    add byte[buffer_stream], bl
    inc edi
    cmp byte[edi], 0
    jne print_output
    mov edi, dword[code]
    jmp print_output
    
    
no_args:
    mov edi, dword[code] ; edi containts the code
    jmp read_input
    
read_input:

    pushad

    mov eax, 3
    mov ebx, dword[input]
    mov ecx, buffer_stream
    mov edx, 1
    int 0x80
    
    cmp eax, 0
    je end_program
    
    popad
    
    cmp byte[encode_flag], 1
    je encode_input
    
    mov bl, byte[buffer_stream]
    cmp bl, 'a'
    jl print_output
    cmp bl, 'z'
    jg print_output
    mov byte[buffer_stream], bl
    sub byte[buffer_stream], 32
    jmp print_output
    
print_output:
    pushad
    
    mov eax, 4   
    mov ebx, dword[output]
    mov ecx, buffer_stream 
    mov edx, 1           
    int 0x80
    
    popad
    jmp read_input
    
    
end_program:
    popad
    pop ebp
    ret
