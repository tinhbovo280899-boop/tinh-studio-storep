# Topline Vocal Tune và T-AutoTune: kết quả kiểm tra

Ngày lập báo cáo: 31/10/2026. Đã mở đủ ba tệp mới gửi. Đây là báo cáo phân tích, không phải bản plugin đã sửa hoặc bộ cài.

**Có hai vấn đề tái hiện được trong mã T-AutoTune: trộn Dry/Wet lệch thời gian và tốc độ Retune thay đổi theo sample rate. Chưa có phép đo âm thanh Topline để khẳng định độ trễ bằng 0, hoặc kết luận nguyên nhân duy nhất của cảm giác hát nặng/vang cứng.**

## 1. Đã đọc được gì từ ba tệp Topline?

| Tệp | Kết quả |
|---|---|
| `uaudio_topline_vocal_tune.vst3` | DLL Windows x64, 11.625.168 byte. Có lớp giao tiếp VST3 và thông tin plugin. |
| `algo.bundle.zip` | Giải nén an toàn: 58 mục, gồm DLL thuật toán 31.205.720 byte, manifest và preset. Manifest ghi Topline Vocal Tune 1.0.2. |
| `extras.zip` | 84 mục: preset NKS, metadata và ảnh giao diện; không có mã nguồn DSP. Không chạy tệp `.reg`. |

DLL thuật toán có cơ chế bảo vệ mã. Vùng `.text` có entropy gần 8 bit/byte, đi cùng cấu trúc bảo vệ và thông tin PACE. Không thể coi vùng này là mã máy thông thường để suy ra đầy đủ công thức DSP. Không giải mã hay sửa cơ chế bảo vệ, không chạy DLL/installer của hãng.

Các chuỗi chẩn đoán còn đọc được có nhắc `IN Mark`, `OUT Mark`, `OLA`, `PerFFT`, `Lookahead`, nhận diện tiếng hữu thanh/vô thanh và thay đổi cao độ/biên độ nhanh. Chúng gợi ý có xử lý theo chu kỳ giọng, đánh dấu cao độ và tổng hợp chồng cộng. **Chuỗi chẩn đoán không chứng minh thuật toán nào đang chạy, nhánh nào được dùng, công thức PSOLA cụ thể hay số mẫu trễ.**

Lớp giao tiếp có chuỗi `StreamAdapterZeroLatency`. Đây cũng không phải phép đo độ trễ. Các từ `Reverb Level`/`compress` xuất hiện trong thư viện giao tiếp chung không chứng minh plugin có reverb hoặc compressor trong đường vocal.

## 2. Điểm làm Topline giữ độ tự nhiên theo tài liệu chính thức

Đã đối chiếu manifest với hướng dẫn của Universal Audio:

| Điều khiển | Vai trò được UA mô tả |
|---|---|
| Speed | Tốc độ kéo về nốt, 0–400 ms; khác với độ trễ đường âm thanh. |
| Expressive | Cho phép sai lệch nhỏ quanh tâm nốt đi qua; tăng mức để bớt chỉnh và giữ biến thiên cao độ. |
| Elastic | Có thể giữ các đoạn luyến/trượt nằm ngoài vùng bắt nốt; tránh kéo mọi chuyển động giọng về tâm nốt. |
| Sustain | Kéo chậm hơn khi ngân dài, trong khi nốt ngắn vẫn theo Speed. |
| Low Latency, bản Native | Tùy chọn dành cho thu/live. Tắt để ưu tiên độ chính xác khi mix hoặc dịch cao độ lớn; DAW bù độ trễ được plugin khai báo. |

UA không công bố số mẫu hoặc số mili giây cụ thể trong các trang đã đọc. Vì vậy chưa thể gán cho Topline một con số trễ thực. Bản VST3 được gửi là Native; các thông tin về Apollo DSP/Console là một đường chạy khác.

**Natural trong Topline không tương đương nút Natural Vibrato của T-AutoTune.** Trong mã T-AutoTune được kiểm tra, Natural Vibrato thay đổi thành phần dao động giữa đường cao độ nhanh/chậm. Flex-Tune và Humanize cũng có mục tiêu giữ tự nhiên, nhưng công thức và thang điều khiển khác Topline. Đặt cùng số trên hai plugin không đảm bảo cùng mức chỉnh giọng.

Topline Vocal Tune là sản phẩm chỉnh cao độ chuyên biệt. UA phân biệt nó với Topline Vocal Suite có chuỗi preamp/compressor/EQ/effect. Tài liệu và thông số đã đọc không cho thấy một bộ reverb riêng trong Vocal Tune. Chưa có cơ sở nói Topline tự thêm vang khiến nó mượt hơn.

## 3. Phép đo trực tiếp trên mã T-AutoTune của anh

Nguồn: T-Studio VocalPack 1.7.0 đã gửi trước đó, bản sao `working-fixed` của lần sửa license/AutoKey I/O. Lần sửa đó giữ nguyên thân thuật toán DSP. Không thay đổi mã plugin trong đợt phân tích này.

Đã tách lớp `TAutoTuneChannelDSP` và chạy nguyên thân thuật toán bằng C++17 trên Linux. Chỉ tạo khai báo độc lập, thay quyền truy cập để quan sát trạng thái và cung cấp enum nốt giống nguồn. Không cần JUCE, không chạy license, không tải plugin của hãng. SHA-256 và phương pháp nằm trong `metrics.json`.

### Độ trễ khi chưa chỉnh cao độ

Điều kiện: mọi nốt đặt Bypass trong bộ chỉnh nốt, Formant tắt, transpose bằng 0, vibrato tổng hợp tắt. Đây vẫn là đường xử lý plugin, không phải Master Bypass. Làm nóng 0,5 giây rồi đưa xung vào.

| Sample rate | Đỉnh xung Wet 100% | Nhánh Dry nội bộ |
|---|---:|---:|
| 44,1 kHz | 243 mẫu, khoảng 5,51 ms | 256 mẫu, khoảng 5,80 ms |
| 48 kHz | 264 mẫu, 5,50 ms | 256 mẫu, khoảng 5,33 ms |
| 96 kHz | 512 mẫu, khoảng 5,33 ms | 256 mẫu, khoảng 2,67 ms |
| 192 kHz | 512 mẫu, khoảng 2,67 ms | 256 mẫu, khoảng 1,33 ms |

Ở 44,1 kHz, vị trí đọc Wet là 242,55 mẫu và được nội suy; bảng ghi mẫu có đỉnh xung. Khi có sửa cao độ, vị trí đọc Wet còn thay đổi theo chu kỳ và điểm nối. **Không được lấy 264 mẫu rồi coi đó là độ trễ cố định cho mọi chế độ.**

Nguồn processor chưa có lời gọi `setLatencySamples`. Điều đó không phù hợp với việc hiểu plugin là đường âm thanh không trễ. Muốn khai báo trễ đúng cần thiết kế đường ra có độ trễ xác định; chỉ sửa số báo cho host không làm âm thanh đến tai nhanh hơn. Các phép đo trên cũng chưa tính buffer ASIO, chuyển đổi A/D–D/A hay plugin khác.

### Lỗi trộn Dry/Wet: đã tái hiện bằng tín hiệu

Dry dùng mốc 256 mẫu cố định, trong khi Wet có vị trí đọc khác. Ngay cả trạng thái chưa sửa nốt, đặt Mix 50% đã làm hai nhánh triệt một số dải âm:

| Điều kiện | Kết quả đo |
|---|---|
| 48 kHz, Mix 50%, sine 1 kHz | Khoảng −1,25 dB so với đầu vào. |
| 48 kHz, Mix 50%, sine 3 kHz | Triệt gần hoàn toàn trong bài thử lý tưởng. |
| 96 kHz, Mix 50%, sine 187,5 Hz | Triệt gần hoàn toàn trong bài thử lý tưởng. |
| 96 kHz, Mix 50%, sine 1 kHz | Khoảng −6,02 dB. |
| Mix 100%, cùng bài thử trung tính | Các tần số thử giữ mức xấp xỉ 0 dB. |

Giá trị triệt sâu trong JSON được chặn hiển thị ở −200 dB. Đây là sai lệch pha/thời gian khi cộng hai đường, không phải phép đo compressor. Biểu đồ đính kèm biểu diễn mô hình từ độ trễ đã đo và các điểm đo sine thực tế.

**Lỗi này có ý nghĩa khi dùng Mix ở giữa. Nếu anh đang dùng Mix 100%, không quy ngay cảm giác vang cứng cho lỗi Mix.** Khi đang tune, hai tín hiệu còn khác cao độ nên không thể hứa hết mọi tương tác chỉ bằng một delay cố định ở nhánh Dry.

### Retune chưa đồng nhất giữa các sample rate

Bộ điều khiển cập nhật sau mỗi 5 mẫu của đường phân tích, nhưng hệ số làm mượt được tính bằng sample rate đường âm thanh. Khi hạ mẫu phân tích 2 hoặc 4 lần, thời gian Retune thực bị kéo dài tương ứng.

Đo đáp ứng khi transpose chuyển 0 → +12 semitone sau khi đã bắt ổn định giọng 220 Hz. Tham số Retune luôn là 20 ms; ghi thời gian tỷ lệ dịch đi được nửa khoảng từ 1 lên 2:

| Sample rate | Thời gian đi được nửa đáp ứng |
|---|---:|
| 48 kHz | 19,94 ms |
| 96 kHz | 40,04 ms |
| 192 kHz | 79,94 ms |

Đây là **thời gian bám nốt**, không phải số mili giây trễ tín hiệu đến tai. Cùng cơ sở thời gian còn được dùng ở Humanize và LFO vibrato; cần rà đồng bộ khi sửa, thay vì đổi một hệ số đơn lẻ.

### Có bị nén giọng không?

Trong bài thử trung tính, Formant/vibrato tổng hợp tắt và Mix 100%, nhiễu thử ở mức đỉnh −40/−24/−12/−6 dBFS đều giữ tỷ lệ RMS đầu ra/đầu vào 0 dB sau căn trễ. Đường này không thể hiện đáp ứng nén theo mức.

Nguồn có nhân biên độ cho vibrato tổng hợp, bộ lọc Formant/LPC và chặn mẫu ở ±4. Các phần đó có thể đổi biên độ/timbre trong điều kiện khác. Kết quả trung tính không chứng minh mọi preset hoặc mọi mức đầu vào đều giữ âm lượng. Chưa đo được compressor hay độ méo của binary Topline/Auto-Tune Pro của hãng.

## 4. Vì sao Send vang giữ nguyên mà nghe khác?

Anh đã xác nhận bật Direct Monitor ở soundcard và Monitor trong Cubase. **Nếu cả hai thực sự phát đồng thời**, tai đang nhận một đường khô trực tiếp và một đường đã qua tune/buffer. Khi thay plugin, quan hệ thời gian, cao độ và mức âm giữa hai đường thay đổi. Giọng có thể nghe đục, rỗng hoặc nặng; tỷ lệ vocal/vang cảm nhận cũng thay đổi.

Lưu ý Cubase có ASIO Direct Monitoring: trong cấu hình đó, nút Monitor có thể điều khiển chính đường hardware monitor. Chỉ nhìn hai nút bật chưa chứng minh có hai đường âm; cần mute từng đường để xác nhận routing thực.

Nếu Send lấy âm sau insert Tune, thay đổi phổ âm, độ luyến, mức hoặc transient của đầu vào cũng thay đổi tiếng đưa vào reverb. Giữ nguyên nút Send/Return chưa đảm bảo reverb nhận cùng một tín hiệu. Tương tự, nếu Return vang còn trộn vocal khô thì có thêm một đường song song cần kiểm tra. Chưa có ảnh routing hiện tại để kết luận trường hợp nào đang xảy ra.

## 5. Cách thử ngắn trong Cubase 15 Windows

1. Tắt/mute đường Direct Monitor ở mixer của soundcard; xác nhận chỉ còn một đường vocal nghe qua Cubase. Nếu dùng ASIO Direct Monitoring, kiểm tra đường thực sự đang phát bằng cách mute từng nhánh.
2. Giữ nguyên sample rate, ASIO buffer, chuỗi insert còn lại và vị trí Send. Chỉ bật một plugin Tune mỗi lần. Với T-AutoTune, thử Mix 100%, Formant tắt trước. Với Topline Native, bật Low Latency; bỏ dịch quãng/hòa giọng bằng Shift khi so chế độ chỉnh nốt thông thường.
3. So trên cùng một bản vocal gốc. Căn mức đầu ra, không chỉ nhìn vị trí knob. Nghe vocal khô trước, rồi bật lại cùng FX Send. Return reverb dùng 100% Wet nếu đang phục vụ như AUX.
4. Ghi nhận: tắt đường monitor trực tiếp có giảm cảm giác nặng/vang cứng không? Nếu còn, xuất cùng đoạn qua Topline và T-AutoTune, kèm đoạn gốc và thông số đang dùng. Chưa có ba bản thu cùng điều kiện này để chốt nguyên nhân.

Để đo độ trễ live tuyệt đối cần ghi/loopback đường ra ở cùng cấu hình ASIO; bản export offline đã được DAW bù trễ không tự cho biết độ trễ người hát nghe được. Khi đánh giá màu tiếng bằng playback thì căn thời gian các bản trước khi so A/B.

## 6. Thứ tự sửa T-AutoTune hợp lý

1. Sửa cơ sở thời gian của các điều khiển theo tốc độ phân tích; kiểm tra Retune/Humanize/vibrato ở 44,1/48/96/192 kHz.
2. Thiết kế lại trộn Dry/Wet để không có triệt phổ ở trạng thái trung tính. Khi có dịch cao độ phải xác định rõ ý nghĩa Mix và cách ghép hai đường.
3. Xác định độ trễ đầu ra, khai báo đúng với host và thử qua Cubase Windows. Không dùng một hằng số để che việc Wet đang có vị trí đọc biến thiên.
4. Sau khi routing và mức âm đã được đối chiếu, mới đánh giá thêm cơ chế giữ luyến/ngân/đầu phụ âm để làm bản live mềm hơn. Hành vi Expressive/Elastic/Sustain trong tài liệu UA là hướng tham khảo về chức năng; chưa có công thức DSP độc quyền đã được xác minh để sao chép.

Không thay đổi plugin, preset, ID, license hoặc public key trong lần phân tích này. Không có xác nhận đã hết lỗi trên Cubase và không có bản build Windows/macOS mới.

## Nguồn và khả năng tái lập

- Hướng dẫn chính thức: https://help.uaudio.com/hc/en-us/articles/49175014433556-Topline-Vocal-Tune-Manual
- FAQ chính thức: https://help.uaudio.com/hc/en-us/articles/49263310693780-FAQ-Topline-Vocal-Tune
- Trang sản phẩm: https://www.uaudio.com/products/topline-vocal-tune
- Đã truy cập trong đợt kiểm tra 31/10/2026. Hành vi mô tả theo tài liệu hiện hành; không giả định mọi phiên bản binary giống nhau.
- `INPUTS.json`: kích thước và SHA-256 ba tệp gửi.
- `metrics.json`: số đo T-AutoTune, SHA-256 nguồn và điều kiện từng phép đo.
- `run_own_dsp_probe.py`: script tái lập. Chạy bằng Python 3 và trình biên dịch `g++` hỗ trợ C++17: `python3 run_own_dsp_probe.py /duong/dan/AutoTune/Source`. Script đọc nguồn, tạo chương trình đo trong thư mục riêng, không sửa nguồn. Không cần DLL Topline hoặc JUCE.
- Gói báo cáo không kèm binary/asset/preset độc quyền của UA, bản thu riêng hay mã license.
