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
