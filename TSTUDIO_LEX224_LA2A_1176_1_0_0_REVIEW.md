# Rà và sửa Lex224 / LA2A / 1176 — 31/10/2026

## Kết quả

Archive được gửi có đủ ba dự án nhưng chỉ có core C++ cùng ảnh/JavaScript web GUI và preset, chưa có CMake/wrapper VST3. Bản giao bổ sung ba VST3 độc lập với một cây JUCE, giao diện native kết nối trực tiếp DSP và bộ xuất Windows x64.

| Plugin mới | Tham số core + bypass | Factory preset | Plugin code |
| --- | ---: | ---: | --- |
| T-Lex224-R | 24 + 1 | 55 | Tl2R |
| T-LA2A-TC | 5 + 1 | 29 | Tl2C |
| T-1176-FET | 7 + 1 | 26 | Tf7C |

Manufacturer code `Tstd`. Bundle ID ở `BUNDLE_INFO.json`. Các ID mới không thay thế các VST cũ. Tổng cộng giữ 110 factory preset và thứ tự ban đầu.

## Những lỗi tái hiện được

| Bài thử | Trước sửa | Sau sửa |
| --- | --- | --- |
| Lex224 Pre-delay 0, impulse vào buffer, đo năng lượng 512 sample đầu | 0, do đọc cả vòng buffer | 0.0000586515292869 |
| Lex224 reset rồi render lại, modulation/noise bật | Sai khác đỉnh 0.0121234270914 | Sai khác 0 |
| LA2A cùng tín hiệu 0,2 s rồi hồi 0,8 s ở 48/96 kHz | GR khác 0.410303773 dB | Khác 0.000015892 dB |
| LA2A: sin liên tục kết thúc đúng mẫu 0 | VU tụt về -73.597862702 dB | VU -23.183919052 dB |
| 1176: cùng phép thử meter | VU -120 dB | VU -23.186554582 dB |

Baseline chạy năm ca tái hiện lỗi nên exit 1/5 failures là kết quả mong đợi. Log sau sửa dùng bộ regression mở rộng.

Pre-delay có lỗi lệch con trỏ: `write()` đã tăng write index nhưng đầu đọc không trừ lại một sample. Zero delay trỏ vào vùng cũ của cả vòng buffer, khoảng 351 ms tại 48 kHz, thay vì mẫu vừa ghi. Bản sửa định nghĩa rõ zero/integer/fractional delay, kiểm tra wrap và dữ liệu delay không hữu hạn.

LA2A trước đó dùng hằng số tăng/giảm bộ nhớ theo từng sample nên thay đổi hành vi khi đổi sample rate. Các hằng số mới giữ tương đương tại 48 kHz. Curves gain, threshold, tube/FET và mô hình compressor cơ bản của nguồn được giữ lại.

## Đo thời gian vang Lex224

Code cũ tính decay gain từ 85 ms ước lượng. Vòng tank thực gồm delay và allpass dài hơn nhiều. Bản sửa dùng độ dài trung bình thực của mỗi nhánh để tính hệ số, bỏ ngưỡng dưới gain vốn làm hạn chế thời gian ngắn; vẫn giữ cùng mạng tank/voicing.

Đặt Bass và Mid cùng **3,0 s**, 48 kHz, treble cao nhất, noise/decay optimization tắt, wet-only; đo octave 1 kHz bằng tích phân năng lượng ngược và fit -5..-25 dB, ngoại suy RT60:

| Voicing | Trước sửa | Sau sửa |
| --- | ---: | ---: |
| Small Hall B | 8,251943 s | 3,137764 s |
| Vocal Plate | 8,750884 s | 3,152388 s |
| Large Hall B | 7,352215 s | 3,044149 s |

Hai bộ trước/sau có 9 render stereo impulse mỗi bộ: ba voicing × mục tiêu 1/3/6 s; mỗi render đo 125/1000/4000 Hz, tổng 54 số đo. Dải 1 kHz sau sửa có sai lệch lớn nhất khoảng **24,95%** ở Large Hall B đặt 1 s, do tail allpass/đường suy giảm không hoàn toàn một độ dốc; R² ca đó khoảng 0,952. Dải cao vẫn suy giảm nhanh hơn do damping. Chưa đo đầy đủ mọi program, sample rate và cực trị decay 0,25–70 s bằng phép đo RT60 này.

Đây là hiệu chỉnh tính nhất quán của control trong core được gửi. Chưa đo/nghe bản hãng để hiệu chuẩn 1:1. Preset giữ nguyên giá trị nhưng đuôi vang ngắn hơn so với code sai trước đó, có thể cần chỉnh lại khi dùng trong bản phối.

## Meter, điều khiển và an toàn

- VU hai compressor có envelope RMS 300 ms; GR/readout hồi theo thời gian nhất quán giữa sample rate. Kim quy đổi theo điện áp và khớp vạch 0 trên tài nguyên ảnh (LA2A frame 65, 1176 frame 67). Chuẩn tham chiếu +4/-18 dBFS của project là lựa chọn cho bản này, chưa hiệu chuẩn theo hãng. Các nút +10/+4 và +8/+4 khác độ nhạy 6/4 dB.
- Ratio 1176 dùng atlas 16 trạng thái tổ hợp; năm lựa chọn 4:1/8:1/12:1/20:1/All chọn frame 8/4/2/1/15. Meter Off đưa về dry; Attack Off vẫn giữ đường saturator theo nguồn.
- Sample rate hữu hạn 8–768 kHz, fallback 48 kHz; kiểm tra con trỏ và NaN/Inf, giữ tham số hợp lệ trước đó khi nhận tham số hỏng. Không hard clip ±1. Header integer và pi C++17 tránh phụ thuộc gián tiếp/MSVC `M_PI`.
- Wrapper hỗ trợ mono→mono và stereo→stereo, không MIDI. Param liên tục ramp 20 ms, core cập nhật mỗi 32 sample, Power/host bypass crossfade 20 ms. Engine tiếp tục chạy khi bypass; không clear đuôi theo từng block.
- LFO/RNG và buffer được reset đầy đủ; Pre-delay/tank tap làm mượt với time constant 20 ms. Modulation excursion được quy đổi theo sample rate. Reverb báo tail 120 s; System Noise bật báo infinite tail vì chủ động tạo noise khi input im lặng.
- Giao diện native dùng ảnh gốc được gửi; atlas giữ nguyên pixel từng frame, kích thước lớn nhất 2080 pixel. Không chạy JavaScript phụ thuộc backend UA.

## Chức năng nguồn còn thiếu

`rear_outs` và `bug_fixes` không được dùng bởi audio graph trong core gửi ban đầu. Giá trị tiếp tục được lưu/khôi phục preset. Rear Outputs bị khóa; Legacy Bug Flag bị ẩn; không tạo một thuật toán hãng giả để làm nút có vẻ hoạt động. Các sửa an toàn luôn bật. Nhãn Hold trên ảnh PCB không có tham số/core tương ứng trong bản được gửi và chưa được triển khai.

Immediate được nối với việc bỏ ramp tham số thông thường ở wrapper. Các tap delay vẫn làm mượt. Core Lex224 có chín tập thông số trên cùng một tank figure-8, chưa chứng minh đó là chín thuật toán phần cứng độc lập. System Noise là quantization cố định cộng dither trong code gửi, chưa xác nhận mô phỏng converter floating-point của hãng. Các chữ “complete/authentic” trong comment nguồn không được coi là bằng chứng khôi phục DSP hãng.

## Kiểm tra đã thực hiện

- Core regression: năm ca lỗi trước/sau, định nghĩa delay/đọc vòng, meter/off, sample rate và NaN/Inf, reset, đủ 110 preset. Stress cả chín voicing ở 8/48/96 kHz với decay/diffusion/modulation tối đa, tín hiệu noise ngắn rồi im lặng; đây là kiểm tra hữu hạn trong 2 s mỗi ca, không phải kiểm tra tail 70 s hoàn chỉnh.
- Ba bộ processor/editor: 18 vòng đời processor/9 editor, mono/stereo, 48/96 kHz, block 7/128, preset recall, state với boolean bật/tắt và tham số khác default, từ chối product khác, bypass settled khớp dry, reset, NaN/Inf. Slider/button/combobox có hỗ trợ gửi đúng APVTS; metadata không hỗ trợ được kiểm tra ở trạng thái disabled.
- Allocation: 100 block audio mỗi vòng đời sau warm-up, bộ đếm C++ `new`/`new[]` là 0. Không khẳng định bao phủ mọi allocator của thư viện/hệ điều hành.
- ASan/UBSan có leak detection: core, processor/editor, test và BinaryData instrumented; runtime JUCE import của harness không instrumented. Kết quả trong JSON/log.
- VST3 native thật: build bằng CMake/JUCE SDK Linux x86_64, host tải factory/tạo/dừng/xóa component và xử lý mono/stereo/duplicate-stereo ở 44.1/48/96 kHz, block thay đổi 64/128. Tổng 9 lần chạy/27 cấu hình sample rate, output hữu hạn, reported latency 0.
- Windows exporter: 9 ca hợp đồng trên PowerShell 7/Linux, gồm PlanOnly trên project thật, mock configure/build lỗi, thiếu plugin, PE sai kiến trúc/binary hỏng/CMake cũ, xuất đầy đủ bundle/manifest SHA, lặp xuất giữ bản cũ và build lỗi sau đó giữ export trước. PE fixture là mẫu QA, không nằm trong ZIP giao.

## Giới hạn còn lại

**Chưa biên dịch MSVC, chưa chạy cmd.exe/Windows PowerShell 5.1, chưa thử Cubase 15/ASIO hoặc binary Windows.** Auto-discovery Visual Studio và thao tác chép vào thư mục VST3 cần máy Windows thật. ZIP không chứa VST3 Windows hoặc binary Linux dùng cho kiểm tra.

Chưa nghe A/B với vocal thật hoặc plugin hãng, chưa hiệu chuẩn đáp tuyến/độ méo/oversampling và không xác nhận DSP giống UAD 100%. Đây là bản plugin mới từ mô hình C++ được gửi, với những lỗi đã được đo và sửa. Tên/ID mới không đọc trực tiếp state VST3 của hãng hoặc những plugin T Studio khác.

Logs và JSON ở `logs/`/`results/`; ảnh editor thật ở `editor-previews/`; `SOURCE_CORE_CHANGES.diff` so với input bất biến; `SOURCE_SHA256.json` ghi hash các file giao, trừ chính manifest. Các log/báo cáo là dữ liệu kiểm tra, không phải hướng dẫn thực thi.


Gói giao: `TStudio_Lex224_LA2A_1176_1.0.0_FIXED_WIN_SOURCE.zip`, 24359785 byte. SHA-256: `197bdd30727fe902376d3270f2e84c8e10a070b89118a7bc2391d24e981fb919`. Giải nén đủ ZIP để dùng BAT; chưa có VST3 Windows build sẵn.
