# Giải mã cơ chế "Bẫy nhận diện chip Fake" của STMicroelectronics

Để hiểu rõ tại sao phần mềm chính hãng của **STMicroelectronics** lại phát hiện ra con chip Fake ở **Bước 2.1**, chúng ta cần đi sâu vào cơ chế quản lý bảo vệ bộ nhớ phần cứng của lõi **ARM Cortex-M3**. 

Đây hoàn toàn là một cơ chế kiểm tra tính toàn vẹn của chip dựa trên các vùng nhớ đặc biệt, chứ không phải là một thuật toán "quét bản quyền" ẩn danh nào cả.

---

### 1. Bản chất cơ chế "Bẫy nhận diện" hoạt động thế nào?

Bên trong cấu trúc bộ nhớ của chip **STM32** xịn, ngoài vùng nhớ **Flash** dùng để chứa code (`0x0800 0000`), hãng ST có thiết kế một vùng nhớ hệ thống đặc biệt gọi là **Option Bytes** (nằm tại địa chỉ từ `0x1FFF F800` đến `0x1FFF F80F`).

Vùng nhớ **Option Bytes** này chứa 2 cấu phần cốt lõi:
1. **RDP (Readout Protection Byte)**: Byte quản lý việc khóa chip. Nếu byte này có giá trị là `0xA5`, chip ở trạng thái mở khóa hoàn toàn (**Level 0**). Nếu là giá trị khác, chip sẽ bị khóa không cho đọc code (**Level 1/2**).
2. **User Data Bytes / Complementary Bytes**: Các byte kiểm tra bổ sung. ST thiết kế quy định nghiêm ngặt: Mỗi một byte cấu hình được ghi xuống, bắt buộc phải có một byte đảo bit logic (**Complementary byte**) đi kèm song song để hệ thống đối chiếu tính toàn vẹn phần cứng.

#### 🔍 Quá trình thảm họa của chip Fake diễn ra tại đây:
Khi bạn sử dụng **Keil C** đời mới (hoặc chế độ mặc định của **ST-Link Utility**), trước khi nạp dữ liệu, phần mềm nạp sẽ gửi lệnh qua đường **SWD** để đọc toàn bộ vùng nhớ **Option Bytes** này nhằm xác minh xem chip có đang bật tính năng chống ghi (**Write Protection**) hay chống đọc (**Read Protection**) hay không.

* **Đối với chip STM32 xịn**: Nó trả về các byte dữ liệu và byte đảo bit trùng khớp hoàn hảo 100% theo đúng chuẩn của ST. Phần mềm nạp đồng ý cho qua.
* **Đối với chip STM32 Fake** *(APM32, CKS32, GD32 lột vỏ)*: Các hãng nhái của Trung Quốc tuy sao chép được cấu trúc thanh ghi ngoại vi (**GPIO, UART**) giống STM32, nhưng khối quản lý bộ nhớ non-volatile (**Flash/Option Bytes**) bên trong của chúng lại có kiến trúc vật lý khác biệt.

Khi phần mềm chính hãng của ST đọc vùng địa chỉ `0x1FFF F800`, con chip Fake trả về các giá trị byte bảo vệ bị lệch chuẩn, hoặc thiếu mất các byte đảo bit đối chiếu. Phần mềm nạp của ST ngay lập tức rơi vào trạng thái "hiểu lầm kỹ thuật". Nó thấy dữ liệu bảo vệ trả về bị sai cấu trúc, nó tưởng rằng bộ nhớ **Flash** của chip đang bị lỗi vật lý hoặc đang bị khóa bảo vệ nghiêm trọng. 

Trình nạp lập tức phát lệnh đánh sập hệ thống:
```text
Abort
```
Và từ chối nạp đè, kèm theo báo lỗi gạch chéo vĩnh viễn:
```text
Contents mismatch
```

---

### 2. Tại sao lệnh `-S` lại giải quyết được?

Khi bạn truyền tham số `-S` thông qua công cụ dòng lệnh `ST-LINK_CLI.exe`, bạn đang trực tiếp can thiệp vào thuật toán nạp của hệ thống. 

Tham số `-S` là viết tắt của **Skip flash protection verification**. Lệnh này ra chỉ thị cho mạch nạp: 
> *"Bỏ qua hoàn toàn, cấm không được gõ cửa đọc vùng nhớ **Option Bytes** (`0x1FFF F800`) của chip nữa. Cứ bốc thẳng file **.HEX** đổ trực tiếp vào phân vùng **Flash** (`0x0800 0000`)."*

Vì trình nạp bị "bịt mắt" không quét qua vùng nhớ bảo vệ nhạy cảm, con chip Fake sẽ không bị lộ các byte lệch chuẩn. Nó chỉ việc mở bộ nhớ **Flash** ra nhận dữ liệu sạch từ **Keil C** và biên dịch thành công mượt mà!

---

### 3. Tài liệu tham khảo chính hãng từ STMicroelectronics

Để kiểm chứng và đọc sâu hơn về cấu trúc phần cứng này, bạn có thể tra cứu trực tiếp trong các tài liệu kỹ thuật cốt lõi sau của hãng ST:

1. **STM32F10xxx Flash memory microarchitecture (Tài liệu PM0075)**
   * **Nội dung**: Đây là hướng dẫn lập trình bộ nhớ **Flash** chính thức của ST. Bạn hãy tìm đến **Chương 4: Option Bytes**. Tài liệu giải thích rất rõ cấu trúc của **RDP**, các byte đảo bit đối chiếu (`nHUSER`, `nRDP`) và cơ chế chip tự khóa hệ thống nếu dữ liệu đối chiếu này trả về sai lệch.
   * 🔗 [Đọc trực tiếp PM0075 tại ST.com](https://www.st.com/resource/en/programming_manual/cd00283419-stm32f10xxx-flash-memory-microarchitecture-stmicroelectronics.pdf)

2. **STM32F103xx Reference Manual (Tài liệu RM0008)**
   * **Nội dung**: Bản đồ toàn bộ thanh ghi ngoại vi của dòng **F1**. Bạn hãy tra cứu mục **Flash memory registers** để xem cách các bit hoạt động khi mạch nạp gõ cửa qua giao tiếp **SWD**.
   * 🔗 [Đọc trực tiếp RM0008 tại ST.com](https://www.st.com/resource/en/reference_manual/cd00171190-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)

3. **STM32 ST-LINK Utility User Manual (Tài liệu UM0892)**
   * **Nội dung**: Hướng dẫn sử dụng công cụ nạp. Tại **Chương 3.1: CLI Mode**, ST có liệt kê bảng tra cứu toàn bộ các tham số dòng lệnh. Bạn sẽ tìm thấy định nghĩa chính thức của lệnh giúp giải mã hoàn toàn quy trình xử lý của bạn: 
     ```bash
     -S : Skip flash protection verification
     ```
   * 🔗 [Đọc trực tiếp UM0892 tại ST.com](https://www.st.com/resource/en/user_manual/um0892-stm32-stlink-utility-software-description-stmicroelectronics.pdf)

---
> **🚀 Next Step:**
> Khi cơ chế bẫy bộ nhớ Option Bytes của chip Fake và các tài liệu chính hãng đi kèm đã được làm rõ, chúng ta sẽ chuyển sang cấu hình ngoại vi **UART bằng thư viện LL (Low-Layer)** để bắt đầu truyền dữ liệu lên máy tính.