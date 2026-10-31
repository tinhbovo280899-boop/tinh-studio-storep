# T Studio FX 1.2.2 — bản sửa chức năng DSP

Gói này chứa **toàn bộ mã nguồn, JUCE và script build/tạo bộ cài** cho bốn plugin T-Verb, T-RVerb, T-PANTHEON và T-LIQUID DELAY. Chưa có VST3/EXE/PKG Windows hoặc macOS đã biên dịch native.

Đợt này sửa T-PANTHEON và HD của LIQUID Delay. T-Verb kèm guard NaN đầu vào đã gửi ở đợt audit trước; phần engine T-Verb và T-RVerb giữ nguyên.

## T-PANTHEON đã thay đổi

- Hall, Chamber, Room, Plate, Ambience, Custom nay là **sáu voicing có xử lý khác nhau trên mạng buồng vang đang có**: thay độ khuếch tán đầu vào/trong tank và tỷ lệ phản xạ sớm/đuôi muộn. Chuyển màu có làm mượt 20 ms. Đây là thiết kế của dự án, chưa xác nhận tương đương sáu thuật toán của hãng.
- RT60 dùng suy hao theo độ dài thực của từng đường trễ trong tank, gồm cả allpass. Bỏ cách áp một hệ số tính từ đường dài nhất nhiều lần. Cấu hình mặc định 2,03 s nay đo ở 1 kHz khoảng 2,03 s; số đo theo điều kiện nằm trong Validation.
- Damping là lọc thấp thông ở đường wet, ánh xạ Hz đúng chiều và đúng điểm -3 dB của bộ lọc một cực. Tăng Hz mở phần cao. Đặt lọc màu ngoài hồi tiếp giúp giữ RT60 theo núm; Damping không còn dùng để thay đổi riêng thời gian tắt dải cao như một số thiết kế reverb khác.
- Bass Boost là low shelf ở đường wet, dùng được hết dải 0,1×–2,5×. Nó không đẩy gain vòng hồi tiếp tới sát mất ổn định và không còn vùng 1,2×–2,5× bị phẳng ở cấu hình mặc định.
- Các đầu đọc thay đổi vị trí nối chéo 20 ms; các chỉnh sửa dồn dập được gộp lại để tránh khởi động lại chuyển tiếp giữa chừng. Vang ngắn thu gọn phân bố phản xạ sớm. Reset xóa cả tank và trạng thái lọc màu.

**Âm vang sẽ thay đổi so với bản 1.2.1**, nhất là đuôi dài hơn theo RT60 đã sửa và Damping tối hơn ở cùng một số Hz thấp. Mã/ID/thứ tự tham số giữ nguyên không đồng nghĩa âm thanh sau sửa giống từng mẫu bản cũ. Nên lưu bản plugin cũ nếu cần tái hiện chính xác một bản mix đã hoàn tất.

## Dùng trên FX Send/AUX

Instance mới mặc định bật **FX SEND 100% WET**. Khi bật, đường xử lý hợp lệ là wet-only, đổi cả 37 preset vẫn đặt Mix 100%. Chỉnh lượng vang bằng Send/Return trong Cubase. Knob và ô nhập Mix bị khóa, host automation Mix không đưa dry vào return trong chế độ này.

Để chèn trực tiếp vào vocal và trộn dry/wet, **tắt FX SEND**, rồi chọn preset hoặc chỉnh Mix. Nút này được lưu trong state, không thêm hoặc đổi thứ tự các tham số automation cũ.

Project/preset state cũ không có thuộc tính `fxSendMode` sẽ mở ở **Insert**, giữ giá trị Mix đã lưu. Quy tắc này đã thử bằng state cũ mô phỏng. Tuy nhiên bản DSP được sửa vẫn làm âm vang thay đổi như ghi ở trên.

License vẫn áp dụng. Nếu chưa kích hoạt hoặc trial hết hạn, plugin giữ hành vi trả dry của bộ hiện tại; FX Send không thay thế việc kích hoạt. Xem badge license nếu không có vang.

## LIQUID Delay HD

Nút HD nay chọn bộ lọc state-variable TPT với trạng thái double, có đáp ứng LP/HP thực. Tần số được ánh xạ đúng theo sample rate, kể cả 32/44,1 kHz. Hai tuyến lọc được giữ trạng thái và nối chéo 20 ms khi bật/tắt.

**HD tắt giữ tuyến lọc Classic cũ.** Đã so với nguồn cũ ở 44,1/48/96 kHz, ba thứ tự FX và LFO bật/tắt: 18 ca đều trùng từng mẫu. HD bật có màu/đáp ứng khác; mặc định và preset từng lưu HD=true nay sẽ nghe khác vì nút trước đây bị DSP bỏ qua. Filter cần bật để HD có tác động.

Không thêm oversampling/lookahead hoặc tăng độ trễ báo host ở hai plugin này. Đáp ứng pha của IIR vẫn phụ thuộc tần số; không gọi đây là độ trễ mic–tai nghe bằng 0.

## Build và license của anh

1. Giải nén vào thư mục mới, giữ bản nguồn/plugin cũ nếu cần đối chiếu.
2. Giữ/copy **public key đã dùng cho bộ T Studio của anh** vào `config/public_key.txt`. File trong đầu vào anh gửi đang trống; gói này giữ nguyên nó. Product ID TVOCALPACK, brand TSTUDIO, trial 7 ngày và mã license không đổi. Không sinh key mới hay đính kèm private key.
3. Windows: chạy `1_BUILD_VA_TAO_BO_CAI_WINDOWS.bat`. Cần Visual Studio C++ Desktop Development, CMake 3.22+ và Inno Setup để tạo EXE. `2_CHI_BUILD_WINDOWS.bat` dành cho chỉ build.
4. macOS: chạy `1_BUILD_VA_TAO_BO_CAI_MAC.command`; cần Xcode Command Line Tools và CMake. Script sẵn có tạo PKG trên máy Mac.
5. Đóng Cubase trước khi thay plugin đã biên dịch/cài bản mới. Cùng ID nên project cũ tiếp tục tìm đúng sản phẩm.

SDK và script đi kèm không đồng nghĩa đã kiểm tra compiler/bộ cài trên hai hệ điều hành đó. `Validation/` ghi rõ kết quả đã chạy trên Linux.

## Kiểm chứng và giới hạn

Đã biên dịch nguồn bốn processor/editor trong harness JUCE 8.0.12 trên Linux. 24 chương trình kiểm tra chạy thành công, không skip; gồm 16 test dự án và tám test vòng đời/tín hiệu. Dùng store trial QA riêng, không dùng hay sửa trial/key thật.

- 68 kiểm tra chính sách license đạt; các file `shared/licensing/` và `config/` khớp nguồn đầu vào.
- T-PANTHEON: 27 ca đo RT60 ở 0,5/2,03/5 s hoặc sáu voicing và ba room size đạt; thêm 81 phép đo vang ngắn 0,1/0,2/0,5 s ở 44,1/48/96 kHz, sai lệch lớn nhất khoảng +12,7% tại Room/Size 76 m/0,1 s.
- Hai bộ test DSP mới chạy với ASan/UBSan không báo lỗi; toàn bộ mã DSP header mới được instrument, runtime JUCE dùng chung không instrument và leak detection tắt.
- Bốn plugin qua 24 vòng đời processor/16 editor và 16 lần hủy dưới UI lock; không quan sát C++ new hoặc nonfinite trong các callback đã đo ở 48/96 kHz.
- State FX Send/Insert, state cũ thiếu cờ, 37 preset wet-only, menu/nút FX Send và chống dry leakage khi host đổi Mix đã được thử. Preview là ảnh vẽ thật từ editor QA.

Chưa chạy DLL Windows trong Cubase 15, chưa chạy bộ cài Windows/macOS, chưa thử ASIO/monitor trên máy anh hoặc nghe so vocal thật. Rủi ro worker license đợi I/O tới 30 giây ở báo cáo trước vẫn chưa được tái hiện/sửa trong đợt DSP này. Không khẳng định mọi tình huống đơ DAW đã hết hoặc tiếng tương đương bản hãng.

Các CSV/JSON, log và đồ thị so trước/sau nằm trong `Validation/`. `SOURCE_CHANGES.patch` và manifest ghi thay đổi so với đúng ZIP 1.2.1 anh gửi. Các báo cáo lịch sử giữ trong `docs/` không được tính như phép thử mới.
