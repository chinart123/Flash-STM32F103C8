# Debug: Keil báo `Contents mismatch` khi nạp STM32F103C8T6 qua ST-Link clone

Nhật ký debug theo đúng trình tự đã làm. Mỗi bước gồm: giả thuyết, cách kiểm tra, kết quả, kết luận.
Nhãn: **[Đã đo]** = có số liệu hoặc log thật. **[Suy luận]** = hợp lý nhưng chưa chứng minh trực tiếp.

---

## 0. Môi trường và triệu chứng

- Board: Blue Pill STM32F103C8T6 (Device ID `0x410`, Rev A, thạch anh 8 MHz)
- Mạch nạp: ST-Link V2 clone vỏ nhôm, firmware `V2J48S7`
- Keil MDK 5.43a, Compiler V5.06 và V6.24, DFP `Keil.STM32F1xx_DFP.2.4.1`
- Firmware: nháy LED PC13 bằng LL; `SystemClock_Config()` chạy HSE -> PLL x9 = 72 MHz
- Project tạo lại từ đầu, copy tay từng file từ `STM32Cube_FW_F1_V1.8.6`

Triệu chứng: bấm F8 trong Keil thì lỗi, nhưng ST-LINK Utility, `ST-LINK_CLI` và J-Link nạp cùng board đều được.

```text
Contents mismatch at: 08000003H  (Flash=00H  Required=20H) !
Error: Flash Download failed  -  "Cortex-M3"
```

Các biến thể gặp thêm:

```text
Load "...\Objects\STM32F103C8.axf"
Programming Failed!
Error: Flash Download failed  -  "Cortex-M3"
```

```text
Full Chip Erase Done.
Programming Done.
Contents mismatch at: 08000000H  (Flash=28H  Required=60H) !
Contents mismatch at: 08000008H  (Flash=60H  Required=75H) !
...
Too many errors to display !
```

Lưu ý: `Flash Download failed - "Cortex-M3"` là dòng chung của Keil. Nguyên nhân thật nằm ở các dòng phía trên nó.

---

## 1. Loại trừ các giả thuyết ban đầu

| Giả thuyết | Cách kiểm tra | Kết quả |
|---|---|---|
| Chip bị khóa RDP / Option Bytes lệch chuẩn | ST-LINK Utility -> Target -> Option Bytes | [Đã đo] RDP = Disabled, không trang nào bị write-protect. Loại. |
| Chip là hàng fake | Đọc `0xE000ED00` (CPUID) | [Đã đo] `0x411FC231`, khớp giá trị STM32F103 chính hãng (r1p1) theo hiểu biết của mình. Không có bằng chứng fake. |
| CMSIS 5/6 + LL gây lỗi | Xem vai trò của `core_cm3.h`, LL | [Suy luận] Đây chỉ là header lúc biên dịch, không chạy lúc nạp. Loại. |
| Compiler V6 | Đổi sang V5.06 | [Đã đo] Vẫn lỗi y hệt. Loại. |
| ST-Link clone hỏng | Cùng cục ST-Link nạp bằng Utility | [Đã đo] Nạp và Verify OK. Phần cứng ST-Link dùng được. |
| Tốc độ SWD cao | Hạ Max Clock xuống 100 kHz | [Đã đo] Không đổi. Loại. |
| Nguồn 3.3V yếu | Rút dây 3.3V, cấp nguồn riêng, đo | [Đã đo] Không đổi. Loại. |
| Cần tham số `-S` ("skip protection") | Cấu hình CLI đang chạy dùng `-V` (verify) | [Đã đo] Verify vẫn OK, không cần `-S`. Chưa xác nhận được định nghĩa chính thức của `-S`. |

Kết luận bước 1: các giải thích trong tài liệu `.md` ban đầu (chip fake, CMSIS 6, Compiler V6) không có cơ sở.

---

## 2. Cô lập: nhánh nạp nào lỗi

| Cách nạp | Kết quả |
|---|---|
| ST-LINK Utility | OK |
| `ST-LINK_CLI` làm External Tool trong Keil | OK |
| J-Link trong Keil | OK |
| Driver ST-Link của Keil (Use Target Driver) | **Lỗi** |

Đọc cấu hình thật trong `STM32F103C8.uvoptx`:

```text
pMon           = STLink\ST-LINKIII-KEIL_SWO.dll
Protocol       = 2        (SWD)
DbgClock       = 100000   (100 kHz)
Thuật toán nạp = -FF0STM32F10x_128.FLM -FS08000000 -FL020000
RAM thuật toán = -FD20000000 -FC1000   (0x20000000, 4 KB)
Thuật toán 2   = STM32F10x_OPT.FLM tại 0x1FFFF800 (Option Bytes, mặc định của DFP)
```

Kết luận bước 2: Keil dùng **DLL ST-Link riêng** (`TOOLS.INI` dòng `TDRV6`) và **file nạp `.FLM`**, khác hẳn đường của ST-LINK Utility. Chip và board không lỗi. Lỗi nằm ở tổ hợp "driver ST-Link của Keil + FLM".

---

## 3. Quan sát dữ liệu Flash đọc lại

Cách đọc bằng ST-LINK Utility:

```text
Address: 0x08000000   Size: 0x10   Data Width: 32 bits
```

Giá trị đúng (lấy từ lần nạp bằng CLI): `0x00 = 20000660`, `0x04 = 0800016D`, `0x08 = 08000175`, `0x0C = 08000177`, `0x10 = 08000179`.

| Địa chỉ | Đúng | Sau khi Keil nạp (hình 1, 2) | Sau khi CLI nạp (hình 3, 4) |
|---|---|---|---|
| `0x08000000` | `20000660` | `08000175` (sai) | `20000660` |
| `0x08000004` | `0800016D` | `0800016D` | `0800016D` |
| `0x08000008` | `08000175` | `20000660` (sai) / `08000175` | `08000175` |
| `0x0800000C` | `08000177` | `08000177` | `08000177` |
| `0x08000010` | `08000179` | `08000175` (sai) | `08000179` |

Nhận xét:

- [Đã đo] Chỉ các word ở địa chỉ chia hết cho 8 (word chẵn) bị sai; word lẻ không bao giờ sai.
- [Đã đo] Cùng một địa chỉ cho giá trị khác nhau giữa các lần đọc (khi bật LiveUpdate). Dữ liệu đã nằm trong Flash thì không tự đổi, nên lỗi ở **đường đọc**.
- [Suy luận] Flash F1 đọc theo cặp 2 word (64-bit). Word đầu của mỗi cặp trả về dữ liệu cũ của cặp trước: kiểu lỗi đọc thiếu wait state.

---

## 4. Giả thuyết: chip chạy 72 MHz nhưng Flash bị đặt 0 wait state

Wait state cần cho Flash STM32F1 (theo hiểu biết của mình về RM0008):

| SYSCLK | Wait state | `LATENCY` |
|---|---|---|
| 0 - 24 MHz | 0 | 0 |
| 24 - 48 MHz | 1 | 1 |
| 48 - 72 MHz | 2 | 2 |

**Test A**: sau một lần Keil nạp lỗi, đọc `RCC_CFGR` bằng ST-LINK Utility (`0x40021004`, Size `0x4`):

```text
0x40021004 = 001D040A
```

| Bit | Giá trị | Ý nghĩa |
|---|---|---|
| SW / SWS | `10` | Nguồn xung = PLL |
| PLLSRC | `1` | PLL lấy từ HSE |
| PLLMUL (21:18) | `0111` | x9 |
| HPRE (7:4) | `0000` | HCLK = SYSCLK |
| PPRE1 (10:8) | `100` | APB1 chia 2 |

SYSCLK = 8 MHz x 9 = **72 MHz**, APB1 = 36 MHz. [Đã đo]

**Test B**: Erase Chip trong Utility, cấp nguồn lại cho board, rồi F8 trong Keil -> **nạp được**. [Đã đo]
Ghi chú: lần trước, erase mà không reset/cấp nguồn lại thì vẫn lỗi. Vậy yếu tố quyết định là trạng thái xung nhịp của chip, không phải riêng việc xóa Flash.

Hai kết quả khớp giả thuyết, nhưng chưa biết thuật toán nạp làm gì với wait state. Bước 5 trả lời câu này.

---

## 5. Truy vết nguồn: dịch ngược thuật toán nạp (`.FLM`)

Tìm file và kiểm tra phiên bản DFP:

```powershell
Get-ChildItem C:\Users\ASUS\AppData\Local\Arm\Packs\Keil\STM32F1xx_DFP -Recurse -Filter *.FLM

Get-FileHash "...\STM32F1xx_DFP\2.3.0\Flash\STM32F10x_128.FLM","...\STM32F1xx_DFP\2.4.1\Flash\STM32F10x_128.FLM"

Select-String -Path "C:\Users\ASUS\Desktop\LED_PC13_LL\STM32F103C8.uvprojx" -Pattern "PackID"
```

```text
SHA256 (2.3.0) = 27642BA08E5DC56C7C90573F808ECE013EFABC68776CB8057EBAFDFA60A59648
SHA256 (2.4.1) = 27642BA08E5DC56C7C90573F808ECE013EFABC68776CB8057EBAFDFA60A59648
PackID         = Keil.STM32F1xx_DFP.2.4.1
```

Hai bản DFP có file giống hệt nhau, nên phiên bản DFP không phải biến số. [Đã đo]

Dịch ngược các hàm trong `.FLM` (mã Thumb, tool Python `capstone`). Hàm `Init`:

| Địa chỉ | Việc làm | Ý nghĩa |
|---|---|---|
| `0x04-0x08` | ghi `0` vào `[0x40022000]` | **`FLASH->ACR = 0`**: 0 wait state, tắt prefetch |
| `0x0A-0x10` | ghi `0x45670123`, `0xCDEF89AB` vào `+4` | `FLASH->KEYR`: mở khóa Flash |
| `0x12-0x1A` | đọc `+0x1C`, kiểm tra bit 2 | đọc `FLASH_OBR`, xem watchdog phần cứng |
| `0x1C-0x2C` | ghi vào `0x40003000` | cấu hình IWDG nếu watchdog là phần cứng |

Các phát hiện khác trong file:

- [Đã đo] Không có địa chỉ RCC (`0x40021000`) nào: FLM **không** đụng vào xung nhịp.
- [Đã đo] `UnInit` chỉ khóa Flash lại (`CR |= 0x80`), **không khôi phục ACR**.
- [Đã đo] Không có hàm `Verify` trong FLM (chỉ có Init, UnInit, EraseChip, EraseSector, ProgramPage). Keil verify bằng cách đọc Flash qua ST-Link, **sau khi** ACR đã bị đặt 0.

Kết luận bước 5: chip đang ở 72 MHz (Test A) + `Init` đặt 0 wait state + verify đọc sau đó = đọc sai word chẵn = `Contents mismatch`. Keil halt chip mà không reset nên xung nhịp của firmware cũ vẫn còn.

---

## 6. Khắc phục: Init File chạy trước khi nạp

Tài liệu Keil mô tả ô **Init File** (Options -> Utilities) là file khởi tạo chạy trước khi nạp Flash. Đưa chip về HSI 8 MHz (đủ cho 0 wait state) trước khi `Init` của FLM chạy.

Tạo `flash_init.ini` trong thư mục project (comment viết không dấu để Output không bị lỗi font):

```text
_WDWORD(0x40021004, 0x00000000);   // RCC_CFGR: select HSI
_WDWORD(0x40021000, 0x00000083);   // RCC_CR: keep HSI, HSE and PLL off
```

Alt + F7 -> Utilities -> "Use Target Driver for Flash Programming" -> Init File: trỏ tới file này.

Kết quả:

```text
Load "C:\\Users\\ASUS\\Desktop\\LED_PC13_LL\\Objects\\STM32F103C8.axf"
Include "C:\\Users\\ASUS\\Desktop\\LED_PC13_LL\\flash_init.ini"
Full Chip Erase Done.
Programming Done.
Verify OK.
Application running ...
```

Không cần erase chip bằng ST-LINK Utility nữa. [Đã đo]

Phương án phụ: dùng `ST-LINK_CLI` làm External Tool (đã chạy ổn):

```text
Command:   C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe
Arguments: -c SWD -P "#H" -V -rst
```

Phương án BOOT0 = 1 + Reset rồi F8 (bootloader hệ thống không chạy PLL) mới là lý thuyết, **chưa thử**.

---

## 7. In log xung nhịp, và điểm chưa giải thích

Thêm hàm `ShowClocks()` vào `flash_init.ini` (gọi trước và sau hai lệnh ghi RCC):

```text
FUNC void ShowClocks(void) {
  unsigned long cfgr, acr, sws, mul, src;
  cfgr = _RDWORD(0x40021004);
  acr  = _RDWORD(0x40022000);
  sws  = (cfgr >> 2) & 3;
  printf("RCC_CFGR=0x%08X  FLASH_ACR=0x%08X\n", cfgr, acr);
  printf("Flash wait states (LATENCY) = %d\n", acr & 7);
  if (sws == 0) { printf("SYSCLK = HSI = 8 MHz\n"); }
  if (sws == 1) { printf("SYSCLK = HSE = 8 MHz (assumed)\n"); }
  if (sws == 2) {
    mul = ((cfgr >> 18) & 15) + 2;
    if (mul > 16) { mul = 16; }
    src = 4;
    if ((cfgr >> 16) & 1) { src = 8; if ((cfgr >> 17) & 1) { src = 4; } }
    printf("SYSCLK = PLL x%d = %d MHz\n", mul, src * mul);
  }
}
```

Log thực tế:

```text
RCC_CFGR=0x001D0000  FLASH_ACR=0x00000000
Flash wait states (LATENCY) = 0
SYSCLK = HSI = 8 MHz
RCC_CFGR=0x00000000  FLASH_ACR=0x00000000
Flash wait states (LATENCY) = 0
SYSCLK = HSI = 8 MHz
Full Chip Erase Done.
Programming Done.
Verify OK.
```

Hai điều cần ghi nhận:

1. **Lần nạp này chip đã ở HSI 8 MHz sẵn**, nên script chưa được chứng minh là có tác dụng. Muốn chứng minh: nhấn Reset cho firmware chạy tới 72 MHz (`CFGR` kết thúc bằng `A`), rồi F8 và xem log "trước/sau".
2. **`CFGR = 0x001D0000` là trạng thái giữa chừng** của `SystemClock_Config()`: PLL đã cấu hình (HSE, x9) nhưng chưa chuyển nguồn sang PLL, `ACR` vẫn 0. [Suy luận] Firmware có thể đang kẹt ở `while(LL_RCC_PLL_IsReady() != 1)`. Chưa xác nhận.

Việc cần làm tiếp để làm rõ điểm 2: thêm vào `ShowClocks()`:

```text
cr = _RDWORD(0x40021000);
printf("RCC_CR=0x%08X HSERDY=%d PLLON=%d PLLRDY=%d\n", cr, (cr >> 17) & 1, (cr >> 24) & 1, (cr >> 25) & 1);
```

- `PLLON=1, PLLRDY=0`: PLL không bao giờ khóa, firmware kẹt.
- `PLLON=1, PLLRDY=1`: PLL ổn, chip chỉ bị dừng đúng lúc khởi động.

---

## 8. Tổng kết

| Hạng mục | Trạng thái |
|---|---|
| Nguyên nhân lỗi verify | `Init` của FLM đặt `FLASH->ACR = 0`; chip đang ở 72 MHz; Keil halt không reset; verify đọc thiếu wait state. Khớp Test A, Test B, và log dịch ngược. |
| Cách khắc phục | `flash_init.ini` đưa chip về HSI trước khi nạp (đã thành công, nhưng chưa tách riêng tác dụng của script) |
| Chưa kiểm chứng | `FLASH_ACR` thực tế sau lần nạp lỗi; vì sao CLI và J-Link không bị; vì sao firmware dừng giữa `SystemClock_Config()` |
| Bác bỏ | chip fake, CMSIS 6, Compiler V6, DFP 2.3.0 vs 2.4.1, RDP, tốc độ SWD, nguồn 3.3V |
