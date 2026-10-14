# ⚡ Giải phẫu lỗi `Contents mismatch` trên STM32F103 (Keil MDK) & Chuyển đổi sang STM32CubeIDE (LL)

![MCU](https://img.shields.io/badge/MCU-STM32F103C8T6-03234B?style=flat-square&logo=stmicroelectronics)
![Clock](https://img.shields.io/badge/SYSCLK-72MHz_HSE_PLL-0091BD?style=flat-square&logo=arm)
![Library](https://img.shields.io/badge/Driver-STM32_LL_(Low--Layer)-2ea44f?style=flat-square)
![IDE](https://img.shields.io/badge/IDE-Keil_µVision5_%7C_STM32CubeIDE-orange?style=flat-square)

Hồ sơ điều tra cấp độ thanh ghi, dịch ngược thuật toán nạp `.FLM` và giải pháp khắc phục triệt để lỗi `Contents mismatch` khi nạp Flash cho vi điều khiển **STM32F103C8T6** chạy ở xung nhịp **72 MHz**.

---

## ⚠️ 1. Vấn đề nhức nhối trong Cộng đồng (Community Issues)

Khi lập trình STM32F103 đẩy xung nhịp hệ thống (`SYSCLK`) lên mức tối đa **72 MHz** (sử dụng Bare-metal hoặc thư viện Low-Layer `LL`), việc nhấn `F8` (Download) trên **Keil µVision (`UV4.exe`)** thường xuyên thất bại ở lần nạp thứ 2 (ngay sau khi chip đã chạy lên 72 MHz) với thông báo lỗi kinh điển:

```text
Full Chip Erase Done.
Programming Done.
Contents mismatch at: 08000000H  (Flash=28H  Required=60H) !
Contents mismatch at: 08000008H  (Flash=60H  Required=75H) !
Contents mismatch at: 08000010H  (Flash=75H  Required=79H) !
...
Too many errors to display !
Error: Flash Download failed  -  "Cortex-M3"
```

Vấn đề này đã xuất hiện dai dẳng hơn một thập kỷ trên các diễn đàn kỹ thuật quốc tế nhưng thường bị quy chụp nhầm cho "chip giả/clone" hoặc "hỏng mạch nạp ST-Link":
* **ST Community Issue [`#567522`](https://community.st.com/t5/stm32-mcus-products/st-link-iar-debugger/td-p/567522) & [`#0D50X00009XkbE7`](https://community.st.com/s/question/0D50X00009XkbE7/sysclk-on-stm32f101):** Đọc dữ liệu Flash bị sai lệch hoặc sập chip khi chạy ở `72 MHz` với `0 Wait State`.
* **ST Community Issue [`#0D50X00009XkaK7SAJ`](https://community.st.com/s/question/0D50X00009XkaK7SAJ/unable-to-program-when-in-stop-mode) & [`#155091`](https://community.st.com/stm32-mcus-products-25/stm32-flash-software-155091):** Nạp lần đầu thành công nhưng không thể nạp lại ở các lần tiếp theo khi xung nhịp hệ thống đang ở mức cao.
* **ARM Keil Forum Issue [`#44063`](https://community.arm.com/support-forums/f/keil-forum/44063/mdk-arm-5-28-problem-with-reset-and-run), [`#47144`](https://community.arm.com/support-forums/f/keil-forum/47144/pdsc-sequence-execution-failed-error/167512) & [EEVblog Thread](https://eevblog.com/forum/microcontrollers/cant-get-stm-link-v2-debuggerprogrammer-to-reset-the-target-after-programming):** Trình điều khiển ST-Link V2 trên Keil MDK không phát xung Reset chuẩn xác để đưa ngoại vi về trạng thái mặc định trước khi nạp.
* **CSDN Analysis [`#128350295`](https://blog.csdn.net/sinat_31039061/article/details/128350295):** Phơi bày mã nguồn mẫu `FlashPrg.c` của Keil đặt cứng `FLASH->ACR = 0` (Zero Wait State) bên trong hàm `Init()`.

> **Kết quả kiểm thử chéo phần cứng của dự án:** Thử nghiệm thực tế trên cả các bo mạch mang mã Revision lạ (`DBGMCU_IDCODE = 0x00006410`) lẫn bo mạch mang mã Revision chính hãng ST (`0x20036410`), kết hợp cùng nhiều mạch nạp ST-Link V2 khác nhau đều cho ra **kết quả lỗi giống hệt nhau 100%**. Quá trình `Erase` và `Program` thực chất đã ghi dữ liệu vào Flash **đúng 100%**, lỗi chỉ nổ ra ở khâu đọc đối chiếu (**Verify**).

---

## 🐛 2. Mô tả Bản chất Lỗi & Luồng hoạt động của Bug (Bug Execution Flow)

```text
[UV4.exe] ──► [ST-Link SWD: Halt CPU (RCC vẫn 72 MHz)] ──► [SRAM: STM32F10x_128.FLM] ──► [FLM Init(): FLASH_ACR = 0] ──► 💥 Verify trượt 8-byte!
```

1. **Không Reset được ngoại vi RCC:** Khi nhấn `F8`, `UV4.exe` chỉ dừng lõi Cortex-M3 (`Halt CPU`), nhưng không đưa được thanh ghi `RCC_CFGR` (`0x40021004`) về mặc định. Chip vẫn bị kẹt ở `0x001D040A` (**PLL 72 MHz**).
2. **Thủ phạm ngầm `STM32F10x_128.FLM`:** `UV4.exe` bơm thuật toán nạp `.FLM` từ máy tính vào `SRAM` (`0x20000000`) của STM32 và ép CPU chạy hàm `Init()`. Hàm này thực thi lệnh ghi cứng `FLASH->ACR = 0x00000000` (ép độ trễ đọc Flash về **0 Wait State** và tắt Prefetch) vì mặc định giả sử rằng chip đang ở `8 MHz`.
3. **Quy luật trượt 8-Byte (`Address - 8`) trên bộ chốt 64-bit:** Ở `72 MHz` (chu kỳ `HCLK ≈ 13.88 ns`), mảng tế bào nhớ Flash vật lý cần `35–40 ns` (`2 Wait States`) để xuất xong một khối 64-bit (8 byte = 2 Word) ra thanh ghi chốt đọc (Read Latch). Do `Wait State = 0`, khi đọc Word đầu tiên của mỗi khối 8-byte (địa chỉ tận cùng `0..3` và `8..B`), Bus AHB lấy mẫu quá sớm nên đọc phải **dữ liệu cũ của khối 8-byte liền trước (`Address - 8`)**, trong khi Word thứ hai (tận cùng `4..7` và `C..F`) đã qua ngưỡng 40 ns nên đọc đúng 100%.

---

## 💡 3. Giải pháp Khắc phục & Cấu trúc Mã nguồn

* 📁 **[`LED_PC13_LL/`](./LED_PC13_LL) (Project Keil µVision 5):**
  * Chứa mã nguồn `main.c` chuẩn LL chạy `72 MHz` (đã bổ sung `WAIT_MAX` timeout chống treo thạch anh theo [RIOT-OS `#3711a6d`](https://git.riot-os.org/RIOT-OS/RIOT/blame/commit/3711a6d406e49a816a75492bf1e6285cf6b8590c/cpu/stm32f1/cpu.c)).
  * Khắc phục lỗi khi nhấn `F8` bằng kịch bản `flash_init.ini` (đặt tại `Options for Target -> Utilities -> Init File`), dùng lệnh `_WDWORD` can thiệp thẳng vào thanh ghi `RCC` qua SWD để ép chip lùi từ `72 MHz` về `HSI 8 MHz` trước khi `.FLM` chạy, hoặc gọi `ST-LINK_CLI.exe` qua External Tools.
* 📁 **[`LED_PC13_LL_CUBEIDE/`](./LED_PC13_LL_CUBEIDE) (Project STM32CubeIDE):**
  * Chuyển đổi toàn bộ cấu hình `72 MHz` LL sang STM32CubeIDE. Nhờ sử dụng `ST-LINK GDB Server` và `STM32_Programmer_CLI.exe` (quản lý `Reset & Halt` phần cứng chuẩn xác, không dùng `.FLM` của Keil), lỗi `Contents mismatch` hoàn toàn bị loại bỏ.
  * Khắc phục lỗi mất kết nối SWD (`localhost:61234: Connection timed out`) bằng `LL_GPIO_AF_Remap_SWJ_NOJTAG()` và thiết lập External Tools CLI tách biệt 2 chế độ: *Nạp chạy ngay* vs *Nạp xong đứng im chờ bấm Reset vật lý*.

---

## 📖 4. Hồ sơ Điều tra Chi tiết & Sơ đồ Tương tác (Navigation Links)

Toàn bộ bằng chứng thực nghiệm, nhật ký đo đạc thanh ghi và sơ đồ luồng dữ liệu được lưu trữ tại thư mục tài liệu chính của dự án. Dưới đây là đường dẫn đến từng tệp theo đúng vai trò trong quá trình điều tra:

1. 📂 **[Thư mục Tổng hợp Hồ sơ Debug & Logs (`Fix-too-many-displays-issue-by-using-stlink-cli/`)](https://github.com/chinart123/Flash-STM32F103C8/tree/main/Fix-too-many-displays-issue-by-using-stlink-cli)**  
   *Kho lưu trữ lớn chứa toàn bộ các tệp log thu thập từ phần cứng, hình ảnh cấu hình và các bản báo cáo phân tích.*

2. 📝 **[Nhật ký Debug Thủ công Sơ khai nhất (`hand-record-log.txt`)](https://github.com/chinart123/Flash-STM32F103C8/blob/main/Fix-too-many-displays-issue-by-using-stlink-cli/hand-record-log.txt)**  
   *Tệp hand-record "chay" đầu tiên ghi chép lại từng thao tác thử nghiệm thực tế bằng tay và quan sát trực tiếp sự thay đổi giá trị của các thanh ghi (`RCC_CFGR`, `FLASH_ACR`, `DBGMCU_IDCODE`) qua từng bước.*

3. 🔍 **[Hồ sơ Chẩn đoán & Truy vết ngược Thủ phạm `.FLM` (`claude-diaganose-keil-stlink-flash-debug-log.md`)](https://github.com/chinart123/Flash-STM32F103C8/blob/main/Fix-too-many-displays-issue-by-using-stlink-cli/claude-diaganose-keil-stlink-flash-debug-log.md)**  
   *Bản phân tích truy vết ngược từ dữ liệu hand-record và log lỗi, bóc tách quy luật trượt 8-byte và dịch ngược thuật toán nạp để chỉ đích danh thủ phạm là hàm `Init()` trong tệp `.FLM` của KeilC.*

4. ⚡ **[Sơ đồ Tương tác Luồng chạy của KeilC & Nhược điểm của `UV4.exe` (`KeilC-flash-data-flow.html`)](https://github.com/chinart123/Flash-STM32F103C8/blob/main/Fix-too-many-displays-issue-by-using-stlink-cli/KeilC-flash-data-flow.html)**  
   *Mô phỏng trực quan luồng nạp của KeilC. Khi kết hợp đọc cùng tệp `claude-diaganose-keil-stlink-flash-debug-log.md` ở trên, sơ đồ này cho thấy rõ: dù chúng ta đã ghi chép thủ công (hand-record) mọi thứ nhưng ban đầu không thể ngờ rằng chính `UV4.exe` và tệp `.FLM` đã tự ý can thiệp ngầm vào thanh ghi `FLASH_ACR = 0` trong RAM, đẩy người dùng vào thế bất lực với cấu hình mặc định và buộc phải cầu cứu External Loader / `flash_init.ini` hoặc nạp file `.hex` bên ngoài bằng ST-LINK Utility.*  
   👉 **[Bấm vào đây để chạy trực tiếp Sơ đồ `KeilC-flash-data-flow.html` trên Trình duyệt](https://htmlpreview.github.io/?https://github.com/chinart123/Flash-STM32F103C8/blob/main/Fix-too-many-displays-issue-by-using-stlink-cli/KeilC-flash-data-flow.html)**

5. 📄 **[Báo cáo Kỹ thuật Toàn tập — Bản chuẩn Markdown (`final-document.md`)](https://github.com/chinart123/Flash-STM32F103C8/blob/main/Fix-too-many-displays-issue-by-using-stlink-cli/final-document.md)**  
   *Tài liệu tổng hợp cuối cùng (đầy đủ 5 phần) đúc kết toàn bộ nguyên nhân lỗi, bảng kiểm thử chéo phần cứng, ma trận thí nghiệm và hướng dẫn khắc phục từ A-Z trên cả KeilC lẫn STM32CubeIDE.*

6. 🌐 **[Báo cáo Kỹ thuật Toàn tập — Bản Giao diện Đồ họa Cao cấp (`final-document(better-UI-for-graph).html`)](https://github.com/chinart123/Flash-STM32F103C8/blob/main/Fix-too-many-displays-issue-by-using-stlink-cli/final-document(better-UI-for-graph).html)**  
   *Phiên bản HTML hoàn chỉnh có giao diện UI/UX đẹp mắt (hỗ trợ Dark/Light mode), tích hợp trực tiếp sơ đồ luồng chạy tương tác và hướng dẫn cấu hình cho cả 2 môi trường **KeilC** và **STM32CubeIDE**.*  
   👉 **[Bấm vào đây để mở Bản Báo cáo HTML Giao diện Đẹp trực tiếp trên Trình duyệt](https://htmlpreview.github.io/?https://github.com/chinart123/Flash-STM32F103C8/blob/main/Fix-too-many-displays-issue-by-using-stlink-cli/final-document(better-UI-for-graph).html)**