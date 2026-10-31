# T-Studio Vocal Pack 1.7.2 Live

Sửa từ ZIP **T-Studio_VocalPack_1.7.1.zip** người dùng gửi. Đây là gói mã nguồn đầy đủ, gồm JUCE và các script build Windows/macOS gốc. Không kèm binary hay asset của Topline/Antares, không giải mã thuật toán hãng.

## Đường xử lý live

- Mốc trễ trung tính đổi từ 5,5 ms sang số mẫu gần nhất với 2 ms: 88/96/192/384 mẫu ở 44,1/48/96/192 kHz. Processor thông báo mốc này với host trong `prepareToPlay`, không thay latency liên tục trong audio callback.
- Giảm biên đọc tối thiểu và cửa sổ dịch xuống từ 1,2 chu kỳ còn một chu kỳ. Phần tìm điểm nối theo chu kỳ và nội suy được giữ, có giới hạn để đầu đọc cũ không chạm vùng chưa ghi khi dịch lên quãng lớn.
- Làm mượt việc bắt/mất cao độ và tỷ lệ đọc để đầu chữ, hơi thở và nguyên âm không đổi tốc độ đọc ngay lập tức. Sau khoảng lặng, bộ đọc trở về mốc live.
- Dry dùng vòng đệm của tín hiệu gốc và cùng mốc trung tính với Wet; không còn lấy mốc 256 mẫu cố định ở mọi sample rate. Master Bypass và host bypass tiếp tục chạy trạng thái, chuyển mềm về Dry có bù trễ. Không thêm hay đổi ID tham số bypass.

**2 ms không phải độ trễ tuyệt đối của mọi nốt/preset.** Khi pitch shifting, việc ghép chu kỳ có thể lấy mẫu cũ hơn mốc danh định; ở giọng trầm khác biệt lớn hơn. Thông báo latency cho host không xóa độ trễ này hay độ trễ ASIO/interface. Mix hai tín hiệu khác cao độ vẫn tạo tương tác pha; sửa ở đây loại lỗi triệt phổ trong trạng thái trung tính, không hứa mọi hỗn hợp Dry/Wet sẽ đồng pha.

## Giữ luyến và ngân

- Hệ số Retune/Humanize, thời gian ngân và LFO dùng tốc độ phân tích thực. Retune 20 ms đo thời gian đáp ứng một nửa gần 20 ms ở cả bốn sample rate, thay vì thành 40/80 ms ở 96/192 kHz.
- Đường cao độ chậm dùng thời gian khoảng 80 ms để tách chuyển động ngân khỏi đường bắt nốt nhanh; Humanize giữ thêm phần chuyển động này khi ngân ổn định. Đặt lại đường tâm nốt khi chuyển nốt để tránh kéo đuôi của nốt trước sang nốt sau.
- Natural Vibrato điều chỉnh phần dao động còn lại sau retune. Trước đây tác động trước bộ làm mượt có thể làm mức giảm vibrato trở thành tăng do lệch pha.
- Giá trị vibrato tổng hợp được giữ và làm mượt giữa các tick phân tích; không còn chỉ tác động vào một mẫu rồi về unity ở các mẫu xen kẽ. Tắt Amount hoặc mất giọng đưa phần biên độ/formant về trung tính.

Preset **Live Smooth** được thêm sau 12 preset cũ: Retune 25 ms, Flex-Tune 15, Humanize 35, Natural Vibrato 0 dB, Mix 100%, Formant tắt, transpose 0. Instance mới dùng bộ thông số này; project cũ vẫn nạp các giá trị đã lưu. Các preset cũ giữ vị trí và bộ giá trị của chúng. UI hiện Custom khi thông số khôi phục không khớp Live Smooth.

## Formant/Throat

Giữ các hàm LPC, nghịch lọc, tổng hợp Throat và các tham số hiện có. Đọc nhánh thường và nhánh residual tại cùng vị trí, chuyển Formant bằng crossfade khoảng 10 ms. Nếu nhánh LPC sinh giá trị không hữu hạn hoặc vượt ngưỡng khuếch đại bất thường so với tín hiệu vào, chỉ hồi phục lịch sử lọc LPC và tạm dùng âm đã tune không Formant; thử hồi phục sau khoảng 50 ms. Không reset toàn bộ bộ dò cao độ khi riêng LPC gặp lỗi.

Đây là bảo vệ lỗi nội bộ, không phải compressor/limiter âm lượng tổng. Với dịch quãng hoặc Throat cực đoan, bảo vệ có thể tạm giảm hiệu ứng Formant. Formant mặc định tắt theo yêu cầu.

Giới hạn T0 cũ 2048 mẫu làm cắt ngắn chu kỳ giọng 80 Hz ở 192 kHz. Giới hạn mới theo dung lượng ring, còn chừa chỗ cho crossfade. Phép đo phổ âm ra kiểm tra riêng lỗi này.

## Tương thích và build

- Giữ nguyên 26 ID tham số và thứ tự; giữ manufacturer/plugin/bundle ID.
- Không sửa mã AutoKey, handoff key/scale, license, public key, product/brand ID hoặc thời hạn trial. Phiên bản pack đổi 1.7.1 → 1.7.2; hai plugin trong pack nhận metadata phiên bản mới khi build.
- Mã DSP đã thay đổi nên âm thanh có thể khác dù project cũ giữ cùng thông số; có thể giữ bộ cài 1.7.1 để đối chiếu.
- Các script Windows/macOS giữ nguyên. Build trên máy tương ứng để có VST3/AU hoặc installer; không có `.exe`, `.pkg` hay VST3 Windows/macOS dựng sẵn trong gói này.

## Kiểm chứng

`QA/1.7.2_LIVE/` chứa kết quả cuối và SHA-256 mã đã kiểm tra:

- 89 trường hợp DSP/tín hiệu/điều khiển ở 44,1/48/96/192 kHz: neutral Dry/Wet, Retune, giọng trầm, dịch ±12 semitone, Formant bật/tắt, NaN đầu vào, vibrato, giới hạn đọc nhân quả.
- DSP standalone chạy AddressSanitizer + UndefinedBehaviorSanitizer. JUCE/runtime ở phép thử processor không được instrument; không gọi đó là kiểm tra sanitizer toàn DAW.
- 32 trường hợp đo phổ của tín hiệu âm ra với 80/220 Hz, dịch ±0,25 và ±12 semitone ở bốn sample rate. Sai số lớn nhất của bộ thử dưới 2 cents.
- 8 vòng đời processor/editor Linux: stereo giống nhau khi đầu vào giống nhau, lưu/khôi phục state, Master/host bypass cùng trễ, metadata latency và khởi tạo trial trong kho QA riêng.
- Không chạy binary VST3 trong Cubase Windows, không thử ASIO/interface của người dùng, không đánh giá nghe A/B vocal. Các phép đo có phạm vi riêng, không chứng minh đã hết mọi click hoặc cảm giác hát nặng trên máy thật.

Tái lập kiểm tra DSP bằng Python 3 + `g++` C++17:

```
python tests/live/run_dsp_checks.py
python tests/live/run_dsp_checks.py AutoTune/Source --sanitize
python tests/live/run_pitch_checks.py
```

Kiểm tra phổ cần NumPy. Script chỉ đọc nguồn và tạo executable/tín hiệu thử trong thư mục test; không sửa license hoặc chạy plugin hãng. Processor/editor đã biên dịch riêng với JUCE 8.0.12 trên Linux, dùng kho trial QA riêng; harness không được đưa vào luồng build giao khách.
