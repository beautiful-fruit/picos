# User Function 定義教學

## 為什麼需要特別使用 User Function

使用 XC8 提供的軟體堆疊在某些情況下會出現不可接受的行為，例如 **context switch** 需要額外保存 56 個 btemp，會大幅增加記憶體使用量。雖然編譯堆疊（compile stack）沒有這個問題，但它 **不支援重入**，而重入可能在行程切換時發生（例如：多個行程同時執行 `printf`）。因此，對使用者程式而言，標準堆疊無法滿足需求。

為了解決這個問題，我們必須 **實作自訂堆疊**，用來存放函式區域變數、輸入參數以及相關暫存器。

---

## 使用規範

1. **定義宏（macro）呼叫函式**

   * 使用宏接收輸入參數，並提供儲存回傳值的變數（若函式無回傳可忽略）。
   * 宏會呼叫實際執行的函式。

2. **實際執行的函式**

   * 必須使用 `naked` 屬性，並且輸入與回傳型別都為 `void`。

3. **呼叫前的準備**

   * 調整 `sp`（stack pointer）至適當位置，將參數放入堆疊。

4. **返回地址管理**

   * 調整 `sp` 時，需為返回地址預留 3 個位元組。
   * 在堆疊成長空間中，最前面的 3 個位元組存放返回地址。
   * 除非有特殊需求，請勿在函式內修改這個區域。

5. **函式進入與退出**

   * 進入函式時使用 `enter_user_func`。
   * 回傳值使用 `return_user_func`，若有回傳值，請確保已寫入堆疊對應位置。

6. **增強可讀性**

   * 可使用 `#define` 與 `#undef` 來整理與增加程式可讀性。

7. **函式返回後的處理**

   * 若函式有回傳值，將其從堆疊取出，存入負責存放回傳值的變數。
   * 歸還所有使用的堆疊空間。

---

## 範例
```c
#define test_add(arg1, arg2, output) \
    do {                             \
        sp += 6;                     \
        *(sp - (6 - 3)) = arg1;      \
        *(sp - (6 - 4)) = arg2;      \
        asm("CALL _test_add_impl");  \
        output = *(sp - (6 - 5));    \
        sp -= 6;                     \
    } while (0)

void __attribute__((naked)) test_add_impl(void)
{
#define ret (*(sp - (6 - 5)))
#define arg1 (*(sp - (6 - 3)))
#define arg2 (*(sp - (6 - 4)))
    enter_user_func(6);
    ret = arg1 + arg2;
#undef ret
#undef arg1
#undef arg2
    return_user_func(6);
}
```