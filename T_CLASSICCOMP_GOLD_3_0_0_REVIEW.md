# T-ClassicComp → T-ClassicComp Gold 3.0.0

Mục tiêu người dùng xác nhận: **cả chất nén và bố cục mặt vàng trong ảnh T-RackS CS Classic Comp**. Đã tạo một VST3 T Studio riêng với 7 núm chính và đồng hồ kim. Dùng tên/ID riêng để so A/B, không ghi đè T-ClassicComp cũ.

## Kết luận kiểm tra bản gửi vào

Gói gốc có mô hình C++ compressor, bộ khung plugin và giao diện nhiều chức năng. Có thể build các bài thử DSP gốc và chúng đều đạt; điều này không chứng minh model giống hãng 100%.

1. **Classic trong plugin chưa chạy nguyên đường feedback M/S của model.** NTCompDSP gọi setter của tr5 và mượn stepEnvelopeM/S nhưng nén trên detector L/R, thêm hệ số feedback `0.65+0.35*gain`, rồi áp dụng đường soft-knee/threshold riêng. Ratio vẫn có tác dụng qua công thức này; không phải núm hoàn toàn vô hiệu, nhưng đường đi khác phương thức processSampleStereo của model.
2. **Hai API trong cùng model lệch 6,0206 dB.** Với tone mono trùng L/R, tại đầu vào −50, −18 và −6 dBFS, processStereo lớn hơn processSampleStereo đúng 6,0206 dB. Hàm block/mono thiếu cùng mức chuẩn hóa 0.5. Đây là lỗi nhất quán nội bộ được đo, không phải phép so gain với plugin hãng.
3. **Auto Release ở Classic của wrapper cũ không có tác dụng.** Với đoạn tone thay đổi mức, bật/tắt cho output giống từng bit. releaseCoeff điều khiển Auto Release chỉ được dùng ở nhánh Clean/Opto. Bản Gold bỏ nút này để không dựng chức năng giả.
4. Các lớp mặc định như saturation Drive 15%, DC block, threshold/knee, makeup/Auto Gain, M/S width sau nén có thể tạo khác biệt với model thuần. Ví dụ kiểm soát Drive màu=0, Input/Output=0, tone −6 dBFS: wrapper hybrid lớn hơn phương thức sample model khoảng **3,91 dB**. Không nên dùng việc to/nhỏ để kết luận giống chất nén hơn.
5. Giao diện gốc là panel đồ thị hiện đại 940×680; chưa phải panel vàng và kim VU trong ảnh.

## Thay đổi trong Gold

- Chạy trực tiếp đường feedback M/S processSampleStereo của model gửi vào. Không chạy hybrid threshold/knee hoặc các tầng Air/Drive màu/Auto Gain cũ.
- Đồng nhất processStereo/processMono với xử lý sample đã chuẩn hóa. Mono gửi cùng tín hiệu vào hai nhánh để cùng quy ước mức với stereo dual-mono.
- Giải nghiệm cực RC theo double và công thức tránh phép trừ hai số gần nhau; giữ họ model RC của mã gửi vào.
- Sanitize audio và tham số không hữu hạn trước khi chúng đi vào filter/envelope. Chặn dải điều khiển Gold theo model gửi vào.
- Kim GR dùng hệ số giảm gain thực tế, không dùng hiệu hai giá trị envelope. Bypass crossfade 5 ms, không lookahead cố định.
- Giao diện T Studio tự vẽ: vật liệu vàng, 7 núm, meter, đèn On, Reset, preset và A/B/C/D. Không nhúng ảnh/logo IK Multimedia.
- 3 preset, trong đó Photo Reference lấy 76 Hz / 25 ms / 250 ms / 0 dB Stereo / Ratio 3.10 / Input −6 dB / Output +3 dB từ các số nhìn thấy trong ảnh. Đây không phải khẳng định về preset factory của hãng.
- A/B/C/D lưu trong state của DAW, dùng khóa cho thao tác lưu/chuyển bank, không lấy khóa này trong processBlock. Tham số audio đọc qua atomic; không chạy kiểm tra bản quyền, network hoặc ghi file trong xử lý audio/UI teardown.
- Bản cũ và toàn bộ chức năng nguyên gói được giữ nguyên trong Reference/ORIGINAL_UPLOAD.zip. Gold có ID mới; không nạp state cũ hoặc nhận là plugin IK.

## Đo và kiểm tra

- **299.921 phép kiểm tra, không lỗi**: core mono/stereo/sample/block consistency qua 8/44.1/48/96/192 kHz; 6 vòng processor, 3 editor; preset, bank/state, typed units, UI slider/reset, bypass và input NaN. 6 vòng ×100 block không có C++ new/new[] được bộ đếm chặn. Không khẳng định đã chặn mọi allocator của hệ thống.
- ASan + UBSan đạt với core/processor/editor/tests của dự án. Runtime JUCE QA được nhập sẵn không instrument; leak detection tắt.
- VST3 Linux thật được build với JUCE SDK đầy đủ và nạp qua factory/component/controller ở **15 cấu hình bus/rate/automation**: mono/stereo/dual-mono, 44.1/48/96 kHz, block 64/128 thay đổi, có đưa Input Drive +6 dB qua host queue. Latency báo host=0 sample; không tính buffer/soundcard/chuỗi FX.
- Output +3 dB đo được **+3,000 dB**. Stereo Enhancement từ −5 lên +5 ở tín hiệu Side nhỏ đo khoảng **+9,991 dB**.
- Ratio từ 1.5 lên 4.68, tone −6 dBFS/1 kHz: GR tăng từ khoảng **3,00 lên 8,71 dB**.
- Tone 50 Hz/−6 dBFS: tăng SC HPF 32→218 Hz làm GR giảm từ khoảng **5,46 xuống 1,89 dB**; đúng chức năng detector HPF, không phải lọc trực tiếp vocal.
- Attack 12 ms cho GR sau 20 ms của up-step cao hơn Attack 82 ms; Release 1400 ms còn giữ GR sau down-step nhiều hơn Release 29 ms. Số đo/biểu đồ đi kèm.
- Script xuất Windows qua **9 ca contract mock** trên PowerShell 7/Linux, gồm lỗi configure/build, CMake cũ, thiếu/sai PE x64, xuất lại và giữ bản cũ khi thất bại. Không có PE fixture giả trong gói giao. Đây không phải compile Windows.
- Script Mac chỉ kiểm tra `bash -n`; chưa compile/lipo/codesign trên Mac.

## Điều chưa thể xác nhận

Không có binary hoặc bản thu A/B plugin hãng trong gói này. Chỉ có ảnh, mã C++ và các tuyên bố/địa chỉ binary trong tài liệu gốc. Chúng không đủ chứng minh “bit-accurate” hoặc phục hồi 100% thuật toán gốc. Gold đưa đường nén của **model được gửi vào** vào hoạt động nhất quán hơn; chưa được hiệu chỉnh transfer curve, attack/release, harmonic spectrum, stereo behavior hoặc meter ballistics theo plugin IK thực tế.

Giao diện lấy bố cục và tông vàng làm mục tiêu, dùng thương hiệu/hình vẽ riêng, không nhận là bản sao pixel 1:1. Bản Gold hướng tới workflow gọn của ảnh, nên không mang toàn bộ tính năng studio của bản cũ.

Chưa build Windows/macOS hay chạy Cubase 15 trong lượt này. Cần build trên máy anh và so âm thanh cùng nguồn/cùng mức đầu ra để tiếp tục hiệu chỉnh mức giống hãng. ZIP là source + script xuất, chưa phải binary Windows hoặc installer.
