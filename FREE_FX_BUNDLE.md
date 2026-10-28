# T Studio Free FX Bundle — một gói gồm bốn VST3

[Tải ZIP nguồn Free FX Bundle — khoảng 31 MB](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/TStudio_Free_FX_Bundle_WIN_MAC_SOURCE.zip)

Gồm T-Verb, T-RVerb, PANTHEON II và LIQUID Delays II. Đã gỡ activation/trial/mã máy/nhập key và các tool cấp key của dự án. Giữ DSP, preset và VST3 component/controller IDs của bản sửa trước. Dùng chung một cây JUCE. Các giấy phép thư viện/font vẫn đi kèm.

- Windows: chạy `3_BUILD_VA_TAO_BO_CAI_WINDOWS.bat`; cần Visual Studio C++, CMake 3.22+ và Inno Setup 6.3+.
- macOS: chạy `3_BUILD_VA_TAO_BO_CAI_MAC.command`; cần Xcode Command Line Tools và CMake 3.22+.
- Có lựa chọn chỉ Build hoặc chỉ Tạo bộ cài.

Đầu ra `_EXPORT/VST3` và `_INSTALLERS`: một `.exe` Windows x64 hoặc một `.pkg` macOS Universal chứa đủ bốn plugin. ZIP hiện tại là **mã nguồn và quy trình build/bộ cài**, chưa chứa `.exe`/`.pkg` đã biên dịch native.

Đã build mới bốn VST3 trên Linux, đạt 13 test plugin + 2 test gói chung, thử sạch luồng SDK JUCE dùng chung, kiểm tra checksum/đủ plugin/sai nền tảng và xác nhận không còn symbol activation trong binary. Script Windows đã parse cú pháp, script macOS đã kiểm tra bash. Chưa chạy native Windows/macOS, ISCC/pkgbuild hay notarization. Chi tiết trong README, docs và VALIDATION của ZIP.
