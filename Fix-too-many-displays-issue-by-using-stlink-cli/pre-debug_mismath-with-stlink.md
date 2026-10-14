# Hóa giải `Contents mismatch` (Keil + ST-Link + STM32F103C8T6)

Hai mục tiêu của file này:

1. `main.c` (LL) của bạn có vấp lỗi nào mà tài liệu ST hoặc cộng đồng đã biết trên dòng F103 không?
2. `Debug → Settings → Connect` và `Reset` thực chất làm gì, và quy trình để (A) nạp xong chạy ngay như ST-LINK Utility, (B) nạp xong chờ bấm RESET.

Ngày lập: 2026-10-13. Tài liệu này được lập trong cùng phiên trao đổi với dữ liệu đo của bạn.

**Nhãn độ tin cậy**

| Nhãn | Ý nghĩa |
|---|---|
| **[Đã đo]** | Có log hoặc số đo thật từ bạn |
| **[Tài liệu]** | Có trong tài liệu Keil / ST / IAR / CMSIS-Pack mà mình đã đọc |
| **[Cộng đồng]** | Báo cáo của người dùng trên diễn đàn (một nguồn, không phải xác nhận chính thức) |
| **[Suy luận]** | Hợp lý nhưng chưa chứng minh trực tiếp |
| **[Chưa thử]** | Đề xuất thí nghiệm, chưa có kết quả |

---

## 0. Kết luận ngắn

- **Cơ chế lỗi đã được xác lập chắc:** `main.c` đưa chip lên 72 MHz, file nạp `.FLM` của Keil ghi `FLASH_ACR = 0` (0 wait state), và chip không được reset sạch trước khi nạp, nên verify đọc thiếu wait state.
- **`main.c` không vấp lỗi đã biết nào** trên F103x8 (mục 2). Nó chỉ là điều kiện kích hoạt.
- **Chip thật hay giả không liên quan.** Board mã `0x20036410` (mã revision hợp lệ) vẫn lỗi y hệt (mục 1).
- **Điều chưa biết:** vì sao reset không tới chip khi nạp. Có ba khả năng (mục 3.5). NRST + Hardware Reset như đang cấu hình đã được thử và **chưa** chữa được.
- **Cách nạp ổn định đã chạy:** `flash_init.ini` (đưa chip về HSI trước khi `.FLM` chạy) hoặc `ST-LINK_CLI` sau build.

---

## 1. Bằng chứng đã có

| # | Thí nghiệm | Kết quả | Ý nghĩa | Nhãn |
|---|---|---|---|---|
| 1 | Comment `SystemClock_Config()` (chip ở HSI 8 MHz), không `.ini` | Hết mismatch | 72 MHz là điều kiện kích hoạt | [Đã đo] |
| 2 | 72 MHz bật lại, LED xác nhận nhanh. F8 #1, nhấn RESET trên board, F8 #2 | #1 Verify OK, #2 mismatch | Tái hiện ổn định, không cần `.ini` để gây lỗi | [Đã đo] |
| 3 | Lặp lại với board thứ hai và ST-Link thứ hai | Kết quả y hệt | Lỗi hệ thống, không phải một đơn vị hỏng | [Đã đo] |
| 4 | Board thứ ba, `0xE0042000 = 0x20036410` | Vẫn mismatch ở F8 #2 | Không liên quan chip thật/giả | [Đã đo] |
| 5 | Mẫu địa chỉ sai trong log F8 #2 | Với địa chỉ chia hết cho 8, byte `Flash=` bằng đúng byte `Required` ở địa chỉ − 8 (ví dụ `0x08000010`: `75H`, bằng byte tại `0x08000008`) | Word đầu của mỗi cặp 64-bit trả về dữ liệu cặp trước, dấu hiệu đọc thiếu wait state | [Đã đo] |
| 6 | `Init` của `STM32F10x_128.FLM` (dịch ngược) | Ghi `FLASH->ACR = 0`; `UnInit` không khôi phục | Nguyên nhân phía thuật toán nạp | [Đã đo] |
| 7 | Mẫu `FlashPrg.c` của Keil (trích trong blog) | `Init` đặt ACR = 0 với ghi chú "Zero Wait State" | Khớp với mục 6, cho thấy đây là thiết kế mẫu | [Cộng đồng] |
| 8 | NRST nối, Reset = Hardware Reset, không `.ini` | Vẫn mismatch ở F8 #2 | Cấu hình đó chưa đủ | [Đã đo] |
| 9 | ST-LINK Utility, Connect Under Reset, đọc `0x40021004` khi firmware chạy 72 MHz | `0x001D040A` | Reset không tới chip (hoặc ST-Link không phát xung) | [Đã đo] |
| 10 | `flash_init.ini` đưa chip về HSI trước khi nạp | F8 chạy ổn nhiều lần, kể cả khi Hardware Reset tắt hoặc bật | Cách sửa thực tế đã chạy | [Đã đo] |
| 11 | `.ini` khác (Bản 2) | Keil `UV4.exe` sập, mã `0xC000041D`, module `unknown` | Do nội dung script, chưa bisect được lệnh | [Đã đo] |
| 12 | `0xE0042000 = 0x00006410` trên hai board đầu | `DEV_ID = 0x410` đúng F103, `REV_ID = 0x0000` không có trong các mã revision mình đọc (A=`0x1000`, Z=`0x1001`, B=`0x2000`, Y=`0x2001`) | Hai board này khác board thứ ba. Có thể là silicon không phải ST nhưng chưa chứng minh | [Suy luận] |

Ghi chú về mục 12: `0x2003` mình nhớ ứng với nhóm Rev 1/2/3/X (mới nhất), nhưng chưa kiểm chứng bằng tài liệu trong phiên này. CPUID `0x411FC231` chỉ cho biết lõi Cortex-M3 r1p1, không chứng minh được nhà sản xuất.

---

## 2. Câu hỏi 1: `main.c` có vấp lỗi đã biết không?

### 2.1 Rà từng phần `SystemClock_Config()`

| Hạng mục | `main.c` làm | Chuẩn / tài liệu | Kết luận |
|---|---|---|---|
| Wait state ở 72 MHz | `LL_FLASH_SetLatency(LL_FLASH_LATENCY_2)` | 0 WS cho 0–24 MHz, 1 WS cho 24–48 MHz, 2 WS cho 48–72 MHz [Tài liệu] | Đúng |
| Thứ tự | Đặt latency sau khi PLL sẵn sàng, **trước** khi chuyển SYSCLK sang PLL | Phải đặt trước khi tăng xung. Báo cáo cộng đồng 2010: ở 72 MHz với 0 WS chip crash ngay sau lệnh chuyển sang PLL [Cộng đồng] | Đúng. CubeMX thường đặt latency ngay đầu, cũng chấp nhận được |
| Prefetch | Không đụng tới | Giá trị reset của ACR là `0x30` (bit prefetch bật). Log của bạn thấy `0x30` ngay sau reset và `0x32` sau khi firmware chạy | Ổn |
| Prescaler | AHB /1, APB1 /2, APB2 /1 | APB1 tối đa 36 MHz | Đúng |
| Chờ HSE/PLL | `while(...)` không giới hạn | Mã nguồn mở cho F1 (RIOT) ghi chú rằng MCU sẽ treo mãi nếu chọn nguồn xung ngoài mà không có thạch anh [Cộng đồng] | **Rủi ro**, không phải nguyên nhân lỗi nạp. Nên có timeout |
| `SystemCoreClock` | Không cập nhật | Kiến thức chung: LL cần `LL_SetSystemCoreClock(72000000)` nếu code khác dựa vào biến này [Suy luận] | Không liên quan lỗi nạp |
| `Delay(90000)` | Comment ghi 4500000, code là 90000 | Ở 72 MHz LED đảo cỡ vài ms, trông như sáng mờ | Dễ nhầm "chip không chạy" |

### 2.2 Errata silicon của F103x8 (DocID14574 Rev 13)

| Mục errata | Nội dung | `main.c` có chạm không? |
|---|---|---|
| 2.2 | Đọc Flash sau WFI/WFE khi prefetch bật, 2 WS, FLITF clock bị tắt trong Sleep | Không (không dùng WFI/WFE) |
| 2.3 | `DBGMCU_IDCODE`/`DBGMCU_CR` chỉ đọc được ở chế độ debug, user mode đọc ra 0 | Không (chỉ liên quan khi đọc từ firmware) |
| 2.10 | Revision **Z và B** không hỗ trợ một số chuỗi lệnh của compiler mới (IAR ≥ 5.20, GCC ≥ 4.2.3). Rev Y và 1 không bị | Không. Keil/ARMCC không nằm trong danh sách. **Đây là gốc của cảnh báo IAR** mà bạn thấy |
| 2.12 | Bit BSY lên trễ 1 chu kỳ sau khi đặt STRT khi xóa Flash | Không (`main.c` không xóa/ghi Flash). Chỉ liên quan code nạp |

Không có erratum nào về HSE, PLL hay latency cho F103x8 trong tài liệu này.

### 2.3 Điều cộng đồng đã biết gần nhất với triệu chứng của bạn

| Nguồn | Nội dung | Liên hệ |
|---|---|---|
| ST community 2010 (IAR + ST-Link) | Một board đọc ra toàn số 0 từ Flash ở 72 MHz với 0 WS, cùng code trên board khác chạy "tạm được" | Khớp: thiếu wait state ở 72 MHz cho dữ liệu sai, và mức độ thay đổi theo từng board. Lý do một số lần verify may mắn qua |
| ST community 2010 (SYSCLK trên F101) | Hạ về 0 WS ở 72 MHz luôn crash, kể cả khi HCLK chia nhỏ | Latency phải bám theo xung thực tế |
| ST community 2016 | Xung hệ thống quá cao làm không thể lập trình lại chip, phải dùng Connect Under Reset hoặc BOOT0 | Cùng họ vấn đề: firmware đang ở xung cao khi debugger vào |
| ST community 2025 (F103C8 + Keil) | Nạp lần đầu được, lần sau "No target device". Gợi ý: nối NRST để Connect Under Reset dùng được, hoặc thêm trễ ngắn ở `Reset_Handler` | Liên quan lỗi kết nối, không phải lỗi verify, nhưng cùng một cách chữa |

### 2.4 Kết luận mục 2

`main.c` đặt latency đúng và không chạm erratum nào. "Lỗi" nằm ở **tương tác**: `FLM Init` ép `ACR=0` trong khi chip đang ở 72 MHz và debugger không đưa chip về trạng thái sạch. Không có cách sửa từ phía `main.c`, vì `Init` của FLM chạy sau khi firmware đã dừng.

**Cải tiến tùy chọn** (không sửa được lỗi nạp, chưa biên dịch thử):

```c
#include "stm32f1xx_ll_utils.h"

#define WAIT_MAX 500000UL

void SystemClock_Config(void)
{
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
    while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2) {}

    LL_RCC_HSE_Enable();
    uint32_t n = WAIT_MAX;
    while (LL_RCC_HSE_IsReady() != 1 && --n) {}
    if (n == 0) { return; }          /* thạch anh hỏng: ở lại HSI 8 MHz */

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_9);
    LL_RCC_PLL_Enable();
    n = WAIT_MAX;
    while (LL_RCC_PLL_IsReady() != 1 && --n) {}
    if (n == 0) { return; }

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {}

    LL_SetSystemCoreClock(72000000);
}
```

Và đổi `Delay(90000)` thành `Delay(4500000)` để LED phân biệt rõ hai tốc độ (ước lượng của mình, chưa đo: ~0.5–1 s ở 72 MHz, ~5–6 s ở 8 MHz).

---

## 3. Câu hỏi 2: `Connect` và `Reset` thực chất là gì

Lưu ý nguồn: các định nghĩa dưới đây lấy từ tài liệu Keil của **ULINKpro** và **J-Link**, và tài liệu **IAR** về ST-LINK. Hộp thoại ST-Link của Keil dùng cùng tên tùy chọn nhưng mình **không đọc được trang Keil riêng cho ST-Link**, nên coi ý nghĩa là tương đương chứ chưa phải xác nhận chính thức.

### 3.1 Chuỗi sự kiện khi nhấn F8

```
(1) Kết nối SWD            <- Connect quyết định có tác động chân reset ở đây không
(2) [Reset after Connect]  <- Reset quyết định kiểu reset nếu tick
(3) Init File (Utilities)  <- chạy ở đây nếu có
(4) FLM Init: ACR = 0      <- thủ phạm. Chạy sau 1-3
(5) Erase, Program
(6) Verify                 <- đọc Flash với 0 WS: sai nếu chip còn 72 MHz
(7) [Reset and Run]        <- chạy firmware, dùng kiểu Reset
```

Điều cần có để hết lỗi: **ở bước (1)–(3) chip phải về HSI**. Nếu reset chỉ xảy ra ở bước (7) thì quá muộn.

### 3.2 `Connect`

| Tùy chọn | Tác dụng | Cần NRST? | Với lỗi của bạn |
|---|---|---|---|
| Normal | Kết nối bình thường, không có thao tác reset (mình suy ra, chưa thấy câu mô tả riêng) | Không | Không giúp. Đây là kịch bản gây lỗi |
| with Pre-reset | Áp reset phần cứng **trước** khi kết nối | **Có** | Giúp nếu xung reset thật sự tới chip |
| under Reset | Giữ chân reset phần cứng ở mức thấp **trong lúc** kết nối | **Có** | Giúp nếu xung reset tới chip. Cũng là cách chữa kinh điển cho "No target device" |

[Tài liệu] cho hai dòng cuối. Chúng đều dựa vào chân NRST của ST-Link. Phép đo #9 cho thấy chân đó chưa tác động được chip trong Utility.

### 3.3 `Reset`

| Tùy chọn | Cơ chế | Reset ngoại vi (RCC, `FLASH_ACR`)? | Ghi chú |
|---|---|---|---|
| Autodetect | Keil chọn cách phù hợp. Gặp thiết bị lạ thì dùng SYSRESETREQ | Tùy cách được chọn | Không biết thực tế chọn gì cho F103 (có thể phụ thuộc mô tả debug trong Pack) |
| HW RESET | Kéo chân reset phần cứng | **Thường có** (theo IAR) | Cần NRST nối và ST-Link phát được |
| SYSRESETREQ | Reset mềm qua bit SYSRESETREQ | **Có** (J-Link mô tả "core và ngoại vi") | Không cần NRST. Đáng thử nhất |
| VECTRESET | Reset mềm qua bit VECTRESET | **Không** (J-Link, IAR) | Chỉ reset lõi |

**Suy luận từ đo đạc của bạn** (log.txt): có lúc `FLASH_ACR = 0x02` (LATENCY=2 nhưng prefetch tắt) thay vì `0x32`. Điều này khớp với việc Keil chạy firmware sau nạp **mà không reset ngoại vi**, giống VECTRESET hoặc không reset. [Suy luận]

### 3.4 Các công tắc liên quan

| Công tắc | Vị trí | Tác dụng | Ghi chú |
|---|---|---|---|
| Reset after Connect | Debug → Settings | Thực hiện kiểu Reset đã chọn ngay sau khi kết nối | [Tài liệu] |
| Reset and Run | Debug → Settings → tab Flash Download | Sau nạp: reset rồi chạy. Dòng `Application running ...` | Bình thường, không phải lỗi |
| **Pack → Debug Description: Enable** | Debug → Settings → tab Pack | Dùng mô tả/chuỗi debug trong DFP. Khi bật, một số tùy chọn Connect/Reset có thể bị khóa. Tắt đi để thấy hành vi mặc định của driver [Tài liệu] | Hai báo cáo cộng đồng (Arm, EEVblog): tắt ô này khiến ST-Link reset được sau khi nạp [Cộng đồng] |
| Init File (Utilities) | Utilities → Settings/Init File | Script chạy khi nạp Flash. Khác với Initialization File ở tab Debug | `flash_init.ini` dùng ô này |
| Debug access log | Debug → Settings | Ghi log truy cập debug vào file `<adapter>_DebugLog_<n>.log` trong thư mục project [Tài liệu] | Cách tốt nhất để thấy Keil có phát reset thật hay không. Chưa biết log có bao gồm bước nạp Flash không |

### 3.5 Điều chưa có lời đáp

- Tài liệu Keil mô tả các tùy chọn Connect/Reset là áp dụng "mỗi khi bắt đầu phiên debug". **Chưa tìm thấy tài liệu nói rõ chúng có áp dụng cho F8 (chỉ nạp Flash) hay không.** [Chưa thử]
- Một báo cáo cộng đồng ghi nhận có xung reset ở đầu lần Download nhưng không có xung sau khi nạp xong (MDK 5.28/5.31) [Cộng đồng]. Nếu đúng, xung ở đầu là thứ cần cho bạn.
- Ba khả năng vì sao reset không tới chip trong bài đo #9: (a) dây hoặc chân NRST có vấn đề, (b) clone ST-Link không phát xung NRST, (c) driver không dùng NRST. Phép thử chạm chân R xuống GND để loại phía board vẫn nên làm một lần.

---

## 4. Quy trình

### 4.1 Quy trình kiểm chứng chuẩn (để thí nghiệm hợp lệ)

1. Build lại, xem Build Output có `0 Error(s)` và `main.c` được biên dịch.
2. Đặt đúng một cấu hình cần thử. Bấm OK.
3. **Tắt hẳn Keil, rút USB, cắm lại, mở Keil.** Mở lại hộp thoại kiểm tra cấu hình đã lưu (đặc biệt hai ô Init File).
4. F8 lần 1.
5. Nhấn RESET vật lý trên board. **Kiểm tra LED nháy chu kỳ 72 MHz** (xem `Delay` ở mục 2.4). Nếu LED chậm, thí nghiệm không hợp lệ, dừng lại.
6. F8 lần 2 và ghi lại kết quả.

**Chẩn đoán phụ:** sau khi firmware chạy, đọc `0x40022000` bằng ST-LINK Utility (Mode = Normal hoặc Hot Plug để không reset khi đọc): `0x32` nghĩa là chip được reset sạch rồi chạy, `0x02` nghĩa là ngoại vi không được reset.

### 4.2 Ma trận thí nghiệm (điền kết quả)

Thứ tự đề xuất, mỗi dòng một biến thay đổi:

| # | Pack: Debug Description | Connect | Reset | Reset after Connect | Giả thuyết | LED sau F8 #1 | F8 #2 |
|---|---|---|---|---|---|---|---|
| B0 | Enable (mặc định) | Normal | Autodetect | tắt | Baseline: mismatch (đã đo) | | mismatch |
| E1 | **Disable** | giữ B0 | giữ B0 | giữ B0 | Cộng đồng báo cáo ST-Link reset được khi tắt Pack | | |
| E2 | Enable | Normal | **SYSRESETREQ** | **bật** | Reset mềm xóa ACR và RCC, không cần NRST | | |
| E3 | Enable | **with Pre-reset** | SYSRESETREQ | bật | Thêm xung HW trước kết nối nếu NRST có tác dụng | | |
| E4 | Enable | **under Reset** | SYSRESETREQ | bật | Giữ reset trong lúc kết nối | | |
| E5 | Enable | under Reset | **HW RESET** | bật | Cấu hình cộng đồng dùng cho một số STM32 "khó tính". Cần NRST hiệu quả | | |
| E6 | **Disable** | under Reset | SYSRESETREQ | bật | Kết hợp E1 và E4 | | |
| E7 | tùy | tùy | tùy | tùy | `flash_init.ini` (HSI trước khi nạp) | | pass (đã đo) |
| E8 | — | — | — | — | `ST-LINK_CLI -c SWD -P "#H" -V -rst` làm External Tool | | |

Nếu một dòng pass, lặp lại ít nhất 5 lần F8 và thêm kịch bản "nạp `.hex` bằng ST-LINK Utility rồi F8 trong Keil".

Nếu cần xem chi tiết: bật **Debug access log** trong lúc chạy một dòng và đối chiếu với log.

### 4.3 Chế độ A: nạp xong chạy ngay (giống ST-LINK Utility)

| Bước | Thiết lập |
|---|---|
| 1 | Chọn một dòng pass ở ma trận 4.2 để tránh `Contents mismatch` |
| 2 | Flash Download: **Program**, **Verify**, **Reset and Run** đều tick |
| 3 | Reset = **SYSRESETREQ** (hoặc HW RESET nếu NRST được chứng minh hiệu quả). **Tránh VECTRESET** vì không reset ngoại vi |
| 4 | Kiểm tra: sau F8, LED nháy đúng chu kỳ 72 MHz **mà không cần nhấn RESET** và `0x40022000` đọc ra `0x32` |

Nếu bước 4 không đạt, dòng thiết lập đó chưa cho "chạy ngay đúng" dù nạp xong. Khi đó dùng `ST-LINK_CLI` (`-rst` reset và chạy ngay).

### 4.4 Chế độ B: nạp xong chờ nhấn RESET vật lý

| Bước | Thiết lập |
|---|---|
| 1 | Giống chế độ A về cách tránh `Contents mismatch` |
| 2 | Flash Download: **bỏ tick Reset and Run** |
| 3 | Sau F8 (thấy `Verify OK`), nhấn RESET trên board. Chân NRST nối vào ST-Link **không cần thiết** cho chế độ này |

Đây là hành vi bạn đang thấy khi dùng ST-Link Debugger của Keil. Nó an toàn vì nút RESET vật lý luôn reset sạch.

### 4.5 Phương án dự phòng chắc chắn

| Phương án | Cách làm | Trạng thái |
|---|---|---|
| `flash_init.ini` | Init File ở Utilities. Nhớ `ResetClocks()` ghi CFGR **thêm một lần** sau khi tắt PLL (các bit PLL chỉ xóa được khi PLL tắt). Nên đọc thêm `RCC_CR` **trước** khi reset | Đã chạy ổn |
| `ST-LINK_CLI` | External Tool hoặc Run User Programs After Build | Đã chạy ổn |
| BOOT0 = 1 rồi F8 | Chip chạy bootloader hệ thống, không chạy PLL | Chưa thử |

---

## 5. Giả thuyết đã hạ cấp hoặc bác

| Giả thuyết | Trạng thái | Lý do |
|---|---|---|
| "Chip giả nên lỗi" | Bác | Board `0x20036410` vẫn lỗi |
| "Hai board giống nhau nên không phải clone" | Không đứng vững | Hành vi wait state không phân biệt thật/giả |
| "CPUID `0x411FC231` chứng minh 100% chính hãng" | Quá tay | CPUID chỉ nói về lõi Cortex-M3 |
| "NRST + Hardware Reset đã chữa" | Bác (với cấu hình đã thử) | Test hợp lệ vẫn mismatch |
| "Flash bị ghi đè chồng chập" | Bác | Mỗi lần F8 có Full Chip Erase, dữ liệu sai là do đường đọc |
| "Flash nhỏ nên vài lần là lỗi" | Bác | Độ bền Flash cỡ hàng chục nghìn lần xóa |
| "`Application running ...` là lỗi" | Bác | Là dòng thông báo của Reset and Run |
| "DLL ST-Link của Keil gây sập" | Chưa có bằng chứng | Mã `0xC000041D` và module `unknown` không chỉ ra DLL |

---

## 6. Nguồn

- Keil, ULINKpro — Connect & Reset Options: https://www.keil.com/support/man/docs/ulinkpro/ulinkpro_ctx_debug.asp
- Keil, J-Link — Debug (Connect, Reset): https://www.keil.com/support/man/docs/jlink/jlink_conf_debug.asp
- IAR, ST-LINK — Setup (kiểu reset): https://docs.iar.com/ewarm/9.7/en/c-spy-debugging/debugger-options/reference-information-on-c-spy-hardware-debugger-driver-options/st-link---setup.html
- CMSIS-Pack — Use of debug descriptions in µVision: https://arm-software.github.io/CMSIS_5/5.7.0/Pack/html/dbg_sqns_uvision.html
- Arm community — MDK 5.28 problem with Reset and Run: https://community.arm.com/support-forums/f/keil-forum/44063/mdk-arm-5-28-problem-with-reset-and-run
- EEVblog — ST-Link V2 không reset target sau khi nạp: https://eevblog.com/forum/microcontrollers/cant-get-stm-link-v2-debuggerprogrammer-to-reset-the-target-after-programming
- Arm community — "PDSC: Sequence Execution failed" (Connect under Reset + HW RESET): https://community.arm.com/support-forums/f/keil-forum/47144/pdsc-sequence-execution-failed-error/167512
- ST, errata STM32F101/102/103x8/B (DocID14574 Rev 13): https://git.ondrovo.com/gex/gex-platform-info/raw/branch/master/F103_BLUEPILL/stm32f103%20errata.pdf
- ST community — SYSCLK trên STM32F101 (0 WS ở 72 MHz crash): https://community.st.com/s/question/0D50X00009XkbE7/sysclk-on-stm32f101?
- ST community — st-link + IAR debugger: https://community.st.com/t5/stm32-mcus-products/st-link-iar-debugger/td-p/567522
- ST community — chip không lập trình được khi xung quá cao (2016): https://community.st.com/s/question/0D50X00009XkaK7SAJ/unable-to-program-when-in-stop-mode?
- ST community — F103C8 nạp lần hai báo "No target device" (2025): https://community.st.com/stm32-mcus-products-25/stm32-flash-software-155091
- Blog trích `FlashPrg.c` của Keil (Init đặt ACR = 0): https://blog.csdn.net/sinat_31039061/article/details/128350295
- RIOT-OS, `cpu/stm32f1/cpu.c` (ghi chú treo khi thiếu thạch anh): https://git.riot-os.org/RIOT-OS/RIOT/blame/commit/3711a6d406e49a816a75492bf1e6285cf6b8590c/cpu/stm32f1/cpu.c