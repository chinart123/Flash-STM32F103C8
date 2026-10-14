# Hướng dẫn dò vùng nhớ Option Bytes và Cấu trúc Complementary Bytes

### 1. Cách dò giá trị `0xA5` tại vùng địa chỉ `0x1FFF F800`

Giá trị `0xA5` (mã Hex) là chìa khóa báo hiệu chip đang ở trạng thái **Mở khóa hoàn toàn (Readout Protection Level 0)**. Vị trí chính xác của nó nằm ngay tại byte đầu tiên của phân vùng **Option Bytes**: địa chỉ `0x1FFF F800`.

Bạn có thể dò và đọc trực tiếp vùng nhớ này bằng 2 công cụ:

#### Cách 1: Xem bằng phần mềm STM32 ST-LINK Utility (Xem dữ liệu thô)
1. Cắm mạch nạp **ST-Link** vào máy tính và kết nối với chip.
2. Mở **ST-LINK Utility**, nhấn **Connect** (biểu tượng phích cắm).
3. Nhìn xuống cửa sổ **Memory display**, tại ô **Address** nhập vào `0x1FFFF800`.
4. Ô **Size** nhập `0x10` (để xem đủ 16 byte từ `800` đến `80F`). Nhấn **Enter**.
5. Bạn sẽ thấy cột giá trị hiện chuỗi số:
   ```text
   FFFF5AA5
   ```
   Do vi điều khiển đọc theo chuẩn **Little-Endian** (từ phải sang trái), nên byte ngoài cùng bên phải chính là `A5` (nằm tại `0x1FFF F800`), tiếp theo là `5A` (nằm tại `0x1FFF F801`).

> **💡 Mẹo (Lưu ý quan trọng):**
> Nếu bạn chọn thẻ **Target** -> **Option Bytes...**, phần mềm sẽ mở ra một bảng giao diện đồ họa. Tuy nhiên, **bảng này KHÔNG hiển thị mã Hex hay địa chỉ bộ nhớ**. Nó chỉ đóng vai trò như một "thông dịch viên", âm thầm đọc mã `A5` ở nền và dịch thành chữ **`Disabled`** để bạn kiểm tra trạng thái khóa chip một cách trực quan. Để xem giá trị Hex thô, bạn BẮT BUỘC phải nhìn ở cửa sổ **Memory display** bên ngoài.

#### Cách 2: Xem bằng cửa sổ Debug trong Keil C
1. Biên dịch code không lỗi và nhấn nút **Debug** để nạp code vào RAM/Flash.
2. Trên thanh menu, chọn **View** -> **Memory Windows** -> **Memory 1**.
3. Tại ô **Address** của cửa sổ vừa hiện ra, gõ chính xác: `0x1FFFF800` và nhấn **Enter**.
4. Bạn sẽ thấy ô bộ nhớ đầu tiên hiện lên giá trị `A5`. *(Nhấp chuột phải vào vùng nhớ, chọn **Unsigned / char** để xem từng byte một cho dễ nhìn)*.

---

### 2. Sơ đồ vùng nhớ User Data Bytes / Complementary Bytes

Vùng **Option Bytes** của STM32F1 trải dài **16 bytes** (từ `0x1FFF F800` đến `0x1FFF F80F`). ST chia 16 bytes này thành **8 cặp**. 

Nguyên tắc bắt buộc của ST (Complementary): **Byte địa chỉ lẻ luôn luôn phải là đảo bit logic (NOT) của byte địa chỉ chẵn liền trước nó.**

Dưới đây là bản đồ quy hoạch chi tiết:

| Địa chỉ (Chẵn) | Tên Byte gốc | Nội dung | Địa chỉ (Lẻ) | Tên Byte đảo (Complementary) |
| :--- | :--- | :--- | :--- | :--- |
| `0x1FFF F800` | **RDP** | Khóa đọc code (`0xA5` là mở) | `0x1FFF F801` | **nRDP** (Đảo bit của RDP) |
| `0x1FFF F802` | **USER** | Chỉnh Watchdog, Stop/Standby reset | `0x1FFF F803` | **nUSER** (Đảo bit của USER) |
| `0x1FFF F804` | **Data0** | Byte dữ liệu trống cho user | `0x1FFF F805` | **nData0** (Đảo bit của Data0) |
| `0x1FFF F806` | **Data1** | Byte dữ liệu trống cho user | `0x1FFF F807` | **nData1** (Đảo bit của Data1) |
| `0x1FFF F808` | **WRP0** | Khóa chống ghi Flash (Trang 0-31) | `0x1FFF F809` | **nWRP0** (Đảo bit của WRP0) |
| `0x1FFF F80A` | **WRP1** | Khóa chống ghi Flash (Trang 32-63) | `0x1FFF F80B` | **nWRP1** (Đảo bit của WRP1) |
| `0x1FFF F80C` | **WRP2** | Khóa chống ghi Flash (Trang 64-95) | `0x1FFF F80D` | **nWRP2** (Đảo bit của WRP2) |
| `0x1FFF F80E` | **WRP3** | Khóa chống ghi Flash (Trang 96-127)| `0x1FFF F80F` | **nWRP3** (Đảo bit của WRP3) |

#### 🔍 Giải phẫu cơ chế đối chiếu (Lý do chip Fake bị bắt bài):
Hãy lấy cặp **RDP** làm ví dụ:
1. Nếu chip ở trạng thái mở khóa, byte `0x1FFF F800` sẽ chứa `0xA5`. Đổi ra nhị phân là: `1010 0101`.
2. Byte đảo bit (phép NOT logic) của nó sẽ lật ngược toàn bộ `0` thành `1`, `1` thành `0`: `0101 1010`.
3. Đổi `0101 1010` ngược lại hệ Hex, ta có giá trị: `0x5A`.

Khi khởi động hoặc khi bị mạch nạp truy xuất, phần cứng của STM32 xịn sẽ tự động đọc cả 2 byte này và kiểm tra chéo:
```c
if ( Byte_0x1FFFF801 == ~(Byte_0x1FFFF800) ) {
    // OK, phần cứng an toàn, cấu trúc đúng chuẩn ST
    // -> Cho phép mạch nạp truy cập Flash
} else {
    // LỖI: Dữ liệu bị hỏng hoặc chip bị tấn công / kiến trúc sai lệch. 
    // -> Ngay lập tức khóa mọi truy cập, kích hoạt lỗi.
}
```

**Kết luận:** Chip Fake do có kiến trúc vật lý vùng ROM/EEPROM khác biệt, đôi khi phần cứng không tự sinh ra các byte đảo `nByte` tại các địa chỉ lẻ (hoặc đọc lên bị rác `0xFF`). Khi ST-Link đọc lên thấy byte chẵn là `A5` nhưng byte lẻ bên cạnh lại không phải `5A`, thuật toán lập tức xác định bộ nhớ bị lỗi -> Báo lỗi `Contents mismatch` và từ chối nạp code.