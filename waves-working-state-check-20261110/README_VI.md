Thu thập trạng thái Waves đang chạy được

Mục đích: tìm thành phần mà bộ tự đóng gói có thể chưa cài đủ sau thử nghiệm “cài bộ chính thức → gỡ → cài bộ tự gom thì chạy được”. Không cần thử thêm Cubase để thu thập thông tin này.

1. Trên máy hiện đang chạy được, mở DAW và giữ một plugin Waves như VEQ4 đang mở.
2. Giải nén ZIP công cụ vào một thư mục có quyền ghi, chạy Thu_Thap_Waves_Dang_Chay.bat bằng tài khoản đang mở DAW.
3. Gửi lại duy nhất ZIP KetQua_Waves_*.zip được tạo cạnh BAT.

Báo cáo gồm:

- Module mà các tiến trình có nạp Waves đang dùng: đường dẫn, kiến trúc, phiên bản; kèm hash SHA-256 cho module được nhận diện là Waves. Bắt đầu từ tiến trình đang chạy trong cùng phiên đăng nhập, không cần đoán tên EXE của NT Studio.
- Danh sách file trong các vị trí Waves dưới Program Files, Common Files, ProgramData, Public và AppData; chỉ tìm WaveShell tại các đường VST2/VST3 phổ biến. Giữ danh sách riêng cho cả x86 và x64.
- SHA-256 cho các file native .dll/.exe/.vst3 được kiểm tra. Những dữ liệu khác chỉ ghi tên, kích thước và thời gian, không sao chép nội dung.
- Một số giá trị đường dẫn cài đặt dưới registry Waves của tài khoản hiện tại và máy, ở cả nhánh 32/64-bit. Không xuất cả nhánh registry.
- Metadata một số runtime Microsoft và phiên bản Windows đọc từ registry.

Chỉ tạo thư mục và ZIP báo cáo. Không nạp plugin, không tiêm mã vào DAW, không chép binary plugin vào báo cáo, không cài/gỡ phần mềm, sửa registry, license, cache hay tự tải/gửi dữ liệu lên mạng. Bỏ qua các mục license/device/token/cache/log và file .wle/.lic/.key. Thay đường dẫn hồ sơ người dùng hiện tại bằng %USERPROFILE%.

Nếu không đọc được module do quyền của tiến trình hoặc không phát hiện host có nạp Waves, Summary.json ghi rõ; không kết luận rằng plugin không chạy. Quét tối đa 50.000 file, có cờ báo nếu đạt giới hạn. Đường VST tùy chỉnh ngoài những vị trí phổ biến có thể chỉ xuất hiện khi host đang nạp WaveShell.

Đây là báo cáo sau khi máy chạy được, chưa phải lịch sử đầy đủ trước/sau cài đặt. Các module đã nạp và danh sách file giúp khoanh vùng, chưa xác định được mọi dependency hay bước đăng ký còn thiếu. Việc một host nạp được plugin không xác nhận mọi host khác cũng nạp được.

Kiểm chứng phát triển: PowerShell 7 trên Linux, AST và dữ liệu giả/mô phỏng; không có Windows, DAW hoặc bộ Waves thật để kiểm thử tại môi trường phát triển.
