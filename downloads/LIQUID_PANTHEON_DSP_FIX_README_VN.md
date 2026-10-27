# Liquid Delays II 2.7.1 và Pantheon II 1.0.1 — DSP fixes

Hai ZIP mã nguồn đầy đủ, kèm JUCE, Tests, script build và owner Keygen. Windows giải nén vào thư mục mới, chạy `3_BUILD_VA_TAO_BO_CAI.bat`; macOS chạy `MAC_1_BUILD_VA_TAO_BO_CAI.command`. Đây là source để build, không phải bộ cài Windows/macOS biên dịch sẵn.

Sửa tank reverb tự dao động, đơn vị dB/Hz/ms, Pre-delay, DC Overdrive, LFO phụ thuộc buffer, mono/stereo, bypass, smoothing và tail. Sửa script build và công cụ sinh key cho đúng TAK1. Thử 7 ngày tự bắt đầu vẫn được giữ.

Đã build VST3 và chạy DSP/native processor tests trên Linux; ASan/UBSan và các kiểm tra mock Windows qua. Chưa native-test Windows/macOS, Inno Setup, pkgbuild hoặc Cubase. Chất tiếng/mức ra thay đổi, cần cân bằng lại Gain/Send khi nghe so bản cũ.

Khóa riêng của chủ đã loại khỏi ZIP; public key và product/brand ID cũ được giữ. Giữ khóa riêng từ bản gốc để tiếp tục cấp key cũ. Xem README và VALIDATION trong từng ZIP.
