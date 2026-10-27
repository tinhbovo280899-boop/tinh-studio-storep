# Bản mã nguồn VST đã sửa

Hai ZIP đã sửa theo yêu cầu của chủ repo. Bấm link bên dưới để tải về máy:

- [Tải PANTHEON II](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/PANTHEON_II_V1_0_0_FIXED_WIN_MAC_SOURCE.zip)
- [Tải LIQUID DELAYS II](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/LIQUID_DELAYS_II_V2_7_0_FIXED_WIN_MAC_SOURCE.zip)

Đây là mã nguồn kèm JUCE, chưa phải bộ cài hoặc plugin Windows/macOS đã biên dịch. Mỗi dự án đã build VST3 trên Linux và vượt qua 3/3 bộ kiểm thử; chưa kiểm tra build native Windows/macOS. Xem README.md, GHI_CHU_BAN_SUA.md và TEST_RESULTS bên trong mỗi ZIP để biết cách build, những sửa đổi và giới hạn còn lại.

PANTHEON II vẫn sử dụng một mạng Chamber. HDFilter của LIQUID DELAYS II chưa có xử lý riêng. Chưa xác nhận tương đương thuật toán của hãng.

Các ZIP giữ nguyên public key và product ID; không chứa PRIVATE_KEY.txt. SHA256SUMS.txt dùng để kiểm tra file tải về. Nhánh này chỉ chứa bản tải xuống; nhánh main được giữ nguyên.
