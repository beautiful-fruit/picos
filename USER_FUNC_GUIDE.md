# User Function Definition Guide

## Why User Functions Are Needed

Using the software stack provided by XC8 can exhibit unacceptable behaviors. For example, **context switching** requires saving an additional 56 B of temporary memory, which significantly increases memory usage. Although the compile stack does not have this problem, it **does not support reentrancy**, which may occur during process switching (e.g., multiple processes executing `printf`). Therefore, the standard stack cannot meet the needs of user programs.

To address this, we must **implement a custom stack** to store function local variables, input parameters, and relevant registers.

---

## Usage Guidelines

1. **Define a Macro to Call the Function**

   * The macro should accept input parameters and provide storage for a return value (if the function has no return value, this can be ignored).
   * The macro calls the actual implementation function.

2. **Implementation Function Requirements**

   * Must be defined with the `naked` attribute.
   * Both input and return types must be `void`.

3. **Preparation Before Calling**
   1. Place Parameters and Return Value Area
      * Before modifying `sp`, store all input parameters and reserve space for the return value on the stack.
      ```c
      *(current->sp) = arg1;
      *(current->sp + 1) = arg2;
      *(current->sp + 2) = 0; // optional: reserve space for return value
      ```
   2. Adjust `sp` to the Appropriate Value
4. **Return Address Management**

   * Reserve 3 bytes for the return address when adjusting `sp`.
   * In the stack growth area, the last 3 bytes store the return address.
   * Do not modify this area inside the function unless necessary.

5. **Function Entry and Exit**

   * Use `enter_user_func` when entering the function.
   * Use `return_user_func` for returning; if the function has a return value, make sure it is written to the designated location in the stack.

6. **Enhancing Readability**

   * You may use `#define` and `#undef` to improve code readability and organization.

7. **After Function Return**

   * If the function has a return value, read it from the stack and store it into the designated variable.

   * Finally, release the used stack space (restore `sp` to its original value).
---

## example
```c
#define test_add(arg1, arg2, output) \
    do {                             \
        *(current->sp) = arg1;   \
        *(current->sp + 1) = arg2;   \
        current->sp += 6;            \
        asm("CALL _test_add_impl");  \
        current->sp -= 6;            \
        output = *(current->sp + 2); \
    } while (0)

void __attribute__((naked)) test_add_impl(void)
{
#define ret (*(current->sp - 4))
#define arg1 (*(current->sp - 6))
#define arg2 (*(current->sp - 5))
    enter_user_func();
    ret = arg1 + arg2;
#undef ret
#undef arg1
#undef arg2
    return_user_func();
}
```
