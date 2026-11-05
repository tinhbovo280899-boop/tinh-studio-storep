# Bản vá LiveNT – 4 yêu cầu

[Tải ZIP bản vá](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/livent-patches/patches/LiveNT_PATCH_4_YEU_CAU.zip)

1. Vang: bỏ chỉnh/xử lý không gian có sẵn; giữ mức Send và chỉnh hiệu ứng trong VST.
2. Míc: chuột phải chỉ còn Giảm ồn.
3. Vang dài, vang ngắn, Delay: hiện mục thêm Send trong Inspector F2.
4. Nút đóng app: nền đỏ thường trực, cả giao diện đầy đủ và thu gọn.

Giải nén vào thư mục gốc của đúng bản dự án đã gửi, chép đè 8 file trong `Source`,
rồi build lại với bộ thư viện/SDK đầy đủ. ZIP có hướng dẫn, diff và SHA-256 từng file.

Đây là bản vá source, chưa phải file EXE hoặc bộ cài. 55 kiểm tra độc lập đã đạt;
chưa build/chạy ứng dụng Windows hoàn chỉnh vì đầu vào còn thiếu bản JUCE đã vá,
module CMake, SDK và bộ kiểm thử/tài nguyên đi kèm.
