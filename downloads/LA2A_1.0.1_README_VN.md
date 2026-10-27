# LA-2A 3D 1.0.1 — bản sửa DSP

[Tải ZIP mã nguồn đầy đủ](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/uad-la2a-dsp-fixed-1.0.1/downloads/LA2A_1.0.1_DSP_Fixed_Source_WIN_MAC.zip)

Giải nén vào thư mục mới. Windows chạy `4_BUILD_VA_TAO_BO_CAI.bat`; macOS chạy `MAC_4_BUILD_VA_TAO_BO_CAI.command`. Gói có JUCE, mã nguồn, script build và tài liệu; không phải bộ cài Windows/macOS đã biên dịch sẵn.

Sửa Power/bypass, đồng hồ phụ thuộc buffer, tốc độ làm mượt theo sample rate, saturation khi quá tải; thêm oversampling 4× và khai báo latency 32 mẫu. Sửa cả lỗi cú pháp vòng lặp trong script Windows.

Đã build VST3/Standalone Linux và qua 2 suite CTest, DSP ASan/UBSan, 28 kiểm tra mô phỏng script Windows. Chưa build/chạy native trên Windows/macOS hoặc kiểm tra trong Cubase. Chưa xác nhận độ giống chất âm với UAD gốc. Xem CHANGELOG và VALIDATION trong ZIP.
