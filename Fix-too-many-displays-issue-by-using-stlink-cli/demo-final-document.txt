
BÁO CÁO TOÀN TẬP: ĐIỀU TRA LỖI Contents mismatch TRÊN KEILC VÀ THIẾT LẬP STM32CUBEIDE (LL LIBRARY)
Tài liệu này lưu trữ toàn bộ hồ sơ điều tra, số đo thực nghiệm, phân tích dịch ngược thuật toán nạp .FLM, giải phẫu dữ liệu bộ nhớ Flash ở cấp độ chu kỳ xung nhịp trên Keil MDK (UV4.exe), cùng quy trình chuyển đổi hoàn chỉnh sang STM32CubeIDE sử dụng 100% thư viện LL (Low-Layer).
Bảng nhãn độ tin cậy dữ liệu trong báo cáo:





























NhãnÝ nghĩa[Đã đo]Có log thực tế, ảnh chụp hoặc số đo thanh ghi trực tiếp từ phần cứng của dự án[Tài liệu]Có trong tài liệu chính thức của Keil / STMicroelectronics / IAR / CMSIS-Pack[Cộng đồng]Báo cáo thực tế của kỹ sư trên các diễn đàn kỹ thuật (ST, Arm, EEVblog, CSDN)[Suy luận]Suy luận kỹ thuật hợp lý khớp với kiến trúc phần cứng nhưng chưa đo trực tiếp xung[Chưa thử]Đề xuất thí nghiệm trong ma trận kiểm chứng, chưa chạy thực tế

MỤC LỤC TƯƠNG TÁC

PHẦN 1: ĐIỀU TRA, KIỂM THỬ CHÉO VÀ GIẢI PHẪU LỖI Contents mismatch TRÊN KEILC

1.1. Môi trường thử nghiệm và Triệu chứng lỗi
1.2. Loại trừ giả thuyết và Kiểm thử chéo Phần cứng (Chip & ST-Link Thật/Clone)
1.3. Cô lập nhánh nạp, Quan sát dữ liệu Flash và Dịch ngược thuật toán .FLM
1.4. Sơ đồ luồng nạp của UV4.exe và Giải mã các tùy chọn Connect / Reset
1.5. Quy trình Kiểm chứng chuẩn và Ma trận Thí nghiệm (Đề xuất từ Claude)
1.6. Khắc phục bằng flash_init.ini và Phân tích Log Thanh ghi (72 MHz vs 8 MHz)
1.7. Tổng hợp Dẫn chứng từ Datasheet, Reference Manual, Blog và Cộng đồng


PHẦN 2: THIẾT LẬP PROJECT TRÊN STM32CUBEIDE (THƯ VIỆN LL)

2.1. Khởi tạo Project và chọn vi điều khiển
2.2. Cấu hình chân Pinout và nguồn xung RCC
2.3. Cấu hình sơ đồ xung nhịp Clock Configuration (72 MHz)
2.4. Chuyển đổi Driver từ HAL sang LL
2.5. Xử lý lỗi mất kết nối SWD (Connection timed out)


PHẦN 3: CẤU HÌNH FLASH BẰNG EXTERNAL TOOLS (PHÂN TÁCH 2 CHẾ ĐỘ NẠP)

3.1. Chế độ A: Nạp xong chạy ngay (Flash & Run)
3.2. Chế độ B: Nạp xong chờ bấm Reset vật lý (External Tools)


PHẦN 4: GHI CHÚ ĐẶC BIỆT VỀ JUMPER BOOT0 TRÊN STM32CUBEIDE

4.1. Hiện tượng thực tế và giả thuyết ban đầu
4.2. Giải thích cơ chế can thiệp lõi của SWD và GDB Server


PHẦN 5: TOÀN BỘ SOURCE CODE main.c (KEILC & STM32CUBEIDE)

5.1. Bản main.c cải tiến có Timeout an toàn cho KeilC
5.2. Bản main.c hoàn chỉnh trên STM32CubeIDE





PHẦN 1: ĐIỀU TRA, KIỂM THỬ CHÉO VÀ GIẢI PHẪU LỖI Contents mismatch TRÊN KEILC

1.1. Môi trường thử nghiệm và Triệu chứng lỗi
A. Cấu trúc thư mục Project trên KeilC (Add file thủ công vs RTE)
Dự án gốc LED_PC13_LL trên KeilC được tổ chức bằng cách tự thêm (add) từng tệp thư viện LL và CMSIS vào thư mục Drivers và User, không thông qua hệ thống quản lý gói RTE (Run-Time Environment):
C:\Users\ASUS\Desktop\LED_PC13_LL|   EventRecorderStub.scvd|   flash_debug.log|   flash_init.ini|   JLinkLog.txt|   JLinkSettings.ini|   main.c|   STM32F103C8.uvguix.ASUS|   STM32F103C8.uvoptx|   STM32F103C8.uvprojx|   STM32F103C8_Sequences_0000.log -> STM32F103C8_Sequences_0082.log|+---DebugConfig|       Target_1_STM32F103C8_1.0.0.dbgconf|+---Drivers|       cmsis_armcc.h|       cmsis_armclang.h|       cmsis_compiler.h|       cmsis_version.h|       core_cm3.h|       stm32f103xb.h|       stm32f1xx.h|       stm32f1xx_ll_bus.h|       stm32f1xx_ll_gpio.h|       stm32f1xx_ll_rcc.h|       stm32f1xx_ll_system.h|       system_stm32f1xx.h|+---Listings|       startup_stm32f103xb.lst|       STM32F103C8.map|+---Objects|       STM32F103C8.axf|       STM32F103C8.hex|       ...+---RTE|   \---Device|       \---STM32F103C8|               STM32F101_102_103_105_107.dbgconf|               STM32F101_102_103_105_107.dbgconf.base@1.0.0|\---User        startup_stm32f103xb.s        system_stm32f1xx.c
Đánh giá việc tự add file thủ công so với dùng RTE:

Việc tự gom các tệp .h, .c, .s vào thư mục riêng hay tích chọn qua giao diện RTE (Run-Time Environment) chỉ khác nhau ở khâu quản lý mã nguồn trước khi biên dịch (Compile & Link) để tạo ra tệp thực thi STM32F103C8.axf.
Khi nhấn phím F8 (Download), KeilC vẫn gọi cùng một trình điều khiển ST-Link và sử dụng chính xác cùng một tệp thuật toán nạp STM32F10x_128.FLM nằm trong gói CMSIS-Pack của dòng F1. Do đó, dù có chuyển sang dùng RTE thì lỗi Contents mismatch vẫn xảy ra y hệt 100%.

B. Rà soát mã nguồn main.c (Hàm SystemClock_Config())
Trước khi nghi ngờ công cụ nạp, mã nguồn main.c viết bằng thư viện LL đã được rà soát kỹ từng dòng so với chuẩn Reference Manual (RM0008):





















































Hạng mụcmain.c thực hiệnChuẩn tài liệu ST (RM0008)Kết luậnWait State ở 72 MHzLL_FLASH_SetLatency(LL_FLASH_LATENCY_2)0 WS cho 0–24 MHz0 \text{--} 24\text{ MHz}0–24 MHz, 1 WS cho 24–48 MHz24 \text{--} 48\text{ MHz}24–48 MHz, 2 WS cho 48–72 MHz48 \text{--} 72\text{ MHz}48–72 MHz [Tài liệu]Đúng chuẩn.Thứ tự thiết lậpĐặt Latency sau khi PLL sẵn sàng, trước khi chuyển SYSCLK sang PLLBắt buộc phải mở Latency trước khi nâng xung hệ thống lên 72 MHz72\text{ MHz}72 MHz [Cộng đồng]Đúng. (CubeMX đặt Latency ngay đầu hàm cũng hợp lệ).Prefetch BufferKhông can thiệpGiá trị mặc định sau Reset của FLASH_ACR là 0x30 (bit Prefetch tự bật). Khi firmware chạy sẽ thành 0x32 [Đã đo]Ổn.Bộ chia PrescalerAHB /1, APB1 /2, APB2 /1Bus APB1 tối đa chịu được 36 MHz36\text{ MHz}36 MHz, bắt buộc chia 2 khi SYSCLK ở 72 MHz72\text{ MHz}72 MHz [Tài liệu]Đúng chuẩn.Vòng chờ HSE/PLLwhile(...) không có giới hạn (Timeout)Nếu thạch anh ngoài hỏng hoặc lỏng chân, MCU sẽ treo vĩnh viễn tại vòng lặp while [Cộng đồng]Rủi ro treo mạch, nhưng không phải nguyên nhân gây lỗi nạp.Biến SystemCoreClockKhông gọi hàm cập nhậtCần gọi LL_SetSystemCoreClock(72000000) nếu các hàm thư viện khác phụ thuộc vào biến này [Suy luận]Không liên quan lỗi nạp Flash.Hàm Delay(90000)Comment ghi 4500000, nhưng code thực tế truyền vào 90000Ở tốc độ 72 MHz72\text{ MHz}72 MHz, vòng lặp 90000 chạy hết chỉ vài mili-giây, làm LED PC13 chớp cực nhanh trông như sáng mờ [Đã đo]Dễ gây nhầm lẫn là "chip bị treo không chạy".
C. Kịch bản chuẩn tái hiện triệu chứng lỗi

Nếu comment dòng SystemClock_Config(); trong main() (để chip chạy ở xung nội mặc định HSI 8 MHz), nhấn F8 liên tục bao nhiêu lần cũng báo Verify OK [Đã đo].
Khi mở lại SystemClock_Config(); (đưa chip lên 72 MHz):

Nhấn F8 lần #1 (khi chip đang ở 8 MHz hoặc vừa xóa sạch): Nạp thành công, báo Programming Done. Verify OK.
Nhấn nút RESET vật lý trên board: Chip khởi động lại và thực thi code mới, đẩy xung nhịp hệ thống lên 72 MHz (LED chớp nhanh).
Nhấn F8 lần #2 (ngay khi chip đang chạy ở 72 MHz): Quá trình Erase và Program báo hoàn tất, nhưng đến bước Verify lập tức văng lỗi Contents mismatch và dừng quá trình nạp [Đã đo].



⬆️ Quay về Mục lục


1.2. Loại trừ giả thuyết và Kiểm thử chéo Phần cứng (Chip & ST-Link Thật/Clone)
A. Bảng tổng hợp 12 bằng chứng thực nghiệm đã đo
































































































#Thí nghiệm thực hiệnKết quả thu đượcÝ nghĩa kỹ thuậtNhãn1Comment SystemClock_Config() (chip ở HSI 8 MHz), không dùng .iniHết hoàn toàn lỗi mismatchTốc độ 72 MHz là điều kiện kích hoạt bắt buộc của lỗi[Đã đo]2Bật lại 72 MHz, LED xác nhận chạy nhanh. Bấm F8 #1 -> Nhấn RESET trên board -> Bấm F8 #2Lần #1 Verify OK, lần #2 nổ Contents mismatchLỗi tái hiện ổn định 100% theo chu kỳ, không phải lỗi ngẫu nhiên[Đã đo]3Lặp lại quy trình trên với Board thứ 2 và mạch nạp ST-Link V2 thứ 2Kết quả lỗi y hệt 100%Đây là lỗi hệ thống (Systemic issue), không phải do một linh kiện bị hỏng[Đã đo]4Thử nghiệm trên Board thứ 3 có mã định danh 0xE0042000 = 0x20036410Vẫn bị Contents mismatch ở lần F8 #2Bác bỏ hoàn toàn giả thuyết "do chip giả/clone mới bị lỗi"[Đã đo]5Phân tích mẫu địa chỉ sai trong log F8 #2Tại mọi địa chỉ chia hết cho 8, byte Flash= bằng đúng byte Required= ở địa chỉ - 8 (VD: tại 0x08000010 đọc ra 75H, bằng đúng byte cần ghi tại 0x08000008)Word đầu của mỗi cặp 64-bit trả về dữ liệu của cặp 64-bit trước đó -> Bằng chứng vật lý của việc đọc Flash thiếu Wait State[Đã đo]6Dịch ngược hàm Init của file STM32F10x_128.FLMPhát hiện lệnh ghi cứng FLASH->ACR = 0; hàm UnInit không khôi phục lạiTìm ra thủ phạm trực tiếp phía thuật toán nạp của KeilC[Đã đo]7Đối chiếu mã nguồn mẫu FlashPrg.c của Keil (trích dẫn trên CSDN)Hàm Init đặt FLASH->ACR = 0 kèm comment "Zero Wait State"Khớp tuyệt đối với kết quả dịch ngược ở mục 6, xác nhận đây là thiết kế gốc của tệp mẫu Keil[Cộng đồng]8Nối dây chân NRST, đặt Reset = Hardware Reset trong Keil, không dùng .iniVẫn bị Contents mismatch ở lần F8 #2Cấu hình Hardware Reset mặc định này chưa đủ để đưa chip về HSI trước khi nạp[Đã đo]9Mở ST-LINK Utility, chọn Connect Under Reset, đọc thanh ghi 0x40021004 (RCC_CFGR) khi firmware đang chạy 72 MHzGiá trị trả về vẫn là 0x001D040A (đang ở PLL 72 MHz)Tín hiệu Reset phần cứng không truyền tới được chân reset của chip (hoặc mạch ST-Link clone không phát xung NRST)[Đã đo]10Sử dụng script flash_init.ini ép thanh ghi RCC về HSI 8 MHz trước khi nạpNhấn F8 chạy ổn định liên tiếp nhiều lần, bất kể tắt hay bật Hardware ResetGiải pháp phần mềm can thiệp đúng điểm yếu, khắc phục thành công trên KeilC[Đã đo]11Chạy thử phiên bản script .ini khác (Bản 2)Làm phần mềm Keil UV4.exe bị crash văng khỏi màn hình (Mã lỗi 0xC000041D, module unknown)Engine thông dịch script của Keil bị sập khi gặp cú pháp/địa chỉ không tương thích trong file .ini[Đã đo]12Kiểm tra thanh ghi 0xE0042000 (DBGMCU_IDCODE) trên 2 board đầu tiênTrả về 0x00006410 (DEV_ID = 0x410 đúng dòng F103 Medium-density, nhưng REV_ID = 0x0000 không có trong tài liệu RM0008 của ST)Hai board đầu có silicon khác với board thứ ba (0x20036410), nhưng cả hai loại silicon đều gặp lỗi giống hệt nhau[Suy luận]
B. Bảng các giả thuyết đã bị bác bỏ hoàn toàn








































Giả thuyết ban đầuTrạng tháiLý do bác bỏ dựa trên bằng chứng thực nghiệm"Do mua phải chip STM32 giả/clone nên mới bị lỗi nạp"Bác bỏBoard thứ 3 có mã Revision chuẩn chính hãng ST (0x20036410 - nhóm Rev 1/2/3/X) vẫn nổ lỗi Contents mismatch y hệt."CPUID 0x411FC231 chứng minh 100% chip chính hãng"Bác bỏThanh ghi CPUID (0xE000ED00) chỉ định danh kiến trúc lõi của ARM (Cortex-M3 r1p1), mọi hãng làm chip clone dùng lõi M3 đều có cùng mã này. Phải xem DBGMCU_IDCODE (0xE0042000)."Chỉ cần nối chân NRST và bật Hardware Reset là hết lỗi"Bác bỏĐã nối dây và đo thực tế (Thí nghiệm #8 & #9): Xung reset không kéo được thanh ghi RCC_CFGR (0x001D040A) về 0 trên mạch ST-Link V2 đang dùng."Dữ liệu trong Flash bị ghi đè chồng chập / ghi lỗi"Bác bỏMỗi lần bấm F8 đều có bước Full Chip Erase Done. và Programming Done.. Kiểm tra độc lập trên ST-LINK Utility cho thấy dữ liệu nằm trong Flash đã được ghi đúng 100%, lỗi chỉ xảy ra trên đường đọc về (Verify)."Do bộ nhớ Flash nhỏ hoặc nạp nhiều lần làm hỏng ô nhớ Flash"Bác bỏTuổi thọ bộ nhớ Flash của STM32F103 chịu được tối thiểu 10.00010.00010.000 chu kỳ xóa/ghi."Dòng thông báo Application running ... là báo lỗi"Bác bỏĐó là thông báo hoàn toàn bình thường của KeilC khi tùy chọn Reset and Run được kích hoạt sau khi nạp.
⬆️ Quay về Mục lục


1.3. Cô lập nhánh nạp, Quan sát dữ liệu Flash và Dịch ngược thuật toán .FLM
A. Nguyên văn Log lỗi Contents mismatch trên KeilC
Dưới đây là toàn bộ log xuất ra từ cửa sổ Build Output của KeilC khi nhấn F8 ở lần nạp thứ #2 (khi chip đang chạy ở xung nhịp 72 MHz):
Load "C:\\Users\\ASUS\\Desktop\\LED_PC13_LL\\Objects\\STM32F103C8.axf" Full Chip Erase Done.Programming Done.Contents mismatch at: 08000000H  (Flash=28H  Required=60H) !Contents mismatch at: 08000001H  (Flash=01H  Required=06H) !Contents mismatch at: 08000003H  (Flash=08H  Required=20H) !Contents mismatch at: 08000008H  (Flash=60H  Required=75H) !Contents mismatch at: 08000009H  (Flash=06H  Required=01H) !Contents mismatch at: 0800000BH  (Flash=20H  Required=08H) !Contents mismatch at: 08000010H  (Flash=75H  Required=79H) !Contents mismatch at: 08000018H  (Flash=79H  Required=7DH) !Contents mismatch at: 08000020H  (Flash=7DH  Required=00H) !Contents mismatch at: 08000021H  (Flash=01H  Required=00H) !Contents mismatch at: 08000023H  (Flash=08H  Required=00H) !Contents mismatch at: 08000030H  (Flash=00H  Required=81H) !Contents mismatch at: 08000031H  (Flash=00H  Required=01H) !Contents mismatch at: 08000033H  (Flash=00H  Required=08H) !Contents mismatch at: 08000038H  (Flash=81H  Required=83H) !Contents mismatch at: 08000040H  (Flash=83H  Required=87H) !Contents mismatch at: 080000F0H  (Flash=87H  Required=00H) !Contents mismatch at: 080000F1H  (Flash=01H  Required=F0H) !Contents mismatch at: 080000F2H  (Flash=00H  Required=2CH) !Contents mismatch at: 080000F3H  (Flash=08H  Required=F8H) !Contents mismatch at: 080000F9H  (Flash=F0H  Required=0CH) !Contents mismatch at: 080000FAH  (Flash=2CH  Required=82H) !Contents mismatch at: 080000FBH  (Flash=F8H  Required=44H) !Contents mismatch at: 08000100H  (Flash=00H  Required=01H) !Contents mismatch at: 08000101H  (Flash=0CH  Required=07H) !Contents mismatch at: 08000102H  (Flash=82H  Required=DAH) !Contents mismatch at: 08000103H  (Flash=44H  Required=45H) !Contents mismatch at: 08000108H  (Flash=01H  Required=21H) !Contents mismatch at: 08000109H  (Flash=07H  Required=F8H) !Contents mismatch at: 0800010AH  (Flash=DAH  Required=AFH) !Contents mismatch at: 0800010BH  (Flash=45H  Required=F2H) !Contents mismatch at: 08000110H  (Flash=21H  Required=0FH) !Contents mismatch at: 08000111H  (Flash=F8H  Required=00H) !Contents mismatch at: 08000112H  (Flash=AFH  Required=13H) !Contents mismatch at: 08000113H  (Flash=F2H  Required=F0H) !Contents mismatch at: 08000118H  (Flash=0FH  Required=FBH) !Contents mismatch at: 08000119H  (Flash=00H  Required=1AH) !Contents mismatch at: 0800011AH  (Flash=13H  Required=43H) !Contents mismatch at: 08000120H  (Flash=FBH  Required=CCH) !Contents mismatch at: 08000121H  (Flash=1AH  Required=02H) !Contents mismatch at: 08000122H  (Flash=43H  Required=00H) !Contents mismatch at: 08000123H  (Flash=F0H  Required=00H) !Contents mismatch at: 08000128H  (Flash=CCH  Required=00H) !Contents mismatch at: 08000129H  (Flash=02H  Required=23H) !Contents mismatch at: 0800012BH  (Flash=00H  Required=24H) !Contents mismatch at: 08000130H  (Flash=00H  Required=10H) !Contents mismatch at: 08000131H  (Flash=23H  Required=3AH) !Contents mismatch at: 08000132H  (Flash=00H  Required=28H) !Contents mismatch at: 08000133H  (Flash=24H  Required=BFH) !Contents mismatch at: 08000138H  (Flash=10H  Required=52H) !Contents mismatch at: 08000139H  (Flash=3AH  Required=07H) !Contents mismatch at: 08000140H  (Flash=52H  Required=0BH) !Contents mismatch at: 08000141H  (Flash=07H  Required=60H) !Contents mismatch at: 08000142H  (Flash=28H  Required=70H) !Contents mismatch at: 08000143H  (Flash=BFH  Required=47H) !Contents mismatch at: 08000148H  (Flash=0BH  Required=10H) !Contents mismatch at: 08000149H  (Flash=60H  Required=B5H) !Contents mismatch at: 0800014AH  (Flash=70H  Required=10H) !Contents mismatch at: 0800014BH  (Flash=47H  Required=BDH) !Contents mismatch at: 08000150H  (Flash=10H  Required=11H) !Contents mismatch at: 08000151H  (Flash=B5H  Required=46H) !Contents mismatch at: 08000152H  (Flash=10H  Required=FFH) !Contents mismatch at: 08000153H  (Flash=BDH  Required=F7H) !Contents mismatch at: 08000158H  (Flash=11H  Required=DBH) !Contents mismatch at: 08000159H  (Flash=46H  Required=F8H) !Contents mismatch at: 0800015AH  (Flash=FFH  Required=00H) !Contents mismatch at: 0800015BH  (Flash=F7H  Required=F0H) !Contents mismatch at: 08000160H  (Flash=DBH  Required=FFH) !Contents mismatch at: 08000161H  (Flash=F8H  Required=F7H) !Contents mismatch at: 08000162H  (Flash=00H  Required=F2H) !Contents mismatch at: 08000163H  (Flash=F0H  Required=FFH) !Contents mismatch at: 08000168H  (Flash=FFH  Required=57H) !Contents mismatch at: 08000169H  (Flash=F7H  Required=F8H) !Contents mismatch at: 0800016AH  (Flash=F2H  Required=00H) !Contents mismatch at: 0800016BH  (Flash=FFH  Required=00H) !Contents mismatch at: 08000170H  (Flash=57H  Required=09H) !Contents mismatch at: 08000171H  (Flash=F8H  Required=48H) !Contents mismatch at: 08000173H  (Flash=00H  Required=47H) !Contents mismatch at: 08000178H  (Flash=09H  Required=FEH) !Contents mismatch at: 08000179H  (Flash=48H  Required=E7H) !Contents mismatch at: 0800017AH  (Flash=00H  Required=FEH) !Contents mismatch at: 0800017BH  (Flash=47H  Required=E7H) !Contents mismatch at: 08000188H  (Flash=FEH  Required=04H) !Contents mismatch at: 08000189H  (Flash=E7H  Required=48H) !Contents mismatch at: 0800018AH  (Flash=FEH  Required=05H) !Contents mismatch at: 0800018BH  (Flash=E7H  Required=49H) !Contents mismatch at: 08000190H  (Flash=04H  Required=70H) !Contents mismatch at: 08000191H  (Flash=48H  Required=47H) !Contents mismatch at: 08000192H  (Flash=05H  Required=00H) !Contents mismatch at: 08000193H  (Flash=49H  Required=00H) !Contents mismatch at: 08000198H  (Flash=70H  Required=EDH) !Contents mismatch at: 08000199H  (Flash=47H  Required=00H) !Contents mismatch at: 0800019BH  (Flash=00H  Required=08H) !Contents mismatch at: 080001A0H  (Flash=EDH  Required=60H) !Contents mismatch at: 080001A1H  (Flash=00H  Required=06H) !Contents mismatch at: 080001A3H  (Flash=08H  Required=20H) !Contents mismatch at: 080001A9H  (Flash=06H  Required=02H) !Contents mismatch at: 080001B0H  (Flash=60H  Required=70H) !Contents mismatch at: 080001B1H  (Flash=02H  Required=47H) !Contents mismatch at: 080001B2H  (Flash=00H  Required=75H) !Too many errors to display !Error: Flash Download failed  -  "Cortex-M3"Flash Load finished at 12:22:23
B. Giải phẫu Log Contents mismatch: Cách nhìn vào Log để biết ngay chip bị thiếu 2 Wait States ở bước Verify
Đoạn log trên không phải là những con số ngẫu nhiên bị hỏng (corrupt), mà chứa đựng một quy luật toán học và vật lý tuyệt đối giúp phân biệt ngay lập tức lỗi nằm ở giai đoạn nào của quá trình nạp Flash:
1. Phân biệt các giai đoạn trong chu trình Flash:

Giai đoạn 1 (Full Chip Erase Done.): Bộ điều khiển Flash xóa toàn bộ chip (kéo tất cả các bit lên 0xFF). Quá trình này dùng xung nội của bộ điều khiển Flash nên Thành công 100%.
Giai đoạn 2 (Programming Done.): CPU lấy dữ liệu từ Buffer trong RAM ghi xuống từng nửa từ (Half-word 16-bit) của Flash. Do tốc độ ghi Flash rất chậm (cỡ 40–50 μs40\text{--}50\ \mu\text{s}40–50 μs mỗi Half-word) và CPU phải đợi cờ BSY (Busy) trong thanh ghi FLASH_SR hạ xuống, việc thiếu Wait State đọc không làm hỏng quá trình ghi. Thực tế khi dùng ST-LINK Utility đọc lại vùng nhớ 0x08000000 sau khi Keil báo lỗi, toàn bộ dữ liệu trong Flash đều khớp 100% với cột Required! Tức là quá trình Program đã Thành công 100%.
Giai đoạn 3 (Verify - Đối chiếu dữ liệu): Keil đọc tuần tự từng Word (32-bit = 4 byte) từ địa chỉ 0x08000000 trở đi để so sánh với tệp .axf. Chính tại giai đoạn đọc tốc độ cao này, dữ liệu đọc về (Flash=) bị trượt nhịp!

2. Phát hiện "Quy luật trượt 8-Byte (64-bit Fetch Latch)" ngay trên bảng Log:
Nếu đặt các dòng log nằm trên biên địa chỉ chia hết cho 8 (0x...0 và 0x...8) cạnh nhau, ta thấy một hiện tượng đáng kinh ngạc: Giá trị Flash= đọc được tại địa chỉ AAA luôn bằng chính xác giá trị Required= của địa chỉ (A−8)(A - 8)(A−8) (cách đó đúng 8 byte / 64 bit)!

























































































Địa chỉ hiện tại (AAA)Dữ liệu đọc về sai (Flash=) tại AAADữ liệu đúng cần có (Required=) tại AAAĐối chiếu với Required= tại (A−8)(A - 8)(A−8)0x08000000..0328H 01H 00H 08H60H 06H 00H 20H (Word 0x20000660)(Dữ liệu còn sót từ chu kỳ đọc cuối trước đó)0x08000008..0B60H 06H 00H 20H75H 01H 00H 08H (Word 0x08000175)Khớp 100% với Required của 0x08000000..03!0x0800001075H (75H 01H 00H 08H)79H (79H 01H 00H 08H)Khớp 100% với Required của 0x08000008!0x0800001879H7DHKhớp 100% với Required của 0x08000010!0x08000020..237DH 01H 00H 08H00H 00H 00H 00HKhớp 100% với Required của 0x08000018..1B!0x08000030..3300H 00H 00H 00H81H 01H 00H 08HKhớp 100% với Required của 0x08000028..2B!0x0800003881H83HKhớp 100% với Required của 0x08000030!0x0800004083H87HKhớp 100% với Required của 0x08000038!0x08000100..0300H 0CH 82H 44H01H 07H DAH 45HKhớp 100% với Required của 0x080000F8..FB!0x08000108..0B01H 07H DAH 45H21H F8H AFH F2HKhớp 100% với Required của 0x08000100..03!0x08000110..1321H F8H AFH F2H0FH 00H 13H F0HKhớp 100% với Required của 0x08000108..0B!0x08000148..4B0BH 60H 70H 47H10H B5H 10H BDHKhớp 100% với Required của 0x08000140..43!0x08000150..5310H B5H 10H BDH11H 46H FFH F7HKhớp 100% với Required của 0x08000148..4B!
3. Tại sao các địa chỉ tận cùng bằng 4, 5, 6, 7 và C, D, E, F hoàn toàn VẮNG MẶT trong Log lỗi?

Quan sát kỹ toàn bộ danh sách lỗi từ 0x08000000 đến 0x080001B2: Chỉ có các byte nằm ở Word chẵn của khối 8-byte (địa chỉ kết thúc bằng 0, 1, 2, 3 và 8, 9, A, B) bị báo lỗi mismatch! Không có bất kỳ một địa chỉ nào kết thúc bằng 4, 5, 6, 7 hoặc C, D, E, F bị sai!
Giải thích cơ chế phần cứng (Flash Interface - FLITF):

Kiến trúc bộ nhớ Flash của STM32F103 tổ chức đường đọc vật lý theo từng khối 64-bit (8 byte = 2 Word) mỗi lần truy xuất:

Word đầu tiên nằm ở offset +0 (tận cùng 0..3 hoặc 8..B).
Word thứ hai nằm ở offset +4 (tận cùng 4..7 hoặc C..F).


Khi CPU chạy ở 72 MHz (1 chu kyˋ HCLK≈13.88 ns1\text{ chu kỳ HCLK} \approx 13.88\text{ ns}1 chu kyˋ​ HCLK≈13.88 ns), mảng tế bào nhớ Flash vật lý cần khoảng 35–40 ns35\text{--}40\text{ ns}35–40 ns để kích hoạt dòng đọc và đẩy 64-bit dữ liệu mới ra thanh ghi chốt (Read Latch). Đó là lý do ở 72 MHz72\text{ MHz}72 MHz, phần cứng bắt buộc phải cấu hình 2 Wait States (LATENCY = 2) để bắt Bus AHB đợi thêm 2 chu kỳ HCLK trước khi lấy mẫu dữ liệu.
Khi tệp .FLM của Keil ghi đè FLASH_ACR = 0 (0 Wait State, tắt luôn Prefetch):

Khi trình Verify yêu cầu đọc Word đầu tiên của khối 64-bit mới (ví dụ tại 0x08000008), bộ điều khiển Flash bắt đầu kích hoạt đọc dòng 64-bit 0x08000008..0F.
Nhưng vì Wait State = 0, Bus AHB không chờ 2 chu kỳ mà lập tức lấy mẫu dữ liệu trên thanh ghi chốt ngay ở chu kỳ đầu tiên (13.88 ns13.88\text{ ns}13.88 ns)! Lúc này dữ liệu mới chưa kịp ra tới chốt, nên Bus đọc phải dữ liệu cũ còn nằm trên chốt của lần đọc 64-bit liền trước (0x08000000..07)!
Tới khi trình Verify chuyển sang đọc Word thứ hai của cùng khối 64-bit đó (tại 0x0800000C), thời gian trôi qua giữa 2 lệnh đọc trên Bus đã vượt quá 40 ns40\text{ ns}40 ns, nên toàn bộ 64-bit của khối 0x08000008..0F đã kịp nạp xong vào chốt! Nhờ vậy, việc đọc Word thứ hai (0x0800000C..0F) lấy ra dữ liệu hoàn toàn chính xác!
Ngay khi bước sang địa chỉ 0x08000010 (mở đầu khối 64-bit tiếp theo), chu trình đọc sớm lại tái diễn, và nó lại trả về Word đầu của khối 0x08000008..0B!





C. Dịch ngược STM32F10x_128.FLM và đối chiếu mã nguồn gốc FlashPrg.c
Khi dịch ngược (Disassemble) hàm Init() bên trong tệp thuật toán nạp STM32F10x_128.FLM của KeilC và đối chiếu với mã nguồn mẫu FlashPrg.c của Keil, thủ phạm hiện nguyên hình:
int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {  // Zero Wait State  FLASH->ACR  = 0x00000000;   // <--- THỦ PHẠM: Ép Latency về 0 WS và tắt Prefetch Buffer!
  // Unlock Flash  FLASH->KEYR = FLASH_KEY1;  FLASH->KEYR = FLASH_KEY2;  ...
Tác giả viết tệp FlashPrg.c tại Keil giả định rằng: "Trước khi hàm Init() này chạy, vi điều khiển chắc chắn đã được Reset về xung nhịp mặc định HSI 8 MHz". Ở 8 MHz, việc đặt FLASH->ACR = 0 là hoàn toàn hợp lệ. Tuy nhiên, khi KeilC kết nối qua ST-Link mà không thực hiện Reset phần cứng/ngoại vi thành công trước bước nạp, vi điều khiển vẫn đang chạy ở 72 MHz và thảm họa Contents mismatch xảy ra.
⬆️ Quay về Mục lục


1.4. Sơ đồ luồng nạp của UV4.exe và Giải mã các tùy chọn Connect / Reset
A. Vai trò của UV4.exe và phân tích Log Crash Windows Event Viewer
UV4.exe là tệp thực thi lõi của IDE Keil µVision trên máy tính, đóng vai trò là Tổng chỉ huy (Host Orchestrator): quản lý giao diện, gọi DLL của mạch nạp (ST-LINKIII-KEIL_SWO.dll), thông dịch tệp script .ini, bơm thuật toán .FLM xuống RAM của chip và điều khiển quá trình Verify.
Khi thử nghiệm một tệp .ini có cú pháp/lệnh can thiệp không tương thích (Thí nghiệm #11), Windows Event Viewer ghi nhận sự cố sập ứng dụng UV4.exe:
Faulting application name: UV4.exe, version: 5.43.1.0, time stamp: 0x68a4cd5dFaulting module name: unknown, version: 0.0.0.0, time stamp: 0x00000000Exception code: 0xc000041dFault offset: 0xb57cb6d4Faulting application path: C:\Users\ASUS\AppData\Local\Keil_v5\UV4\UV4.exe

Mã ngoại lệ 0xC000041D (STATUS_FATAL_USER_CALLBACK_EXCEPTION) xảy ra tại giai đoạn UV4.exe gọi hàm callback để thực thi kịch bản trong tệp .ini trước khi nạp Flash.

B. Sơ đồ luồng hoạt động nạp Flash của KeilC (UV4.exe)
  [MÁY TÍNH (UV4.exe - KeilC)]                     [MẠCH NẠP ST-Link]                      [CHIP STM32F103]               |                                           |                                      |               |                                           |     (Chip đang chạy 72 MHz từ trước) |               |                                           |                                      |=========================================== GIAI ĐOẠN 1: KẾT NỐI ====================================================               |                                           |                                      |               | (1) Khởi tạo kết nối & Yêu cầu Debug      |                                      |               |------------------------------------------>|                                      |               |                                           | (2) Gửi tín hiệu SWD                 |               |                                           |------------------------------------->| (Halt CPU)               |                                           |                                      |               | (3) [Script] Chạy flash_init.ini (nếu có) |                                      |               |------------------------------------------>| Mạch nạp ghi thanh ghi RCC           |               |     (Nơi duy nhất có thể dập 72 MHz)      |------------------------------------->| Chip về lại 8 MHz               |                                           |                                      |========================================= GIAI ĐOẠN 2: BƠM THUẬT TOÁN ===============================================               |                                           |                                      |               | (4) Đọc file STM32F10x_128.FLM trên ổ cứng|                                      |               |----------------                           |                                      |               |               | Lấy mã nhị phân của       |                                      |               |<--------------- thuật toán nạp            |                                      |               |                                           |                                      |               | (5) Đẩy toàn bộ mã nhị phân FLM vào RAM   |                                      |               |------------------------------------------>|------------------------------------->| RAM (SRAM)               |                                           |                                      |  [Khối mã FLM]               |                                           |                                      |=================================== GIAI ĐOẠN 3: THỰC THI THUẬT TOÁN TRÊN CHIP ======================================               |                                           |                                      |               | (6) Ra lệnh: "CPU, hãy chạy hàm Init()    |                                      |               |     của khối FLM đang nằm trong RAM!"     |                                      |               |------------------------------------------>|------------------------------------->|                |                                           |                                      | CPU chạy FLM_Init()               |                                           |                                      |  └─> ÉP FLASH_ACR = 0               |                                           |                                      |               | (7) Bơm mã code Firmware của bạn vào RAM  |                                      |               |------------------------------------------>|------------------------------------->| RAM (SRAM)               |                                           |                                      |  [Buffer Firmware]               |                                           |                                      |               | (8) Ra lệnh: "CPU, hãy chạy hàm           |                                      |               |     ProgramPage() của FLM!"               |                                      |               |------------------------------------------>|------------------------------------->| CPU chạy ProgramPage()               |                                           |                                      |  └─> Ghi từ RAM -> Flash               |                                           |                                      |========================================== GIAI ĐOẠN 4: KIỂM TRA & KẾT THÚC =========================================               |                                           |                                      |               | (9) Verify: "CPU, hãy gửi nội dung Flash  |                                      |               |     vừa ghi về cho tao kiểm tra"          |                                      |               |------------------------------------------>|------------------------------------->| CPU đọc lại Flash (ACR=0)               |                                           |                                      |  |               |<------------------------------------------|<-------------------------------------|  |               | [!] NẾU CHIP ĐANG Ở 72 MHz:               |                                      |  |               |     => Lỗi Mismatch! (Trượt 8-byte Latch) |                                      |  |               |                                           |                                      |  |               | (10) Ra lệnh Reset và Chạy (Resume)       |                                      |  |               |------------------------------------------>|------------------------------------->| Chip khởi động lại               v                                           v                                      v
C. Chuỗi 7 sự kiện khi nhấn F8 và Giải mã các tùy chọn Connect / Reset
Tóm tắt chuỗi thứ tự thời gian khi nhấn F8:
(1) Kết nối SWD            <- Tùy chọn [Connect] quyết định có tác động chân NRST ở đây không(2) [Reset after Connect]  <- Tùy chọn [Reset] quyết định kiểu reset nếu ô này được tick(3) Init File (Utilities)  <- Script flash_init.ini chạy ở đây nếu được khai báo(4) FLM Init: ACR = 0      <- Thủ phạm ép 0 Wait State. Luôn chạy sau bước (1)-(3)(5) Erase, Program         <- Xóa và Ghi thành công 100%(6) Verify                 <- Đọc Flash với 0 WS: Nổ lỗi Mismatch nếu chip vẫn còn ở 72 MHz(7) [Reset and Run]        <- Chạy firmware mới sau khi nạp xong

Điều kiện tiên quyết để không bị lỗi: Ngay tại bước (1) -> (3), vi điều khiển bắt buộc phải được đưa về xung nhịp HSI 8 MHz. Nếu lệnh Reset chỉ diễn ra ở bước (7) thì đã quá muộn!

Bảng giải mã tùy chọn Connect (Debug -> Settings):





























Tùy chọn ConnectCơ chế hoạt độngYêu cầu nối chân NRST?Tác dụng với lỗi Contents mismatchNormalKết nối SWD bình thường, không kéo chân Reset phần cứngKhôngKhông giúp được. Giữ nguyên xung 72 MHz của chip, dẫn thẳng tới lỗi.with Pre-resetPhát một xung kéo chân Reset phần cứng xuống thấp trước khi bắt đầu kết nối [Tài liệu]CóChỉ giúp được nếu xung NRST từ ST-Link thực sự truyền tới chân Reset của chip.under ResetGiữ chặt chân Reset phần cứng ở mức thấp trong suốt lúc thiết lập kết nối [Tài liệu]CóGiúp đưa chip về trạng thái sạch và cứu mạch khi bị khóa SWD (nếu chân NRST hoạt động thật).
Bảng giải mã tùy chọn Reset (Debug -> Settings):



































Tùy chọn ResetCơ chế hoạt độngCó Reset ngoại vi (RCC, FLASH_ACR) không?Ghi chú kỹ thuậtAutodetectKeil tự chọn cơ chế dựa theo mô tả trong gói CMSIS-Pack (DFP)Tùy thuộc cấu hình trong PackThường bị khóa hoặc hoạt động không nhất quán với mạch nạp ST-Link.HW RESETKéo chân phần cứng NRST xuống mức thấp (GND)Có (Reset toàn bộ chip)Phụ thuộc hoàn toàn vào dây NRST và khả năng phát xung của mạch ST-Link.SYSRESETREQGhi bit SYSRESETREQ trong thanh ghi AIRCR của lõi Cortex-M3 qua SWDCó (Reset cả lõi CPU và toàn bộ ngoại vi RCC, FLASH)Đáng giá nhất! Reset mềm qua dây SWDIO/SWCLK, không cần dây NRST vật lý.VECTRESETGhi bit VECTRESET trong thanh ghi AIRCR của lõi Cortex-M3KHÔNG! (Chỉ reset mỗi lõi CPU, giữ nguyên ngoại vi)Nguy hiểm cho trường hợp này: Không đưa RCC về 8 MHz và không khôi phục FLASH_ACR.
Bảng các công tắc cấu hình liên quan trong KeilC:



































Công tắcVị trí trong KeilCTác dụng thực tếReset after ConnectDebug -> SettingsNgay sau khi kết nối ở bước (1), thực thi ngay kiểu Reset đã chọn ở bước (2) [Tài liệu].Reset and RunDebug -> Settings -> Flash DownloadSau khi nạp và Verify xong ở bước (6), tự động Reset và cho chip chạy ở bước (7).Pack -> Debug Description: EnableDebug -> Settings -> PackSử dụng chuỗi kịch bản debug định sẵn trong gói DFP (*.dbgconf). Hai báo cáo từ cộng đồng (Arm Forum, EEVblog) xác nhận: Bỏ tick (Disable) ô này giúp ST-Link thực hiện Reset chuẩn xác hơn [Cộng đồng].Init FileUtilities -> Init FileNơi nạp tệp flash_init.ini để chạy các lệnh can thiệp thanh ghi ở bước (3) trước khi nạp Flash.Debug access logDebug -> SettingsXuất nhật ký truy cập SWD ra các tệp STM32F103C8_Sequences_xxxx.log trong thư mục project để kiểm tra Keil có thực sự phát lệnh Reset hay không [Tài liệu].
⬆️ Quay về Mục lục


1.5. Quy trình Kiểm chứng chuẩn và Ma trận Thí nghiệm (Đề xuất từ Claude)
A. Quy trình kiểm chứng chuẩn 6 bước (Đảm bảo tính hợp lệ của phép thử)

Nhấn Rebuild, kiểm tra cửa sổ Build Output báo 0 Error(s) và xác nhận main.c vừa được biên dịch mới.
Thiết lập đúng một biến cấu hình cần thử nghiệm trong hộp thoại Options for Target -> Nhấn OK.
Đóng hoàn toàn phần mềm KeilC, rút cáp USB ST-Link ra khỏi máy tính, cắm lại cáp USB, rồi mở lại KeilC. (Mở lại hộp thoại Settings để kiểm tra chắc chắn cấu hình đã được lưu cứng vào tệp .uvoptx).
Nhấn F8 nạp lần #1 (Đảm bảo báo Verify OK).
Nhấn nút RESET vật lý trên board STM32. Quan sát mắt thường xác nhận LED PC13 nhấp nháy đúng tốc độ nhanh của 72 MHz. (Nếu LED nháy chậm ở tốc độ 8 MHz thì phép thử không hợp lệ, phải dừng lại kiểm tra).
Nhấn F8 nạp lần #2 ngay khi LED đang nháy nhanh và ghi nhận kết quả (Verify OK hay Contents mismatch).

B. Bảng Ma trận Thí nghiệm (B0 -> E8)































































































Mã TestPack: Debug DescriptionTùy chọn ConnectTùy chọn ResetReset after ConnectGiả thuyết kiểm chứngKết quả F8 lần #2B0Enable (Mặc định)NormalAutodetectTắtBaseline gốc: Chip giữ nguyên 72 MHz khi .FLM chạyMismatch [Đã đo]E1DisableGiữ như B0Giữ như B0Giữ như B0Kiểm tra báo cáo cộng đồng về việc tắt Debug Description giúp ST-Link tự reset[Chưa thử]E2EnableNormalSYSRESETREQBậtDùng Reset mềm qua SWD để xóa sạch RCC và FLASH_ACR về mặc định mà không cần dây NRST[Chưa thử]E3Enablewith Pre-resetSYSRESETREQBậtBổ sung xung Reset phần cứng trước khi kết nối kết hợp Reset mềm[Chưa thử]E4Enableunder ResetSYSRESETREQBậtGiữ Reset phần cứng trong lúc kết nối kết hợp SYSRESETREQ[Chưa thử]E5Enableunder ResetHW RESETBậtPhụ thuộc hoàn toàn vào xung phần cứng trên chân NRSTMismatch [Đã đo] (Do NRST không tới chip)E6Disableunder ResetSYSRESETREQBậtTổ hợp mạnh nhất: Tắt Pack Description + Connect Under Reset + SYSRESETREQ[Chưa thử]E7Tùy ýTùy ýTùy ýTùy ýSử dụng tệp kịch bản flash_init.ini tại tab Utilities để ép chip về HSI 8 MHz ở bước (3)PASS 100% [Đã đo]E8————Bỏ qua trình nạp của Keil, gọi trực tiếp ST-LINK_CLI.exe -c SWD -P "#H" -V -rst qua External ToolPASS 100% [Đã đo]
⬆️ Quay về Mục lục


1.6. Khắc phục bằng flash_init.ini và Phân tích Log Thanh ghi (72 MHz vs 8 MHz)
A. Bảng giải phẫu giá trị các Thanh ghi phần cứng (72 MHz vs 8 MHz)
Trong quá trình đo đạc bằng ST-LINK Utility và trình xuất log của KeilC, giá trị thực tế tại các địa chỉ thanh ghi lõi và ngoại vi đã phơi bày toàn bộ trạng thái bên trong con chip:




































































Tên Thanh ghiĐịa chỉ vật lýGiá trị đo đượcGiải mã chi tiết từng cụm Bit (Theo RM0008)Ý nghĩa chẩn đoánDBGMCU_IDCODE0xE00420000x00006410 (Board 1, 2)0x20036410 (Board 3)• Bit [11:0] DEV_ID = 0x410 (Medium-density F103)• Bit [31:16] REV_ID = 0x0000 (Clone) vs 0x2003 (Chính hãng ST)Cả hai mã chip đều bị lỗi Contents mismatch như nhau [Đã đo].SCB_CPUID0xE000ED000x411FC231• Implementer 0x41 (ARM), PartNo 0xC23 (Cortex-M3), Revision r1p1Chỉ xác nhận lõi vi xử lý là Cortex-M3 tiêu chuẩn [Đã đo].RCC_CFGR(Sau Reset)0x400210040x00000000• SW = 00, SWS = 00: Hệ thống đang chạy bằng xung nội HSI 8 MHz• Toàn bộ bộ chia AHB, APB1, APB2 đều bằng /1Trạng thái an toàn tuyệt đối để nạp Flash (0 Wait State đọc chuẩn 100%) [Đã đo].RCC_CFGR(Khi chạy 72MHz)0x400210040x001D040A• Bit [1:0] SW = 10 (Chọn PLL làm SYSCLK)• Bit [3:2] SWS = 10 (PLL đang là SYSCLK)• Bit [10:8] PPRE1 = 100 (APB1 chia 2 = 36 MHz)• Bit [16] PLLSRC = 1 (Nguồn PLL từ HSE)• Bit [21:18] PLLMUL = 0111 (PLL nhân 9 = 72 MHz)Khi chọn Connect Under Reset trên ST-LINK Utility mà vẫn đọc ra 0x001D040A, chứng tỏ xung Reset phần cứng không hề tác động được vào chip [Đã đo].FLASH_ACR(Sau Reset sạch)0x400220000x00000030• Bit [5] PRFTBS = 1 (Prefetch Buffer đang bật)• Bit [4] PRFTBE = 1 (Cho phép Prefetch Buffer)• Bit [2:0] LATENCY = 000 (0 Wait State)Giá trị mặc định chuẩn của nhà sản xuất ngay sau khi Reset phần cứng [Đã đo].FLASH_ACR(Chạy 72MHz chuẩn)0x400220000x00000032• Bit [5:4] PRFTBS = 1, PRFTBE = 1 (Prefetch bật)• Bit [2:0] LATENCY = 010 (2 Wait States)Trạng thái chuẩn khi main.c chạy sau một cú Reset sạch (`0x30FLASH_ACR(Bị .FLM ghi đè)0x400220000x00000000• PRFTBE = 0 (Tắt Prefetch Buffer)• LATENCY = 000 (0 Wait State)Thủ phạm gây lỗi Verify! Hàm Init() của .FLM xóa sạch cả Prefetch lẫn Latency về 0 khi RCC_CFGR vẫn đang là 0x001D040A (72 MHz) [Đã đo].FLASH_ACR(Di chứng sau nạp)0x400220000x00000002• PRFTBE = 0 (Prefetch Buffer vẫn bị tắt!)• LATENCY = 010 (2 Wait States do main.c bật lại)Bằng chứng cho thấy bước Reset and Run của Keil chỉ reset lõi (kiểu VECTRESET) chứ không reset ngoại vi, khiến bit PRFTBE bị .FLM tắt trước đó không được khôi phục về 1 [Đã đo].
B. Giải pháp can thiệp bằng tệp flash_init.ini
Để triệt tiêu lỗi Contents mismatch ngay trên nút F8 của KeilC mà không cần phụ thuộc vào dây NRST, ta gắn tệp flash_init.ini vào mục Options for Target -> Utilities -> Init File. Kịch bản này chạy ở Bước (3) (ngay trước khi .FLM chạy hàm Init()), trực tiếp ghi vào các thanh ghi RCC và FLASH qua giao thức SWD để ép chip lùi từ 72 MHz về HSI 8 MHz:
// Nội dung chuẩn cho file flash_init.ini (Đặt tại tab Utilities -> Init File)FUNC void ResetClocks (void) {    // 1. Mở sẵn 2 Wait States và bật Prefetch Buffer trên FLASH_ACR (0x40022000 = 0x32)    _WDWORD(0x40022000, 0x00000032);
    // 2. Bật dao động nội HSI (Bit 0 HSION = 1 trong thanh ghi RCC_CR tại 0x40021000)    _WDWORD(0x40021000, _RDWORD(0x40021000) | 0x00000001);
    // 3. Chuyển nguồn SYSCLK về lại HSI (Xóa các bit SW trong thanh ghi RCC_CFGR tại 0x40021004)    _WDWORD(0x40021004, 0x00000000);
    // 4. Tắt PLL và HSE (Xóa bit PLLON [24] và HSEON [16] trong RCC_CR, giữ lại HSION)    _WDWORD(0x40021000, 0x00000083);
    // 5. Ghi lại RCC_CFGR = 0 một lần nữa sau khi PLL đã tắt hẳn (vì các bit cấu hình PLL chỉ cho phép xóa khi PLLON = 0)    _WDWORD(0x40021004, 0x00000000);}
// Gọi thực thi hàm ngay trước khi nạp FlashResetClocks();
⬆️ Quay về Mục lục


1.7. Tổng hợp Dẫn chứng từ Datasheet, Reference Manual, Blog và Cộng đồng
A. Rà soát tài liệu Errata Silicon của STM32F103x8/B (DocID14574 Rev 13)






























Mục ErrataMô tả lỗi Silicon của hãng STLiên hệ với lỗi Contents mismatch của dự ánMục 2.2Lỗi đọc bộ nhớ Flash ngay sau khi thoát lệnh ngủ WFI/WFE nếu Prefetch bật, đang ở 2 WS và xung FLITF bị tắt trong chế độ SleepKhông liên quan (main.c không dùng chế độ ngủ WFI/WFE).Mục 2.3Thanh ghi DBGMCU_IDCODE và DBGMCU_CR chỉ đọc được khi có Debugger kết nối; nếu đọc từ code người dùng (User mode) sẽ trả về 0x00000000Giải thích tại sao chỉ kiểm tra mã chip 0x20036410 qua Debugger/ST-LINK Utility.Mục 2.10Các bản Revision cũ (Rev Z và Rev B) không hỗ trợ một số chuỗi lệnh ghép của trình biên dịch mới (IAR ≥5.20\ge 5.20≥5.20, GCC ≥4.2.3\ge 4.2.3≥4.2.3). Rev Y và Rev 1/2/3 không bịĐây là nguồn gốc cảnh báo trên phần mềm IAR, không ảnh hưởng tới ARMCC của KeilC.Mục 2.12Cờ BSY (Busy) trong thanh ghi FLASH_SR bật lên trễ 1 chu kỳ CPU sau khi gạt bit STRT để xóa FlashChỉ liên quan tới code xóa Flash nội bộ, không phải nguyên nhân trượt byte ở bước Verify.
B. Tổng hợp các báo cáo tương đồng từ Cộng đồng Kỹ sư Quốc tế



































Nguồn báo cáoHiện tượng được ghi nhậnSự trùng khớp với kết quả điều tra của chúng taST Community (2010)(IAR + ST-Link)Một bo mạch đọc ra toàn số 0 hoặc dữ liệu sai từ Flash khi chạy ở 72 MHz với 0 Wait State, trong khi cùng code đó nạp lên bo mạch khác lại "tạm chạy được"Khớp 100%: Đọc Flash thiếu Wait State ở 72 MHz gây sai lệch dữ liệu trên đường đọc, và ngưỡng chịu đựng vật lý có dung sai nhỏ giữa từng viên silicon.ST Community (2010)(SYSCLK trên F101)Hạ FLASH_ACR về 0 Wait State khi xung nhịp đang ở mức cao luôn làm vi điều khiển crash ngay lập tức, kể cả khi đã chia nhỏ HCLKKhẳng định độ trễ đọc Flash phải luôn bám sát tần số xung nhịp thực tế.CSDN Blog (2022)(Phân tích FlashPrg.c)Trích dẫn nguyên văn mã nguồn thuật toán nạp .FLM của Keil với dòng lệnh FLASH->ACR = 0; // Zero Wait State trong hàm Init()Xác nhận kết quả dịch ngược tệp STM32F10x_128.FLM của chúng ta là hoàn toàn chính xác.Arm Community & EEVblogMạch nạp ST-Link V2 trên Keil MDK 5.28/5.31 không phát xung Reset chuẩn xác khi bật Pack Debug DescriptionGiải thích tại sao cấu hình mặc định của KeilC không đưa được chip về HSI 8 MHz trước khi gọi .FLM.RIOT-OS Git(cpu/stm32f1/cpu.c)Ghi chú trong mã nguồn hệ điều hành nhúng: MCU dòng F1 sẽ treo vĩnh viễn nếu bật HSE mà thạch anh ngoài không dao độngCơ sở để bổ sung biến đếm Timeout vào hàm SystemClock_Config().
C. Danh mục Đường dẫn Tham khảo Chính thức (Links)

Keil ULINKpro — Connect & Reset Options: https://www.keil.com/support/man/docs/ulinkpro/ulinkpro_ctx_debug.asp
Keil J-Link — Debug (Connect, Reset): https://www.keil.com/support/man/docs/jlink/jlink_conf_debug.asp
IAR ST-LINK — Setup (Reset types): https://docs.iar.com/ewarm/9.7/en/c-spy-debugging/debugger-options/reference-information-on-c-spy-hardware-debugger-driver-options/st-link---setup.html
CMSIS-Pack — Use of debug descriptions in µVision: https://arm-software.github.io/CMSIS_5/5.7.0/Pack/html/dbg_sqns_uvision.html
Arm Community — MDK 5.28 problem with Reset and Run: https://community.arm.com/support-forums/f/keil-forum/44063/mdk-arm-5-28-problem-with-reset-and-run
EEVblog — ST-Link V2 không reset target sau khi nạp: https://eevblog.com/forum/microcontrollers/cant-get-stm-link-v2-debuggerprogrammer-to-reset-the-target-after-programming
Arm Community — PDSC Sequence Execution failed: https://community.arm.com/support-forums/f/keil-forum/47144/pdsc-sequence-execution-failed-error/167512
ST Errata Sheet STM32F101/102/103x8/B (DocID14574 Rev 13): https://git.ondrovo.com/gex/gex-platform-info/raw/branch/master/F103_BLUEPILL/stm32f103%20errata.pdf
ST Community — SYSCLK trên STM32F101 (0 WS ở 72 MHz crash): https://community.st.com/s/question/0D50X00009XkbE7/sysclk-on-stm32f101
ST Community — ST-Link + IAR debugger: https://community.st.com/t5/stm32-mcus-products/st-link-iar-debugger/td-p/567522
CSDN Blog — Trích dẫn FlashPrg.c của Keil (Init đặt ACR = 0): https://blog.csdn.net/sinat_31039061/article/details/128350295
RIOT-OS — cpu/stm32f1/cpu.c: https://git.riot-os.org/RIOT-OS/RIOT/blame/commit/3711a6d406e49a816a75492bf1e6285cf6b8590c/cpu/stm32f1/cpu.c

⬆️ Quay về Mục lục


PHẦN 2: THIẾT LẬP PROJECT TRÊN STM32CUBEIDE (THƯ VIỆN LL)
Để kiểm chứng xem hệ sinh thái chính hãng của STMicroelectronics xử lý bài toán nạp ở 72 MHz như thế nào, chúng ta thiết lập dự án song song LED_PC13_LL_CUBEIDE trên STM32CubeIDE với cùng logic code LL.

2.1. Khởi tạo Project và chọn vi điều khiển

Mở phần mềm STM32CubeIDE, trên thanh menu chọn File -> New -> STM32 Project.
Tại cửa sổ Target Selection, gõ vào ô tìm kiếm Part Number:

STM32F103C8

Chọn dòng STM32F103C8Tx trong danh sách kết quả bên phải và bấm Next.
Tại cửa sổ Project Setup:

Nhập tên dự án vào ô Project Name:



LED_PC13_LL_CUBEIDE

Bỏ chọn dấu tick ở ô Use default location, bấm Browse... và chọn đường dẫn lưu trực tiếp ngoài màn hình Desktop (nằm kế bên thư mục LED_PC13_LL của KeilC):

C:\Users\ASUS\Desktop\LED_PC13_LL_CUBEIDE

Giữ nguyên mục Targeted Project Type là Executable và nhấn Finish.

⬆️ Quay về Mục lục


2.2. Cấu hình chân Pinout và nguồn xung RCC
Trong giao diện đồ họa của tệp cấu hình LED_PC13_LL_CUBEIDE.ioc (tab Pinout & Configuration):

Cấu hình chân điều khiển LED (PC13):

Nhấp chuột trái vào chân PC13-TAMPER-RTC trên hình vẽ vi điều khiển, chọn chức năng:



GPIO_Output

Cấu hình kích hoạt thạch anh ngoài (HSE):

Ở danh sách ngoại vi bên trái, mở rộng mục System Core -> Chọn RCC.
Tại mục High Speed Clock (HSE), đổi từ Disable sang cờ:



Crystal/Ceramic Resonator
(Hai chân thạch anh PD0-OSC_IN và PD1-OSC_OUT trên sơ đồ chip sẽ chuyển sang màu xanh lá).
⬆️ Quay về Mục lục


2.3. Cấu hình sơ đồ xung nhịp Clock Configuration (72 MHz)
Chuyển sang tab Clock Configuration và thực hiện cấu hình theo đúng trình tự sau:

Input frequency (HSE): Đảm bảo ô tần số thạch anh đầu vào (nằm bên trái khối HSE) mang giá trị:

8

PLL Source Mux: Tích chọn nút radio chuyển từ HSI / 2 sang nhánh:

HSE

System Clock Mux: Tích chọn nút radio chuyển từ HSI sang nhánh:

PLLCLK

HCLK (MHz): Nhấp vào ô HCLK (MHz) (có ghi chú 72 MHz max bên dưới), nhập tần số tối đa và nhấn phím Enter:

72

STM32CubeIDE sẽ tự động giải bài toán chia tần và khóa các thông số chuẩn:

Bộ nhân PLLMul:



X 9

Bộ chia AHB Prescaler:

/ 1

Bộ chia APB1 Prescaler (Giới hạn tối đa 36 MHz36\text{ MHz}36 MHz):

/ 2

Bộ chia APB2 Prescaler (Tối đa 72 MHz72\text{ MHz}72 MHz):

/ 1
⬆️ Quay về Mục lục


2.4. Chuyển đổi Driver từ HAL sang LL

Chuyển sang tab Project Manager -> Chọn mục Advanced Settings ở danh sách bên trái.
Trong khung Driver Selector, chuyển đổi cột Driver của cả RCC và GPIO từ HAL sang:

LL

Nhấn tổ hợp phím Ctrl + S để lưu tệp .ioc và chọn Yes để IDE tự động sinh mã nguồn (Generate Code).

⬆️ Quay về Mục lục


2.5. Xử lý lỗi mất kết nối SWD (Connection timed out)
A. Triệu chứng lỗi khi nạp lần 2 trên STM32CubeIDE
Ở lần bấm Run đầu tiên, STM32CubeIDE nạp thành công (Download verified successfully). Tuy nhiên, ngay ở lần bấm nạp thứ 2, cửa sổ Console và hộp thoại báo lỗi liên tục xuất hiện:
Target is not responding, retrying...Target is not responding, retrying...Shutting down...
Error in final launch sequence:Failed to execute MI command:target remote localhost:61234Error message from debugger back end:localhost:61234: Connection timed out.
B. Nguyên nhân gốc rễ và Cách khắc phục
Khi cấu hình .ioc mà quên bật Serial Wire trong mục System Core -> SYS -> Debug, công cụ CubeMX sẽ tự động sinh ra dòng lệnh tắt sạch cả cổng JTAG lẫn cổng nạp SWD ngay đầu hàm main():
/** DISABLE: JTAG-DP Disabled and SW-DP Disabled */LL_GPIO_AF_DisableRemap_SWJ();
Ngay khi vi điều khiển vừa chạy dòng lệnh này ở lần nạp đầu tiên, hai chân SWDIO (PA13) và SWCLK (PA14) bị ngắt khỏi bộ Debug Access Port, làm mạch nạp ST-Link bị mất liên lạc hoàn toàn ở các lần nạp sau.
Cách sửa code và cứu mạch:

Trong tệp Core/Src/main.c, xóa bỏ hàm LL_GPIO_AF_DisableRemap_SWJ(); hoặc đổi thành hàm chỉ tắt JTAG nhưng giữ nguyên cổng SWD:

LL_GPIO_AF_Remap_SWJ_NOJTAG();

(Khuyến nghị) Trong tệp .ioc, vào System Core -> SYS -> mục Debug, chọn cấu hình:

Serial Wire

Thao tác cứu mạch đang bị khóa SWD: Rút cáp USB ST-Link, gạt Jumper BOOT0 trên board từ 0 sang 1, cắm lại cáp USB (để chip vào chế độ System Bootloader, không chạy dòng code khóa SWD trong Flash), sau đó nhấn Run trên CubeIDE để nạp đè bản code đã sửa. Cuối cùng trả Jumper BOOT0 về 0.

Kết quả kiểm chứng với bài toán 72 MHz:
Sau khi giữ lại cổng SWD, tiến hành lặp lại kịch bản gây lỗi của KeilC (Nạp lần 1 -> Bấm Reset vật lý -> Sửa thông số vòng lặp Delay -> Nạp lần 2, lần 3, lần 4...). STM32CubeIDE nạp và Verify thành công 100% ở mọi lần nạp, hoàn toàn không xảy ra lỗi Contents mismatch! Lý do là vì ST-LINK GDB Server gọi công cụ STM32CubeProgrammer bên dưới để quản lý tiến trình Reset & Halt phần cứng chuẩn xác, không dùng tệp .FLM ép FLASH_ACR = 0 một cách mù quáng như KeilC.
⬆️ Quay về Mục lục


PHẦN 3: CẤU HÌNH FLASH BẰNG EXTERNAL TOOLS (PHÂN TÁCH 2 CHẾ ĐỘ NẠP)

3.1. Chế độ A: Nạp xong chạy ngay (Flash & Run)
Khi muốn nạp code vào Flash và cho vi điều khiển tự động chạy ngay lập tức (giống ST-LINK Utility):

Thao tác: Nhấn nút Run (biểu tượng hình tròn màu xanh lá có hình tam giác trắng) trên thanh công cụ.
Các thông số trong Run Configurations -> STM32 C/C++ Application:

Tab Main -> C/C++ Application:



Debug/LED_PC13_LL_CUBEIDE.elf

Tab Debugger -> Debug probe:

ST-LINK (ST-LINK GDB server)

Tab Debugger -> Reset behaviour (Type):

Connect under reset

Tab Debugger -> Misc: Tích chọn cờ:

Verify flash download

Tab Startup -> Runtime Options: Ô Resume được khóa cứng ở trạng thái bật, giúp chip chạy ngay sau khi nạp.

⬆️ Quay về Mục lục


3.2. Chế độ B: Nạp xong chờ bấm Reset vật lý (External Tools)
Do nút Run của STM32CubeIDE bị khóa cứng cờ Resume (luôn tự chạy code ngay sau khi nạp), để thiết lập chế độ "Chỉ nạp code và Verify, sau đó giữ chip đứng im chờ người dùng tự tay bấm nút RESET vật lý trên board" (tương đương phím F8 khi bỏ tick Reset and Run bên KeilC), chúng ta tích hợp công cụ dòng lệnh STM32_Programmer_CLI.exe vào mục External Tools.
A. Các bước thiết lập chi tiết

Trên thanh công cụ của STM32CubeIDE, bấm vào mũi tên nhỏ bên cạnh biểu tượng External Tools (hình chiếc vali màu xanh lá có hình tam giác Play) -> Chọn:

External Tools Configurations...

Nhấp đúp chuột vào mục Program ở cột bên trái để tạo một cấu hình mới.
Tại ô Name, đặt tên định danh:

Flash_Only_Wait_Reset

Tại ô Location, bấm Browse File System... và trỏ đến tệp thực thi STM32_Programmer_CLI.exe nằm trong thư mục cài đặt plugin của STM32CubeIDE:

C:\ST\STM32CubeIDE_1.x.x\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.win32_...\tools\bin\STM32_Programmer_CLI.exe

Tại ô Working Directory, điền biến thư mục không gian làm việc:

${workspace_loc}

Tại ô Arguments, điền chuỗi cờ lệnh sử dụng đường dẫn tuyệt đối trỏ thẳng vào tệp .elf của dự án:

-c port=SWD -d "C:\Users\ASUS\Desktop\LED_PC13_LL_CUBEIDE\Debug\LED_PC13_LL_CUBEIDE.elf" -v

Lưu ý khắc phục lỗi Variable references empty selection: ${project_loc}:
Nếu sử dụng biến động -d "${project_loc}\Debug\${project_name}.elf", khi con trỏ chuột của bạn đang đặt ở cửa sổ Console hoặc ngoài vùng trống (không click chọn tên Project trong cây thư mục Project Explorer), Eclipse sẽ không phân giải được biến ${project_loc} và văng hộp thoại báo lỗi Variable references empty selection: ${project_loc}. Việc thay bằng đường dẫn tuyệt đối "C:\Users\ASUS\Desktop\LED_PC13_LL_CUBEIDE\Debug\LED_PC13_LL_CUBEIDE.elf" giúp nút bấm hoạt động ổn định 100% ở mọi vị trí đặt chuột!

B. Giải thích ý nghĩa từng cờ lệnh trong STM32_Programmer_CLI

Cờ thiết lập kết nối với mạch nạp ST-Link qua giao thức chuẩn SWD:

-c port=SWD

Cờ yêu cầu xóa và tải (Download) tệp thực thi .elf (hoặc .hex/.bin) xuống bộ nhớ Flash:

-d "C:\Users\ASUS\Desktop\LED_PC13_LL_CUBEIDE\Debug\LED_PC13_LL_CUBEIDE.elf"

Cờ yêu cầu đọc lại toàn bộ nội dung vừa nạp để đối chiếu xác minh (Verify):

-v

Cơ chế giữ chip đứng im chờ nhấn Reset vật lý: Do trong dòng lệnh trên chúng ta chủ đích lược bỏ cờ -rst (Reset chip) và cờ -run (Chạy CPU), nên ngay sau khi STM32_Programmer_CLI.exe in ra thông báo Download verified successfully, lõi Cortex-M3 vẫn giữ nguyên trạng thái dừng (Halted). Cả chip Thật lẫn chip Clone đều nằm im chờ đến khi bạn bấm nút RESET vật lý trên board mới bắt đầu chớp LED!

⬆️ Quay về Mục lục


PHẦN 4: GHI CHÚ ĐẶC BIỆT VỀ JUMPER BOOT0 TRÊN STM32CUBEIDE

4.1. Hiện tượng thực tế và giả thuyết ban đầu

Hiện tượng quan sát được: Trong quá trình gạt Jumper BOOT0 lên mức 1 để cứu mạch bị khóa cổng SWD, ngay khi bấm nút Run nội bộ của STM32CubeIDE (lúc này tay chưa kịp gạt Jumper BOOT0 trả về 0), phần mềm không những nạp code thành công mà chương trình trên chip còn tự động chạy ngay lập tức (LED PC13 nhấp nháy ngay cả khi BOOT0 vẫn đang cắm ở mức 1).
Ghi chú suy nghĩ cá nhân (Giả thuyết ban đầu):

"Điểm đặc biệt của CUBEIDE (internal) là mặc dù đang để JUMPER BOOT0 lên 1, chưa kịp gạt về 0 thì chương trình đã chạy luôn rồi. Chỗ này theo tôi nghĩ là do thanh ghi LL quá mạnh, can thiệp sâu vào lõi Cortex-M3 nên nó mới như vậy, chưa kiểm chứng nên chưa biết."



⬆️ Quay về Mục lục


4.2. Giải thích cơ chế can thiệp lõi của SWD và GDB Server
Dưới góc độ kiến trúc phần cứng ARM Cortex-M3, hiện tượng chương trình chạy ngay khi BOOT0 = 1 là kết quả của việc ST-LINK GDB Server cưỡng ép thanh ghi lõi thông qua cổng SWD, kết hợp với mã khởi tạo của bạn:

Vai trò thực sự của chân BOOT0: Chân BOOT0 chỉ được phần cứng lấy mẫu để quyết định xem vùng nhớ nào được ánh xạ (alias) vào địa chỉ khởi động 0x00000000 khi có sự kiện Reset. Khi BOOT0 = 1 và BOOT1 = 0, vùng nhớ System Memory (0x1FFFF000 - chứa Bootloader nạp qua UART của hãng ST) được ánh xạ vào 0x00000000, nên nếu tự khởi động, CPU sẽ chạy Bootloader chứ không nhảy vào Flash chính (0x08000000).
Cách GDB Server vượt qua BOOT0 = 1 khi bấm nút Run:

Khi bạn nhấn nút Run trên STM32CubeIDE, ST-LINK GDB Server nạp tệp LED_PC13_LL_CUBEIDE.elf thẳng vào địa chỉ tuyệt đối của bộ nhớ Flash chính là 0x08000000.
Tiếp theo, do tệp .elf chứa đầy đủ thông tin điểm nhập (Entry Point = địa chỉ hàm Reset_Handler tại 0x0800xxxx) và bảng Vector Table, GDB Server thông qua cổng Debug SWD ghi đè trực tiếp địa chỉ 0x0800xxxx vào thanh ghi bộ đếm chương trình PC (Program Counter - R15) và nạp giá trị đỉnh ngăn xếp vào thanh ghi MSP (R13) của lõi Cortex-M3.
Ngay khi GDB Server phát lệnh Resume, lõi Cortex-M3 bắt đầu thực thi lệnh tại địa chỉ mà thanh ghi PC đang trỏ tới (0x0800xxxx trong Flash chính), hoàn toàn bỏ qua việc vùng nhớ 0x00000000 đang bị chân BOOT0 = 1 ánh xạ đi đâu!


Lưu ý bắt buộc: Dù GDB Server có thể cưỡng ép thanh ghi PC nhảy vào 0x0800xxxx ngay lúc nạp, bạn vẫn bắt buộc phải rút Jumper BOOT0 trả về mức 0. Bởi vì nếu để nguyên BOOT0 = 1, ngay khi bạn rút cáp USB ra cấp nguồn lại hoặc nhấn nút RESET vật lý trên board (không có GDB Server can thiệp), lõi Cortex-M3 sẽ đọc lại địa chỉ 0x00000000 (lúc này là System Bootloader) và mạch sẽ đứng im không chạy chương trình của bạn nữa.

⬆️ Quay về Mục lục


PHẦN 5: TOÀN BỘ SOURCE CODE main.c (KEILC & STM32CUBEIDE)

5.1. Bản main.c cải tiến có Timeout an toàn cho KeilC
Đây là phiên bản nâng cấp của tệp main.c bên KeilC, giữ nguyên cấu trúc LL 72 MHz nhưng bổ sung thêm cơ chế WAIT_MAX (Timeout chống treo vĩnh viễn nếu thạch anh ngoài HSE bị lỏng/hỏng), cập nhật biến SystemCoreClock, và chuẩn hóa thông số trễ Reg_Delay_Spin(900000):
/* USER CODE BEGIN Header *//**  ******************************************************************************  * @file           : main.c  * @brief          : CẤU HÌNH STM32F103 LÊN 72MHZ CHUẨN LL TRÊN KEILC  ******************************************************************************  *//* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/#include "stm32f1xx.h"        #include "stm32f1xx_ll_bus.h"   #include "stm32f1xx_ll_gpio.h"  #include "stm32f1xx_ll_rcc.h"   #include "stm32f1xx_ll_system.h"#include "stm32f1xx_ll_utils.h"
/* Private define ------------------------------------------------------------*//* USER CODE BEGIN PD */#define WAIT_MAX 500000UL/* USER CODE END PD */
/* Private function prototypes -----------------------------------------------*//* USER CODE BEGIN PFP */void Reg_SystemClock_Config_72MHz(void);void Reg_Delay_Spin(uint32_t count);void Reg_GPIO_Init_Custom(void);/* USER CODE END PFP */
int main(void){  /* BƯỚC 1: CẤU HÌNH XUNG NHỊP HỆ THỐNG LÊN 72MHZ */  Reg_SystemClock_Config_72MHz(); 
  /* BƯỚC 2: KHỞI TẠO NGOẠI VI GPIO PC13 */  Reg_GPIO_Init_Custom();
  /* VÒNG LẶP VÔ TẬN */  while (1)  {    LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_13);    Reg_Delay_Spin(900000);   }}
void Reg_SystemClock_Config_72MHz(void){    // 1. Mở trước 2 Wait States trên FLASH_ACR cho mức 72MHz    LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);    while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2) {}
    // 2. Bật thạch anh ngoài (HSE) kèm bộ đếm Timeout an toàn    LL_RCC_HSE_Enable();    uint32_t n = WAIT_MAX;    while (LL_RCC_HSE_IsReady() != 1 && --n) {}    if (n == 0) { return; } // Nếu thạch anh hỏng: Ở lại HSI 8MHz an toàn, không treo máy
    // 3. Cấu hình PLL: HSE / 1 * 9 = 72MHz    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_9);    LL_RCC_PLL_Enable();    n = WAIT_MAX;    while (LL_RCC_PLL_IsReady() != 1 && --n) {}    if (n == 0) { return; }
    // 4. Thiết lập bộ chia Bus AHB, APB1 (/2), APB2 (/1) và chuyển SYSCLK sang PLL    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {}
    LL_SetSystemCoreClock(72000000);}
void Reg_GPIO_Init_Custom(void){    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_13, LL_GPIO_MODE_OUTPUT);                 LL_GPIO_SetPinOutputType(GPIOC, LL_GPIO_PIN_13, LL_GPIO_OUTPUT_PUSHPULL);      LL_GPIO_SetPinSpeed(GPIOC, LL_GPIO_PIN_13, LL_GPIO_SPEED_FREQ_MEDIUM); }
void Reg_Delay_Spin(uint32_t count) {    for (volatile uint32_t i = 0; i < count; i++) { __asm("NOP"); }}
⬆️ Quay về Mục lục


5.2. Bản main.c hoàn chỉnh trên STM32CubeIDE
Dưới đây là toàn bộ nội dung tệp Core/Src/main.c chạy hoàn chỉnh trên STM32CubeIDE, tuân thủ 100% các cặp thẻ bảo vệ /* USER CODE BEGIN ... */ và /* USER CODE END ... */ của CubeMX, đã khắc phục lỗi khóa cổng SWD (LL_GPIO_AF_Remap_SWJ_NOJTAG()):
/* USER CODE BEGIN Header *//**  ******************************************************************************  * @file           : main.c  * @brief          : Main program body  ******************************************************************************  * @attention  *  * Copyright (c) 2026 STMicroelectronics.  * All rights reserved.  *  * This software is licensed under terms that can be found in the LICENSE file  * in the root directory of this software component.  * If no LICENSE file comes with this software, it is provided AS-IS.  *  ******************************************************************************  *//* USER CODE END Header *//* Includes ------------------------------------------------------------------*/#include "main.h"
/* Private includes ----------------------------------------------------------*//* USER CODE BEGIN Includes */
/* USER CODE END Includes */
/* Private typedef -----------------------------------------------------------*//* USER CODE BEGIN PTD */
/* USER CODE END PTD */
/* Private define ------------------------------------------------------------*//* USER CODE BEGIN PD */
/* USER CODE END PD */
/* Private macro -------------------------------------------------------------*//* USER CODE BEGIN PM */
/* USER CODE END PM */
/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* USER CODE END PV */
/* Private function prototypes -----------------------------------------------*/void SystemClock_Config(void);static void MX_GPIO_Init(void);/* USER CODE BEGIN PFP */
/* USER CODE END PFP */
/* Private user code ---------------------------------------------------------*//* USER CODE BEGIN 0 */
/* USER CODE END 0 */
/**  * @brief  The application entry point.  * @retval int  */int main(void){
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */
  /* MCU Configuration--------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  /* System interrupt init*/  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
  /* SysTick_IRQn interrupt configuration */  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));
  /** DISABLE: JTAG-DP Disabled and SW-DP Enabled     * Lưu ý: Đổi từ LL_GPIO_AF_DisableRemap_SWJ() sang LL_GPIO_AF_Remap_SWJ_NOJTAG()    * để giữ lại cổng nạp SWD cho những lần nạp tiếp theo.    */  LL_GPIO_AF_Remap_SWJ_NOJTAG();
  /* USER CODE BEGIN Init */
  /* USER CODE END Init */
  /* Configure the system clock */  SystemClock_Config();
  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */
  /* Initialize all configured peripherals */  MX_GPIO_Init();  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */
  /* Infinite loop */  /* USER CODE BEGIN WHILE */  while (1)  {    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */    LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_13);    for (volatile int i = 0; i < 900000; i++);  }  /* USER CODE END 3 */}
/**  * @brief System Clock Configuration  * @retval None  */void SystemClock_Config(void){  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_2)  {  }  LL_RCC_HSE_Enable();
   /* Wait till HSE is ready */  while(LL_RCC_HSE_IsReady() != 1)  {
  }  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_9);  LL_RCC_PLL_Enable();
   /* Wait till PLL is ready */  while(LL_RCC_PLL_IsReady() != 1)  {
  }  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
   /* Wait till System clock is ready */  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)  {
  }  LL_Init1msTick(72000000);  LL_SetSystemCoreClock(72000000);}
/**  * @brief GPIO Initialization Function  * @param None  * @retval None  */static void MX_GPIO_Init(void){  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */
  /* GPIO Ports Clock Enable */  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOD);
  /**/  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_13);
  /**/  GPIO_InitStruct.Pin = LL_GPIO_PIN_13;  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */}
/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
/**  * @brief  This function is executed in case of error occurrence.  * @retval None  */void Error_Handler(void){  /* USER CODE BEGIN Error_Handler_Debug */  /* User can add his own implementation to report the HAL error return state */  __disable_irq();  while (1)  {  }  /* USER CODE END Error_Handler_Debug */}#ifdef USE_FULL_ASSERT/**  * @brief  Reports the name of the source file and the source line number  *         where the assert_param error has occurred.  * @param  file: pointer to the source file name  * @param  line: assert_param error line source number  * @retval None  */void assert_failed(uint8_t *file, uint32_t line){  /* USER CODE BEGIN 6 */  /* User can add his own implementation to report the file name and line number,     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */  /* USER CODE END 6 */}#endif /* USE_FULL_ASSERT */
⬆️ Quay về Mục lục