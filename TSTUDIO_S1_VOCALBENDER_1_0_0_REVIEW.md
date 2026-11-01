# Kiểm tra và sửa S1 / Vocal Bender

**Đã sửa các lỗi tái hiện được trong mã nguồn gửi vào, thêm hai wrapper/giao diện VST3 native và BAT Windows x64.** Chưa xác nhận giống Waves 100%; chưa chạy MSVC/Cubase 15 Windows hay macOS. Gói gốc gồm core C++, ảnh/XML/disassembly và bài thử nhỏ, chưa có dự án build VST3/GUI hoàn chỉnh.

## Lỗi chính và thay đổi

| Phần | Hiện trạng gốc tái hiện được | Bản sửa |
| --- | --- | --- |
| S1, ma trận trung tính | Width 1, Rotation/Asymmetry 0, Imager: ma trận `[1,1; -1,1]`. Nguồn mono thành L=2x, R=0 | Ma trận identity, Width 0 ra `(L+R)/2` ở cả hai kênh |
| S1, M/S | Phép đổi cơ sở cộng sai hàng/cột; Output M/S không phải phép biến đổi nghịch đảo đúng mức | Dùng `(L+R)/√2`, `(L−R)/√2`, kiểm tra encode/decode vòng kín giữ mức |
| S1, polarity/swap | Đảo các phần tử chéo ma trận thay vì toàn kênh; swap cột bị mô tả là swap đầu ra | Đảo đúng kênh và swap sau xử lý; chuyển có làm mượt |
| S1, bass shelf | Tử số b0/b1 bị đổi chỗ; ở Nyquist đáp ứng là −1 thay vì +1 khi bật shelf | Sửa thứ tự; kiểm tra gain DC theo Shuffle/Trim và gain Nyquist +1, pole trong vòng đơn vị |
| Vocal Bender, semitone | Hàm đổi tỷ lệ cho +12 ≈13.998; −12 ≈0.07144. Sau đó tỷ lệ bị chặn, lệch nốt | `2^(semitone/12)`: +12=2, −12=0.5, Fine đúng cents |
| Vocal Bender, grain | Window chỉ một chu kỳ, scheduler bỏ phần dư lẻ, chọn lẫn đỉnh dương/âm; đổi sample rate làm window chạy sai tỷ lệ | Window hai chu kỳ đầu vào trước Formant, cùng cực đỉnh, giữ phần dư scheduler và thời gian ở đúng đơn vị sample |
| Vocal Bender, đọc phân số | Lệch một cột sinc khi nội suy | Lấy đúng hai phase kề nhau |
| Vocal Bender, level | Peak matcher nhanh tăng/giảm gain theo hạt, tạo thêm điều chế mức/DC | Bỏ peak rider nhanh, chuẩn hóa phần chồng lấp dư và chặn DC wet 15 Hz. Khi hạ pitch giữ taper cửa sổ để còn subharmonic |
| Vocal Bender, trung tính/reset | Vẫn qua hạt/envelope dù không dịch; một số bộ nhớ không reset đầy đủ | Neutral/Mix 0 dry chính xác; reset cả modulation/flatten blend/DC và lịch hạt |
| Vocal Bender, stereo/sample rate | L+R ngược pha làm bộ dò mất giọng; maxLag 1024 làm mất vùng trầm ở sample rate cao | Dò theo một kênh với chọn kênh có hysteresis, hạt liên kết stereo; lag/ring phân tích tăng theo sample rate |
| Host/UI | Chưa có wrapper/native GUI và bộ xuất đầy đủ | APVTS automation/state/preset, typed value parser có đơn vị, 17 preset T Studio, hai ID riêng, bypass 20 ms, BAT kiểm tra đủ hai bundle PE x64 |

## Bằng chứng đo

- Đầu vào harmonic 220 Hz, Pitch +5: bản gốc có thành phần mạnh gần **432 Hz**, bản sửa gần **294 Hz**, mục tiêu **293.665 Hz**. Dò F0 đầu vào đúng không đồng nghĩa Pitch đầu ra đúng; bài thử gốc chưa đo điều này.
- Đo một vocal tổng hợp có phổ formant: Pitch 0, Formant −6 / 0 / +6 cho spectral centroid khoảng **859 / 1224 / 1737 Hz**. Nốt đầu vào không thay đổi; đây là phép thử phản hồi Formant, không phải chứng nhận giống giọng thật/hãng.
- Hạ 1 octave có subharmonic 110 Hz. Đỉnh phổ mạnh nhất có thể là bội âm 220 Hz hoặc cao hơn khi giữ formant, nên không dùng duy nhất đỉnh FFT lớn nhất để kết luận F0.
- S1 đã kiểm tra trung tính, width 0, đảo pha/swap, vòng M/S và shelf qua 8/44.1/48/96/192/384 kHz. Vocal Bender đã kiểm tra block invariance, nốt mục tiêu, stereo ngược pha/right-only, reset, silence và giá trị không hữu hạn; các phép thử âm chính qua 8–192 kHz.

## Kiểm tra đã hoàn thành

- Core: **677.593 phép kiểm tra, không lỗi**.
- Processor/editor: mỗi sản phẩm **6 vòng tạo/hủy processor, 3 vòng editor**, 8 + 9 preset, round-trip state/boolean/typed value, bypass và input NaN/Inf. 12 vòng audio đã warm ×100 block không phát sinh C++ `new/new[]` được bộ đếm chặn; không khẳng định đã chặn mọi malloc của hệ thống.
- Ba chương trình regression qua ASan + UBSan. Mã core/processor/editor/tests được instrument; thư viện JUCE QA nhập sẵn không được instrument; leak detection tắt. Không coi đây là chứng nhận mọi đường đi không có lỗi.
- Hai **VST3 Linux thật** được build với JUCE SDK đầy đủ và nạp qua VST3 factory/component/controller: **24 cấu hình sample rate/bus/automation** ở 44.1/48/96 kHz, block 64/128 thay đổi. Có đưa Pitch +5 và Width qua host parameter queue. S1 từ chối mono đúng thiết kế. Log và SHA binary thử nằm trong results; không đóng binary Linux vào ZIP Windows-source.
- Script xuất Windows: 9 hợp đồng PowerShell/CMake mock trên Linux, gồm bản CMake cũ, thiếu sản phẩm, lỗi configure/build, binary sai kiến trúc/sai header, xuất lặp và giữ bản xuất cũ khi thất bại. **Đây không phải compile Windows.**

## Phạm vi cần hiểu khi sử dụng

Bản này là hiệu ứng S1-style matrix/shuffler và pitch/formant từ C++ được gửi vào có sửa, không phải mã nguồn gốc hãng được phục hồi 100%. Dòng địa chỉ disassembly và chữ “exact” trong file gốc không chứng minh phép port đúng. Bộ dò trong C++ dùng autocorrelation + refinement, không có đường FFT/cepstrum đầy đủ như lời mô tả trong README gốc. Không dựng các nút vocoder/sequencer khác chỉ có trong XML khi thiếu DSP tương ứng.

Giao diện hoạt động là T Studio mới, dùng native JUCE, không nhúng logo/skin hãng. Toàn bộ file gửi vào được giữ nguyên tại Reference/ORIGINAL_UPLOAD.zip; ảnh/XML/preset gốc nằm ở đó để đối chiếu, không tham gia build. Hai plugin mới dùng ID riêng và không nhận project state của bản hãng/bản cũ.

Vocal Bender không phải autotune theo Key/Scale. PSOLA hiện thiên về nguồn giọng đơn âm; chưa đánh giá nghe trên bộ vocal thật của anh cho hai hiệu ứng này, nguồn nhiều nốt, tất cả pitch/formant cực hạn hoặc thao tác trong Cubase. Mức RMS có thể đổi khi dịch phổ, nhất là ±12; không còn tự động peak rider nhanh để ép mức.

Không có lookahead hàng đợi cố định, nên host báo 0 sample; **wet Pitch/Formant vẫn đọc hạt quá khứ có tuổi thay đổi theo giọng/thiết lập**. Không được suy ra độ trễ hát live gần 0 từ con số host này. Neutral và Mix 0 là dry tức thời; không có một độ trễ wet cố định để bù pha toàn bộ Mix ở các thiết lập đang dịch âm.

Windows/Cubase 15 vẫn là bước kiểm tra trên máy anh sau khi build. README hướng dẫn BAT, đường xuất và chép nguyên thư mục VST3.
