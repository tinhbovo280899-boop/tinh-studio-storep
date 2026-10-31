# Kết quả sửa ba dự án — 31/10/2026

## Kết quả

Ba ZIP đầu vào có core C++ và tài nguyên web GUI/preset nhưng không có project CMake/JUCE hay wrapper VST3. Bản giao bổ sung project dùng chung một cây JUCE 8.0.12, ba plugin độc lập, giao diện native, automation, state, preset và script xuất Windows x64.

| Plugin | Tham số DSP + bypass mới | Factory preset | Code / Bundle ID |
| --- | ---: | ---: | --- |
| T-EQP1A | 10 + 1 | 62 | Tep1 / com.tstudio.eqp1a |
| T-MEQ5 | 8 + 1 | 48 | Tme5 / com.tstudio.meq5 |
| T-Century | 15 + 1 | 61 | Tcnt / com.tstudio.century |

Manufacturer code là `Tstd`. Tên/ID mới không thay thế plugin UAD hoặc T-PultecEQ cũ. Đây là các triển khai DSP được gửi dưới dạng C++; không có chứng cứ khôi phục 100% mã nguồn hoặc âm thanh gốc của UAD.

## Lỗi đã tái hiện và sửa

| Trường hợp | Trước sửa | Sau sửa |
| --- | --- | --- |
| EQP1A: gửi lại cùng tham số mỗi 128 sample so với chỉ gửi một lần | Sai khác đỉnh 0.023052174598 | Sai khác 0 trong bài thử |
| MEQ5: cùng phép thử | Sai khác đỉnh 0.0201880522072 | Sai khác 0 |
| Century: mono đảo cực so với stereo có hai kênh đầu vào giống nhau | Sai khác đỉnh 0.00851690769196 | Sai khác 0 |
| Kim GR Century khi 0 dB giảm gain | Getter chọn frame 99, trong khi ảnh frame 0 nằm tại vạch 0 dB | Getter và editor chọn frame 0; giảm gain đi về bên trái |

Hai EQ trước đó gán một đối tượng Biquad mới cho filter, đồng thời xóa trạng thái z1/z2. Bản sửa chỉ thay hệ số. Century mono trước đó áp dụng cực tính lần nữa lên tín hiệu dự phòng cho kênh phải, làm sai bộ dò stereo-linked và xử lý bất đối xứng.

Các sửa khác: hằng số pi C++17 thay `M_PI`; kiểm tra sample rate hữu hạn 8–768 kHz, fallback 48 kHz; chặn tần số filter trên Nyquist khi chạy sample rate thấp; giữ giá trị tham số trước nếu nhận NaN/Inf; vệ sinh NaN/Inf audio; giữ headroom float thay vì hard clip tại ±1. Baseline log chỉ chạy các ca tái hiện lỗi, nên exit 1 trong log baseline là kết quả mong đợi.

Output/Power dùng chung một tham số ở hai Pultec là quy ước có chủ ý trong JavaScript được gửi. Bản sửa giữ quy ước này. Không tách thành một tham số DSP mới, không xóa/đổi thứ tự preset.

## Wrapper và giao diện

Giữ thứ tự và ID tham số DSP, thêm `bypass` ở cuối. Param float dùng 0..1 với đơn vị hiển thị theo core; switch/choice có bước rời rạc. Bộ khung mới làm mượt giá trị liên tục 20 ms và cập nhật filter mỗi 32 sample, không reset filter. Gain Output và crossfade dry/bypass được xử lý từng sample. Không cấp phát bộ nhớ theo block audio trong phần wrapper sau prepare.

Output core được giữ tại unity trong wrapper để gain master làm mượt riêng. Century VU đo tín hiệu sau gain master với hằng số 300 ms. Core DSP vẫn có thể dùng độc lập và giữ cách hoạt động Output gốc.

Các núm/công tắc native kết nối APVTS trực tiếp, không phụ thuộc UA web backend. Giao diện có preset combo, bypass, tooltip, resize giữ tỷ lệ, chỉ thị input/output; Century có GR/VU và đèn trạng thái. Ảnh @2x được xếp lại thành atlas; mọi frame giữ nguyên pixel RGBA, kích thước lớn nhất 3168 pixel. Không khẳng định mọi hành vi của web GUI gốc hoặc menu của host đã được sao chép.

## Kiểm tra đã đạt

- **CoreRegression:** lặp tham số, mono đảo cực, reset, sample rate 8/16/32/44.1/48/96/192/384 kHz và fallback cho Inf, audio/param NaN/Inf, kim GR, toàn bộ 171 preset.
- **Ba bộ processor/editor:** tổng 18 vòng đời processor và 9 vòng đời editor, mono/stereo, 48/96 kHz, block 7/128, recall mọi factory preset, state với mọi boolean bật/tắt và tham số liên tục, từ chối state của product khác, bypass settled khớp dry, reset im lặng, mọi control GUI gửi đúng tham số. Century VU chênh 18 dB đúng với hai mức master được thử.
- **Allocation:** bộ đếm C++ `new`/`new[]` trong 100 block mỗi vòng đời sau warm-up báo 0. Đây không phải phép đo tất cả allocator của thư viện hoặc hệ điều hành.
- **ASan/UBSan:** bốn bộ thử trên đều exit 0, bật phát hiện leak. Core, processor/editor mới, test và BinaryData được instrument; runtime JUCE import dùng cho các bộ test này không được instrument.
- **VST3 native Linux:** CMake dùng JUCE SDK thật, biên dịch đủ ba wrapper VST3. Source module JUCE của SDK trùng với cây vendored; `modules/CMakeLists.txt` chỉ có trong cây nguồn vendored. Host thử tải factory, tạo/dừng/xóa component và xử lý mono/stereo/duplicate-stereo ở 44.1/48/96 kHz, block thay đổi 64/128: 9 lần chạy host, 27 cấu hình sample rate đều exit 0, output hữu hạn và reported latency 0.
- **Windows exporter:** parse PowerShell và 9 ca kiểm tra hợp đồng trên PowerShell 7/Linux: lệnh build thật ở chế độ PlanOnly; mô phỏng configure/build lỗi, thiếu một plugin, PE sai kiến trúc, binary hỏng, CMake quá cũ; xuất đủ bundle và SHA-256; build lặp giữ bản cũ; build lỗi sau đó giữ nguyên export trước. Binary PE trong các fixture chỉ là mẫu QA, không có trong ZIP giao.

## Giới hạn kiểm tra

**Chưa biên dịch bằng MSVC hoặc chạy Windows/Cubase 15, chưa thực thi BAT bằng cmd.exe/Windows PowerShell 5.1.** Windows script chỉ được parse/chạy phần điều phối trên PowerShell 7/Linux, với generator chỉ định khi kiểm tra. Discovery Visual Studio và cài vào thư mục VST3 cần máy Windows thật. Các binary Linux dùng để kiểm tra không nằm trong ZIP và không thể dùng trên Windows.

Chưa nghe A/B với bản hãng ở mức khớp nhau, chưa xác nhận đáp tuyến/phi tuyến hoặc cảm giác âm thanh tương đương UAD. Không nâng các mô hình RBJ/tanh được gửi thành mô phỏng phần cứng đã hiệu chuẩn. Các sửa này xử lý lỗi phần mềm tái hiện được, không phải xác nhận màu tiếng hoàn hảo.

Plugin báo 0 sample latency vì core là xử lý causal. Điều đó không có nghĩa độ trễ microphone–tai nghe bằng 0; ASIO, converter và plugin khác vẫn góp độ trễ.

## File bằng chứng

`results/` lưu kết quả JSON, `logs/` lưu các log liên quan, `editor-previews/` là ảnh chụp editor JUCE thực. `SOURCE_CORE_CHANGES.diff` so sánh các core với bản gửi. `SOURCE_SHA256.json` ghi hash mọi file gói giao, trừ chính manifest. Inventory đầu vào ghi các file đã giải nén từ ba ZIP. Báo cáo và log là dữ liệu kiểm tra, không phải các lệnh cần chạy khi build.


Gói tải: `TStudio_3VST_EQP1A_MEQ5_Century_1.0.0_FIXED_WIN_SOURCE.zip` (34093302 byte). SHA-256: `65e56be9e54162c005978559e69d075ddada582cdcdf3a3773dbe436a334e159`. File BAT và JUCE nằm trong ZIP; giải nén đủ trước khi chạy.
