Gói kiểm tra trường hợp cài Waves xong nhưng Cubase không thấy plugin

1. Giải nén toàn bộ ZIP trên máy bị lỗi.
2. Dùng đúng tài khoản Windows thường mở Cubase, chạy Kiem_Tra_Waves_Cubase.bat. Không cần chạy bằng quyền admin.
3. Gửi lại file KetQua_Waves_Cubase_*.txt xuất hiện cạnh BAT.

Công cụ đọc vị trí WaveShell, header PE và SHA-256 của module, danh sách gói .bundle V16, một số file VC++ và các đoạn nhắc Waves trong file quét Cubase tìm thấy. Thư mục người dùng hiện tại được thay bằng %USERPROFILE% trong báo cáo. Công cụ chỉ tạo file TXT kết quả; không nạp plugin và không sửa hệ thống. Không cần Waves Central để chạy kiểm tra.

Đây là công cụ chẩn đoán, chưa phải bản sửa bộ cài. Đọc được header x64 không chứng minh plugin có thể nạp, đủ mọi dependency, hoặc chạy được. Công cụ không đánh giá tình trạng kích hoạt. Không tìm thấy dấu vết blacklist không chứng minh Cubase đã quét thành công.

Về phạm vi gom của BAT tạo bộ cài đã gửi:

- Chỉ mục khớp WaveShell*16*.vst3 trong VST3 được gom, gồm đầy đủ nội dung nếu mục đó là một bundle. Không gom các VST3 khác. Mẫu tên này còn lỏng; cần đối chiếu tên và phiên bản thực tế.
- Cả thư mục Program Files (x86)\Waves và dữ liệu dùng chung Waves được gom, nên có thể kéo theo các bản Waves khác đang cài. Chưa lọc riêng danh sách plugin anh muốn đóng gói.
- Báo biên dịch/cài đặt thành công chưa xác nhận Cubase nạp được plugin trên máy mới.

Đối chiếu trong Cubase 15:

- Mở Studio > VST Plug-in Manager, chọn danh sách tất cả plugin (tránh chỉ xem một Collection tự tạo).
- Xem thẻ Blocklist có mục WaveShell/Waves không. Ghi nhận trước khi thay đổi hay quét lại.
- VST3 x64 dùng đường mặc định C:\Program Files\Common Files\VST3. Thêm Waves\Plug-Ins V16 vào danh sách đường dẫn VST2 không thay thế cho WaveShell VST3.

Tài liệu tham khảo:

- Waves: https://www.waves.com/support/how-to-find-your-plugins-in-cubase
- Steinberg: https://helpcenter.steinberg.de/hc/en-us/articles/115000177084-VST-plug-in-locations-on-Windows

Kiểm chứng ở môi trường phát triển: cú pháp PowerShell, đọc header trên dữ liệu giả, tìm WaveShell phẳng/bundle và lọc trích đoạn log. Không có máy Windows/Cubase hoặc dữ liệu Waves thật để thử cài mới.
