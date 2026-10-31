# T Studio FX 1.2.3 — kết quả rà thêm và vá reset

[Tải đầy đủ mã nguồn bốn FX 1.2.3](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/TStudio_FX_Bundle_1.2.3_RESET_FIXED_WIN_MAC_SOURCE.zip) — khoảng 18,6 MB. Gồm T-Verb, T-RVerb, T-PANTHEON, T-LIQUID DELAY, JUCE và script build/tạo bộ cài Windows/macOS. Chưa có VST3/EXE/PKG native đã biên dịch; cần copy đúng public key đang dùng vào config/public_key.txt trước khi build. Cơ chế license được giữ như gói 1.2.2.

## Đã sửa sau khi tái hiện

- PANTHEON mất giá trị đầu đọc đang chờ khi reset giữa các lần chỉnh nút nhanh. Trong ca Pre-delay 0 → 50 → 150 ms ở 48 kHz, wet đầu tiên trước sửa ở 50,625 ms, sau sửa và engine mới cùng cài đặt đều ở 150,625 ms. Đây là thời điểm phản xạ đầu tiên, không phải latency thiết bị. Reset cũng đưa hệ số màu lọc đang chuyển về đúng đích hiện tại.
- LIQUID Delay chưa override AudioProcessor::reset(), nên host reset trực tiếp không xóa buffer. Bản này reset cả delay, filters, drive, limiter, LFO, HD blend và meter. Bài thử có đuôi cũ peak 0,175 trước sửa; sau sửa là 0.

## Kiểm chứng

25 chương trình kiểm tra nguồn đạt, không skip; gồm 24 ca reset PANTHEON và 12 ca reset LIQUID trên nhiều sample rate, block 7/128 và các chế độ filter/LFO/HD. Các render sau reset khớp plugin mới từng mẫu. Chính hai bộ bài thử mới báo lỗi khi chạy với nguồn 1.2.2 và đạt với 1.2.3.

Hai bộ kiểm tra reset ASan/UBSan đạt. PANTHEON header DSP và LIQUID processor/harness được instrument; JUCE/GUI/license runtime không instrument, leak detection tắt. Bốn plugin tiếp tục đạt lifecycle/editor/signal, preset/state, RT60/HD filter và 68 kiểm tra license. ID/thứ tự tham số và nguồn licensing/public config giữ nguyên. Log, manifest, patch từ 1.2.2 và báo cáo chi tiết nằm trong ZIP; dữ liệu 1.2.2 giữ riêng như lịch sử.

## Phần còn nên cải thiện

Level của PANTHEON còn đổi gain tức thời: bài thử nhảy 0 → −36 dB tại đỉnh sine biên độ 0,1 có bước mẫu khoảng 0,09836. Mix/Input/Echo cũng nhận giá trị trực tiếp theo mã nguồn; nên thêm ramp nếu muốn kéo/automation thật êm. Bản 1.2.3 tập trung sửa reset, chưa có cải tiến ramp này.

FX Send giữ wet-only khi license cho phép DSP chạy; khi bị khóa hoặc host bypass, return có thể trả dry theo hành vi hiện tại. Với AUX, mute return giúp tắt hiệu ứng mà không cộng thêm dry. Worker license còn timeout dừng 30 giây nếu I/O bị kẹt; chưa tái hiện ca kẹt hoặc chứng minh đây là nguyên nhân đơ Cubase của chủ dự án.

Kiểm tra nguồn trên Linux/JUCE 8.0.12 với store trial QA riêng. Chưa chạy native Cubase 15 Windows, ASIO, Windows/macOS installer hoặc nghe vocal thực tế của đợt này; không kết luận đã hết mọi tình huống đơ DAW.

SHA-256: `b2a3ad13a42fda8d06c08cd1a68751c0e2fe065b998c8b8634b5a28a877e31af`
