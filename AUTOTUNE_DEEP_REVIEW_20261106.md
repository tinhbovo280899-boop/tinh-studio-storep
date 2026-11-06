# AutoTune — rà soát sâu ngày 06/11/2026

**Kết luận:** lõi dò/nắn nốt đứng đạt độ chính xác tốt trong bài thử có đích xác định khi Flex=0 và Transpose=0. Còn những điểm ảnh hưởng trực tiếp đến tâm nốt, lựa chọn Key/Scale và hành vi lúc bật Formant. Chưa kết luận độ giống UAD hoặc chất lượng trên vocal thật từ bộ thử này.

ZIP mới có checksum khác nhưng **cả 5 file source giống từng byte bản gửi ngày 05/11**. Không sửa file gốc. Kết quả Flex/Vibrato cũ được dùng lại sau khi xác minh hash từng file; các bài thử khác dưới đây được chạy thêm.

[Biểu đồ kết quả](AUTOTUNE_DEEP_REVIEW_20261106.png) · [Dữ liệu và probe](AUTOTUNE_DEEP_REVIEW_20261106_DATA)

## Các điểm ưu tiên

### 1. Đồng bộ Auto-Key có thể ghi đè state đã lưu dù Sync OFF

**Điều kiện:** plugin mới được tạo, project hoặc UI tắt Auto-Key trước tick timer đầu tiên khoảng 250 ms, và còn file handoff hợp lệ chứa key/scale khác.

- `TAutoTuneProcessor.h:427`: pendingHandoffSync khởi tạo bằng 2.
- `TAutoTuneProcessor.cpp:1634`: timer đổi pending==2 thành forceReload=true, gọi sync không kiểm tra autoKeySyncAtom trước.
- `TAutoTuneProcessor.cpp:2047`: setStateInformation khôi phục autoKeySync=false nhưng không hủy pending lần đầu.
- `AutoKeyHandoffReader.h:309`: forceReload=true được phép bỏ qua autoSyncEnabled=false.
- `TAutoTuneProcessor.cpp:1593`: khi poll trả valid, sync ghi Key, Scale, ReferencePitch và note mask.

Probe reader với đường file giới hạn vào fixture cho kết quả: OFF + poll thường không cập nhật; OFF + forced first poll vẫn cập nhật, thay state Bb Minor bằng payload C Major. Phần kết nối constructor/timer/state được lần theo mã nguồn; chưa chạy nguyên plugin trong Cubase.

**Hướng sửa:** timer phải tôn trọng trạng thái Sync đã restore; hủy yêu cầu initial forced sync khi OFF. Forced reload thủ công chỉ nên chạy từ thao tác bật/refresh rõ ràng của người dùng. Đây là điểm cần xử lý trước khi so độ chuẩn cả câu với plugin khác.

### 2. Flex-Tune để lại sai lệch tĩnh, không chỉ làm mềm quá trình kéo

`TAutoTuneProcessor.cpp:996`: target được pha giữa cao độ đầu vào và nốt chuẩn bằng effectivePull<1. Khi giữ nguyên nốt, phần sai lệch này không tự hết.

Ở mặc định trong mã: Retune 25 ms / Flex 15 / Humanize 35 / Mix 100 / Formant Off, tone vào lệch 50 cent còn khoảng **18,05 cent** sau khi ổn định; lệch 30 cent còn khoảng **6,34 cent**. Đổi riêng Flex=0 cho sai lệch dưới 1 cent ở bộ đo trước. Với rung 25 cent quanh tâm lệch +30 cent, đổi riêng Flex=0 đưa tâm về gần 0 trong khi rung vẫn tồn tại.

Bài thử mới: **20 ca nốt đứng**, các nốt E2/A2/A3/A4/A5, voice type phù hợp, 44.1/48/96/192 kHz, Flex=0, Transpose=0, Formant Off; sai lệch phổ đầu ra lớn nhất khoảng **0,071 cent**. Đích nốt được cố định trong note mask để không lẫn lỗi chọn thang âm. Đây không phải độ chính xác mọi vocal/noise/thiết lập.

**Hướng sửa:** tách độ mềm ở onset/luyến khỏi việc giữ tâm nốt lâu dài. Khi nốt đã ổn định, đưa tâm về đích; giữ phần rung quanh tâm ở một nhánh riêng. Không tăng correction mạnh vô điều kiện khi key/tracker sai.

### 3. FREEZE trên đồng hồ giữ cả nốt tune

- `TAutoTuneEditor.cpp:612`: click vùng nốt lớn trên tuner gọi setHoldMeter.
- `TAutoTuneProcessor.cpp:1680`: setHoldMeter(true) đặt frozenMidiNoteAtom theo nốt đích; khi chưa có nốt hợp lệ, dùng MIDI 60 (C4).
- `TAutoTuneProcessor.cpp:1854`: frozen note được truyền vào DSP, đi trước chọn nốt theo scale.

Trong thử câu A3 → B3 → C#4, FREEZE=A3 làm target vẫn A3 ở các đoạn sau. UI cũng ngừng cập nhật meter khi hold, nên có thể không thể hiện rõ cao độ đang bị kéo về nốt cũ. Đây có thể là tính năng Hold Pitch có chủ ý; tên setHoldMeter/TUNER FREEZE dễ gây hiểu nhầm nếu người dùng chỉ muốn giữ màn hình. Không kết luận người dùng đang bật nó.

**Hướng sửa:** tách Hold Display và Hold Pitch, đặt tên rõ; không tự khóa C4 khi chưa có nốt voiced hợp lệ.

### 4. Transpose ±12 không còn đủ biên để sửa cao độ

`TAutoTuneProcessor.cpp:1094` và `:1202` đều chặn tỷ lệ về [0.5,2]. Khi yêu cầu đã dùng hết một octave, không còn biên cho phần correction cùng chiều.

| Thiết lập | Nốt vào | Đích | Đo đầu ra |
| --- | --- | --- | --- |
| Transpose −12, input +30 cent | A3 hơi cao | A2 = 110 Hz | 111,923 Hz, còn +30,01 cent |
| Transpose +12, input −30 cent | A3 hơi thấp | A4 = 440 Hz | 432,439 Hz, còn −30,01 cent |

Đã tắt Flex/Humanize và để Retune 0 trong probe biên để tách khỏi tác dụng của các nút này. Với chiều không chạm giới hạn, có trường hợp còn sai lệch nhỏ khoảng 2,5 cent ở +12; trong khi các nốt đứng không transpose đạt kết quả gần 0 như trên.

**Hướng sửa:** bố trí biên tỷ lệ cho cả transpose + correction, rồi kiểm tra lại an toàn read-head/crossfade ở tỷ lệ mới. Không chỉ nới clamp mà bỏ qua giới hạn causal/ring buffer.

### 5. Formant còn vọt đỉnh ở transient, rõ hơn ở sample rate cao

Bộ phục hồi LPC tại `TAutoTuneProcessor.cpp:1303` theo dõi de-emphasis rồi reset lịch sử và chuyển tạm sang nhánh không Formant 50 ms. Đây là biện pháp phục hồi; việc output luôn hữu hạn không đảm bảo không vọt đỉnh hoặc đổi màu lúc chuyển nhánh.

Bài stress harmonic có thay đổi mức tức thời, Throat 100%, Flex=0, không thêm gain:

| Ca thử | Đỉnh đầu vào | Đỉnh Formant Off | Đỉnh Formant On |
| --- | --- | --- | --- |
| 96 kHz, A4+30 cent, đổi mức | 0,548 | 0,580 | **1,878** |
| 192 kHz, E2+30 cent, đổi mức | dưới 0,63 theo giới hạn tín hiệu | 0,565 | **1,141** |

Ca 96 kHz vọt đỉnh gần thời điểm tăng mức; 1,878 tương ứng khoảng +5,47 dBFS nếu đi vào đường phát/export giới hạn 0 dBFS. Ở ca E2/192 kHz bộ guard bị kích hoạt 405 lần trong 2 s — đếm lần reset, **không phải 405 tiếng nổ nghe được**. Các ca vowel/rung thông thường trong bộ thử có ít lần phục hồi hơn, thường ở đầu âm. Không quan sát NaN/Inf ở đầu ra.

Đây là bài stress tổng hợp, không chứng minh vocal thật của người dùng đang có tiếng nổ. Cần đo onset/plosive và nghe A/B để đánh giá mức nghe thấy. Lưu ý LPC dùng frame 512/hop64 ở mọi sample rate; thời lượng frame ngắn đi 4 lần tại 192 so với 48 kHz, và nội suy hệ số synthesis không tự chứng minh ổn định mọi thời điểm.

**Hướng sửa:** kiểm tra ổn định/khớp thời gian của LPC và nội suy, chuẩn hóa theo sample rate, kiểm tra mức sau nhánh reconstruction/crossover; không chỉ dựa vào clip cuối ±4 hoặc tăng số lần reset.

## Auto-Key và điều khiển cần làm chắc thêm

- Reader chấp nhận payload chỉ có Key rồi tự dùng Major; chỉ có Scale rồi tự dùng key C. Probe XML key-only A trả A Major; scale-only Minor trả C Minor. Nếu writer đang ghi dở hoặc nguồn chỉ gửi một trường, không nên coi đó là snapshot đủ để thay cả thang âm. Cần writer ghi atomic và reader phân biệt trường vắng mặt.
- Một dòng JSON `{"Key":"A","Scale":"Minor"}` không được parser hiện tại nhận; bản JSON xuống dòng lại nhận. Tên thang âm 14–28 không được nhận dạng dạng chữ, dù số index vẫn được nhận. Đây là giới hạn tương thích; chưa biết định dạng handoff thực tế trên máy người dùng.
- Dò tông nội bộ dùng histogram pitch-class của vocal trong 3 giây voiced, tự áp kết quả, chưa có ngưỡng confidence/unknown. Một đoạn một vài nốt không đủ xác định duy nhất tông bài nhạc. Không nên coi kết quả đó là chắc chắn hoặc thay tông đang dùng trong lúc hát mà không báo rõ.
- startInputTypeLearn/startAutoKeyDetect reset các bộ đếm/array không atomic từ UI, trong khi processBlock cập nhật chúng. Nếu bấm lại lúc đang chạy có đường truy cập chồng nhau; cờ relaxed không tạo cơ chế sở hữu/snapshot. Đây là rủi ro concurrency từ việc đọc code, chưa chạy TSan hoặc chứng minh crash. Nên gửi request để audio thread tự reset ở đầu block, công bố kết quả về timer/UI.
- note mask/key/scale được ghi từ timer/UI/audio qua nhiều atomic riêng rẽ; chưa có snapshot đồng bộ toàn bộ cấu hình. Cần kiểm tra giao đoạn lúc đổi key/scale, đặc biệt khi restore/handoff.
- License chưa allowed thì processBlock trả nguyên input, trong khi prepare vẫn báo delay 2 ms. Khi được phép xử lý, đường dry/bypass qua ring 2 ms. Đường thời gian khác nhau cần được giữ nhất quán khi chuyển quyền; đây là nhận xét từ wrapper, chưa kiểm tra implementation license worker vì không có file đó trong gói.
- Preset “Tune nhẹ 50%” cuối bảng thực tế đặt Mix 60%; vẫn trộn 40% giọng gốc. Không dùng preset parallel để đánh giá độ kéo nốt của riêng đường tune.

## Phần đã kiểm tra đạt

- 76 ca chạy thường: 52 ca chung + 12 Formant cao sample rate + 12 đối chứng Formant Off. Có nốt trầm/cao, rung, bước nốt, gliss, ngắt tiếng, yếu fundamental, input nhỏ, NaN/Inf, transpose, throat và freeze. Thêm 4 ca biên transpose riêng.
- 52 ca chung và 12 ca Formant cao sample rate chạy với **ASan + UBSan**, không có chẩn đoán sanitizer; leak detection tắt. Các trường hợp vọt đỉnh ở trên vẫn hữu hạn nên không phải lỗi bộ nhớ.
- **68.885 kiểm tra control đạt**: impulse/delay/reset, bypass sau khi ổn định, output hữu hạn sau audio NaN/Inf, tính lặp lại. 12.800 sample DSP không có C++ new/new[] qua bộ đếm thử; không đo mọi malloc hoặc toàn callback host.
- Neutral impulse khớp reference delay: 88/96/192/384 sample tại 44.1/48/96/192 kHz. Đó là độ trễ đường neutral/reference khoảng 2 ms. Pitch splicing có thể dùng mẫu cũ hơn theo chu kỳ, không được suy ra tổng độ trễ hát live bằng 2 ms cho mọi thiết lập.
- UI Input Type 0=Alto/Tenor, 1=Soprano được processor đổi sang index DSP ngược lại đúng cách; khác tên trong comment không phải lỗi routing.
- getRawParameterValue của JUCE 8 dùng StringRef/map, nên không kết luận các lần tra ID bằng chuỗi trong callback tự gây heap allocation.

## Phạm vi và cách đọc kết quả

Chỉ chạy **channel DSP C++ đúng phần thân hàm gốc**, không thay thuật toán. Header trong harness mở quyền đọc trạng thái để log, và cung cấp đúng ba giá trị enum NoteState; không mô phỏng quyền license hoặc dựng plugin crack. Reader gate probe chỉ giới hạn tìm file vào fixture trong workspace, giữ nguyên parser/enabled/force/poll.

Gói upload có 5 file Source, thiếu thư mục licensing/PluginLicense.h, licensing/LicensePanel.h và dự án build đầy đủ. Không chạy nguyên plugin, UI lifecycle, Cubase/Windows hoặc license worker ở lượt này; không khẳng định đã kiểm tra hết lỗi thêm/xóa VST trong DAW. Các timer CPU trong raw JSON có cả ghi file của harness, không dùng làm benchmark realtime.

Không có vocal A/B hiện tại hoặc binary UAD để đo đối chiếu. Các con số tần số/cents/peak là phép thử tổng hợp với đích biết trước, không phải chứng nhận độ tự nhiên hoặc tương đương hãng. Source gốc được giữ nguyên để người dùng quyết định bước sửa tiếp theo.

**Ưu tiên thực hiện:** bảo toàn Key/Scale đã restore → làm chắc tâm nốt mà giữ rung/luyến → phân biệt Freeze màn hình/khóa nốt → kiểm soát Formant transient và sample rate → mở biên transpose có kiểm tra causal → gom các request Learn/Auto-Key về đúng luồng.
