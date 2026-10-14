# Nhật Ký Thảo Luận: Giải Mã Bản Chất Hệ Thống Nạp & Gỡ Lỗi STM32 Trong Môi Trường Keil uVision

> **Trạng thái:** File tài liệu ghi nhận thực nghiệm của người dùng, phân tích hành vi hệ thống, và các điểm sập kiến thức (miss knowledge) của AI cũ để chuyển giao sang mô hình Claude phân tích phương án cứu vãn tầng sâu.

---

## 1. Vị Trí Của File ST-LINKIII-KEIL_SWO.dll Trên Máy Tính

### Câu hỏi:
ST-LINKIII-KEIL_SWO.dll nằm ở đâu trong máy?

### Bản chất hệ thống:
File **ST-LINKIII-KEIL_SWO.dll** (driver nạp và gỡ lỗi ST-Link trong môi trường Keil uVision) mặc định nằm ở các đường dẫn sau tùy thuộc vào phiên bản Keil được cài đặt:

* **Đối với các phiên bản Keil uVision 5 cũ (v5.39 trở về trước) hoặc Keil uVision 4:**
  `C:\Keil_v5\ARM\STLink\` hoặc `C:\Keil\ARM\STLink\`
* **Đối với Keil uVision bản mới (từ v5.40 trở đi):**
  Do Keil thay đổi vị trí cài đặt mặc định để bảo mật, file sẽ nằm ở:
  `%LOCALAPPDATA%\Keil_v5\ARM\STLink\`

---

## 2. Phân Tích Sự Khác Biệt Giữa Nhật Ký Biên Dịch (Build Log) Trong Keil uVision

### Thực nghiệm của người dùng:
Người dùng cung cấp hai tài liệu Build Log của Keil uVision sử dụng ArmClang V6.24 và Armcc V5.06 để đối chiếu hành vi sinh mã máy.

| Đặc tính | Log thứ nhất (ArmClang V6.24) | Log thứ hai (Armcc V5.06) |
| :--- | :--- | :--- |
| **Bộ biên dịch (Compiler)** | **ArmClang (Arm Compiler v6 - AC6)** | **Armcc (Arm Compiler v5 - AC5)** |
| **Nền tảng cốt lõi** | Phát triển dựa trên mã nguồn mở **LLVM / Clang** hiện đại. | Công nghệ đóng **chính chủ độc quyền** của ARM (Legacy). |
| **Thư mục chứa** | `...\ARM\ARMCLANG\Bin` | `...\ARM\ARMCC\Bin` |
| **Kích thước mã (Code Size)** | **576 Bytes** (Tối ưu dung lượng tốt hơn) | **644 Bytes** (Tốn bộ nhớ Flash hơn một chút) |
| **Tiêu chuẩn C hỗ trợ** | Hỗ trợ đầy đủ **C11, C18 và C++14/C++17**. | Chỉ dừng lại ở tiêu chuẩn cũ **C89 / C99**. |
| **Tốc độ biên dịch** | **3 giây** | **1 giây** |

---

## 3. Nhật Ký Thực Nghiệm Đo Đạc Thanh Ghi & Hiện Tượng Xung Đột Xung Nhịp (72MHz vs 8MHz)

### Dữ liệu thực tế do người dùng đo đạc bằng ST-Link Utility độc lập:

1. **Khi không nạp file `.ini`, các địa chỉ Vector Table bị xáo trộn byte ma:**
   * `0x08000000`: `08000179 0800016D 20000660 08000177` HOẶC khi live update: `08000175 0800016D 20000660 08000177`
   * `0x08000004`: `08000177 20000660 08000177 08000175` HOẶC khi live update: `0800017B 20000660 08000177 08000175`
   * Hậu quả: Hiện tượng ghi đè/chồng chập dữ liệu tạo ra địa chỉ rác khiến ST-Link Utility báo lỗi liên hoàn: `Error occured during flash mass erase!`
2. **Hiện tượng biến thiên thanh ghi RCC và FLASH_ACR dưới tác động của file `.ini`:**
   * **Trạng thái Crash Nhịp Cao (72MHz - 2 Wait States):**
     ```text
     RCC_CFGR=0x001D040A  FLASH_ACR=0x00000032
     Flash wait states (LATENCY) = 2
     SYSCLK = PLL x9 = 72 MHz
     ```
   * **Trạng thái Ép Về Nhịp Nền An Toàn (8MHz - 0 Wait State) bởi file `.ini`:**
     ```text
     RCC_CFGR=0x001D0000  FLASH_ACR=0x00000000
     Flash wait states (LATENCY) = 0
     SYSCLK = HSI = 8 MHz
     ```
3. **Mâu thuẫn hành vi giữa hai phần mềm:**
   * **ST-Link Utility:** Nạp file `.hex` ăn liền, phát lệnh Hardware Reset giải phóng chip chạy ngay lập tức.
   * **Keil C (Sử dụng Use Debug Driver):** Ép chip vào trạng thái Halt gỡ lỗi ngầm qua file `ST-LINKIII-KEIL_SWO.dll`. Sinh ra dòng chữ mập mờ `Application running...` nhưng thực chất giữ kết nối SWD khiến chip không chạy độc lập được, đồng thời gây xung đột khóa Flash Memory Controller.

---

## 4. Xác Định Phần Cứng Độc Lập

### Khẳng định phần cứng:
Bằng chứng thực nghiệm từ địa chỉ thanh ghi gốc `0xE000ED00` (CPUID) trả về giá trị **`0x411FC231`**. Đây là chip **STM32F103C8T6 chính hãng (Real) 100%** từ nhà sản xuất STMicroelectronics. Hiện tượng sập tool hoàn toàn là lỗi phần mềm hệ thống/toolchain.

---

## 5. Điểm Sập Kiến Thức (Miss Knowledge) Của AI Và Hiện Tượng Lỗi Chưa Giải Quyết

### Hiện tượng lỗi thực tế:
Khi người dùng add file `.ini` vào mục `Init File` của tab Utilities (bất kể file đó dùng cấu hình hàm đệ quy hay đã được sửa đổi thành dạng script phẳng tuần tự tuyến tính), ngay khi nhấn nút **Flash / Load (F8)**, **phần mềm Keil C lập tức bị tự động thoát phụt ra ngoài (Crash / Out ứng dụng) vĩnh viễn**. Kéo theo đó, khi bấm nút Reset vật lý trên mạch nạp, LED chân PC13 vẫn đứng im hoàn toàn không nhấp nháy.

### Các lý do sai lầm mà AI cũ đã đưa ra (Cần Claude gạt bỏ và chỉnh đốn):
1. *Đổ lỗi cho chất lượng silicon (Chip Fake/Nhái) xử lý sai lệnh Halt gỡ lỗi.* (Sai - Chip đã được chứng minh là hàng chính hãng bằng CPUID).
2. *Đổ lỗi cho cấu trúc hàm đệ quy vô hạn gây Stack Overflow làm Windows ép đóng tiến trình UV4.exe.* (Sai - Vì khi chuyển sang file phẳng không chứa hàm, Keil C vẫn lập tức sập nguồn ứng dụng).

### Gợi ý hướng phân tích sâu cho Claude:
* Tại sao trình xử lý lệnh gỡ lỗi ngầm của Keil C (`ST-LINKIII-KEIL_SWO.dll`) lại chủ động đóng/sập ứng dụng mẹ khi gặp các câu lệnh truy cập trực tiếp bộ nhớ (`_RWORD`, `_WWORD` hoặc lệnh `g`) ngay tại thời điểm Flash Programming ở tab Utilities?
* Có cơ chế ngầm nào trong cấu hình quản lý Run-time Environment của Keil uVision ép đóng kết nối kết hợp với việc găm giữ trạng thái khóa thanh ghi bảo vệ của chip hay không?

---

## 6. Đống Mã Lệnh File .ini Gây Lỗi Sập Để Làm Tư Liệu Phân Tích

### Bản 1 (Chứa hàm đệ quy lỗi logic):
```text
FUNC void CheckAndSyncSystem (void) {
    _WWORD(0xE000ED0C, 0x05FA0004); // Lệnh gọi SYSRESETREQ
    CheckAndSyncSystem(); 
}
CheckAndSyncSystem();
```

### Bản 2 (Dạng phẳng phẳng tuyến tính nhưng Keil C VẪN SẬP PHỤT):
```text
define unsigned int rcc_cfgr;
define unsigned int flash_acr;
rcc_cfgr = _RWORD(0x40021004); 
flash_acr = _RWORD(0x40022000); 

if ((rcc_cfgr & 0x0000000C) == 0x00000008) {
    _WWORD(0x40022000, (flash_acr & ~0x00000003) | 0x00000002); 
}
PC = _RWORD(0x08000004); 
g
```
