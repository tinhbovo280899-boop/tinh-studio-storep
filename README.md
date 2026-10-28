# T Studio Free FX Bundle — một gói gồm bốn VST3

[Tải ZIP nguồn Free FX Bundle — khoảng 31 MB](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/TStudio_Free_FX_Bundle_WIN_MAC_SOURCE.zip)

Gồm T-Verb, T-RVerb, PANTHEON II và LIQUID Delays II. Đã gỡ activation/trial/mã máy/nhập key và các tool cấp key của dự án. Giữ DSP, preset và VST3 component/controller IDs của bản sửa trước. Dùng chung một cây JUCE. Các giấy phép thư viện/font vẫn đi kèm.

- Windows: chạy `3_BUILD_VA_TAO_BO_CAI_WINDOWS.bat`; cần Visual Studio C++, CMake 3.22+ và Inno Setup 6.3+.
- macOS: chạy `3_BUILD_VA_TAO_BO_CAI_MAC.command`; cần Xcode Command Line Tools và CMake 3.22+.
- Có lựa chọn chỉ Build hoặc chỉ Tạo bộ cài.

Đầu ra `_EXPORT/VST3` và `_INSTALLERS`: một `.exe` Windows x64 hoặc một `.pkg` macOS Universal chứa đủ bốn plugin. ZIP hiện tại là **mã nguồn và quy trình build/bộ cài**, chưa chứa `.exe`/`.pkg` đã biên dịch native.

Đã build mới bốn VST3 trên Linux, đạt 13 test plugin + 2 test gói chung, thử sạch luồng SDK JUCE dùng chung, kiểm tra checksum/đủ plugin/sai nền tảng và xác nhận không còn symbol activation trong binary. Script Windows đã parse cú pháp, script macOS đã kiểm tra bash. Chưa chạy native Windows/macOS, ISCC/pkgbuild hay notarization. Chi tiết trong README, docs và VALIDATION của ZIP.

---

# Đợt sửa FX Send — T-Verb, vang ngắn và Delay

Giữ chất tiếng hiện tại theo lựa chọn của chủ dự án; sửa lỗi đã xác định và thêm preset FX Send riêng.

- [T-RVerb đã sửa](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/T-RVerb_V1_0_0_FIXED_WIN_MAC_SOURCE.zip)
- [T-Verb — preset mềm nhẹ FX Send](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/T-Verb_V1_0_0_FX_SEND_FIXED_WIN_MAC_SOURCE.zip)
- [PANTHEON II — preset vang ngắn FX Send](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/PANTHEON_II_V1_0_0_FX_SEND_FIXED_WIN_MAC_SOURCE.zip)
- [LIQUID Delay — đổi Time êm hơn + FX Send](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/LIQUID_DELAYS_II_V2_7_0_FX_SEND_FIXED_WIN_MAC_SOURCE.zip)

Các ZIP là **mã nguồn**, chưa phải plugin/bộ cài Windows hoặc macOS. Cả bốn đã build VST3 trên Linux và đạt các bộ test: T-RVerb 2/2, T-Verb 7/7 (320 kiểm tra engine), PANTHEON II 3/3, LIQUID Delay 4/4. Chưa build/test native Windows/macOS hoặc nghe đối chiếu với vocal thật.

Xem GHI_CHU_DOT_SUA_FX_SEND.md và TEST_RESULTS_BATCH trong từng ZIP để biết cách build, tên preset mới và các giới hạn. T-RVerb sửa NaN/Inf, signed predelay, mono→stereo, state/preset và tail; Delay thêm crossfade 20 ms khi đổi Time/BPM. Engine T-Verb/PANTHEON không đổi, preset mới thêm cuối danh sách. Giữ plugin ID và public license config; không có private key trong ZIP.

Với FX Send: chọn preset mới có hậu tố FX Send; return wet-only. Ở Delay, InLoop Off và cả hai delay On để tránh passthrough trực tiếp vào return.

---

# Bản mã nguồn VST đã sửa

Các ZIP đã sửa theo yêu cầu của chủ repo. Bấm link bên dưới để tải về máy:

- [Tải PANTHEON II](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/PANTHEON_II_V1_0_0_FIXED_WIN_MAC_SOURCE.zip)
- [Tải LIQUID DELAYS II](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/LIQUID_DELAYS_II_V2_7_0_FIXED_WIN_MAC_SOURCE.zip)
- [Tải 1176 LN 3D](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/1176_LN_3D_V1_0_0_FIXED_WIN_MAC_SOURCE.zip)
- [Tải T-Limiter](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/T-Limiter_V1_0_0_FIXED_WIN_MAC_SOURCE.zip)
- [Tải T-AutoTune LIVE 8.1.0](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/T-AutoTune_LIVE_V8_1_0_FIXED_WIN_MAC_SOURCE.zip)

Đây là mã nguồn, chưa phải bộ cài hoặc plugin Windows/macOS đã biên dịch. Cả năm dự án đã build VST3 trên Linux; chưa kiểm tra build native Windows/macOS. PANTHEON II và LIQUID DELAYS II vượt qua 3/3 bộ kiểm thử mỗi dự án; 1176 vượt qua 2/2 bộ kiểm thử. T-Limiter vượt qua 97 trường hợp kiểm thử engine, phép đo peak độc lập và kiểm thử wrapper. Xem README.md, GHI_CHU_BAN_SUA.md và TEST_RESULTS bên trong mỗi ZIP để biết cách build, những sửa đổi và giới hạn còn lại.

PANTHEON II vẫn sử dụng một mạng Chamber. HDFilter của LIQUID DELAYS II chưa có xử lý riêng. Chưa xác nhận tương đương thuật toán của hãng.

Các ZIP giữ nguyên public key và product ID; không chứa PRIVATE_KEY.txt. SHA256SUMS.txt dùng để kiểm tra file tải về. Nhánh này chỉ chứa bản tải xuống; nhánh main được giữ nguyên.

## Auto-Tune LIVE

Giao diện bốn nút chính và menu Settings hoạt động. Bản live loại nhánh LPC Formant/Throat cũ gây xung, cố định Classic Off, bỏ GRAPH và nhóm tạo rung giả/Transpose. Giữ Retune, Flex-Tune, Humanize, Natural Vibrato và dữ liệu tham số cũ để đọc state. Tính năng Formant không có trong bản live này; preset dùng các chức năng bị bỏ sẽ đổi âm thanh tương ứng. Đã build VST3 trên Linux, vượt qua 2/2 bộ kiểm thử; bộ DSP không cần GUI cũng vượt 1/1. Chưa kiểm tra native Windows/macOS hoặc tối ưu thêm độ trễ. Chi tiết trong GHI_CHU_BAN_LIVE.md bên trong ZIP.

![Giao diện T-AutoTune LIVE](T-AutoTune_LIVE_preview.png)

## Gửi tệp lớn để phân tích

[Tải công cụ chia file 20 MB cho Windows](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/CONG_CU_CHIA_FILE_20MB.zip). Giải nén, kéo WAV/VST3/ZIP lên CHIA_FILE_GUI_EM.bat, rồi gửi toàn bộ ZIP và manifest.json trong thư mục GUI_EM_... Công cụ chia theo byte để người nhận ghép lại đúng file ban đầu; không tự tải dữ liệu lên mạng. Chi tiết kiểm thử và giới hạn nền tảng trong ZIP.


## T Studio 6 VST Bundle 1.0.1

[Tải gói sáu VST đã sửa](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/TStudio_6_VST_Bundle_V1_0_1_FIXED_WIN_MAC_SOURCE.zip).

Gồm Kaleidoscopes, T-API550, T-ConsoleEQ, T-HDelay, T-NS1 và T-Vitamin, vẫn là sáu VST3 riêng trong một quy trình tạo bộ cài. Sửa DC của CLA-2A, buffer MetaFlanger, khởi tạo/reset/tail HDelay, cấp phát audio callback Vitamin và lưu preset/profile NS1. Giữ mức khử Gentle và hành vi Solo Vitamin; chi tiết tiêu chí kiểm thử nằm trong ZIP.

Đã build cả sáu VST3 trên Linux, qua 12/12 nhóm CTest, 135 kiểm tra engine với sanitizer và 36 ca nạp/xử lý native mono/stereo. Mã nhận dạng plugin và khai báo tham số được giữ nguyên. Đây là mã nguồn miễn kích hoạt kèm script tạo bộ cài, chưa có EXE/PKG Windows/macOS đã biên dịch hoặc kiểm tra native. Chạy 1_BUILD_VA_TAO_BO_CAI_WINDOWS.bat hoặc 1_BUILD_VA_TAO_BO_CAI_MAC.command trên máy đích. Xem README, GHI_CHU_BAN_SUA_1_0_1.md và VALIDATION.txt trong gói.


## T Studio 5 VST Bundle 1.0.1

[Tải gói năm VST đã sửa](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/TStudio_5_VST_Bundle_V1_0_1_FIXED_WIN_MAC_SOURCE.zip).

Gồm T-BE, T-Comp, T-DeNoise, T-EQ và T-Pitch. Sửa state/profile, đổi sample rate, Learn im lặng và reset DeNoise; reset Pitch; bypass/reset của Comp/EQ; đọc key/scale đồng bộ cho BE. Giữ mã nhận dạng và khai báo tham số của cả năm.

Đã build 5/5 VST3 trên Linux, qua 10/10 nhóm kiểm thử (giữ nguyên tiêu chí test gốc), 30 ca VST3 native mono/stereo, và kiểm tra DeNoise với ASAN/UBSAN. Đây là ZIP mã nguồn kèm script build/test/tạo bộ cài; Windows/macOS native, EXE/PKG và DAW thật chưa được kiểm tra. Chạy 1_BUILD_VA_TAO_BO_CAI_WINDOWS.bat hoặc bản .command cho Mac. Profile DeNoise cũ không lưu sample rate nên cần Learn lại nếu đổi sample rate. Xem README, GHI_CHU_BAN_SUA_1_0_1.md, VALIDATION.txt và báo cáo trong ZIP.
