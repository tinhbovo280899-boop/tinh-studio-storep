# Kiểm tra T-Verb 1.3.0

## Kết quả

- **394 check DSP: 0 fail** — Audit 183, Extended 146, Upgrade 65.
- **394 check dưới ASan/UBSan: 0 fail**, bật leak detection; không có diagnostic trong các lượt thử.
- **65 check Upgrade dưới ThreadSanitizer: 0 fail**, không có cảnh báo data race trong lượt thử.
- **10 check state/UI/processor: 0 fail** — giữ thứ tự host parameter, lưu/đọc nút mới, nạp state cũ ở mức trung tính, preset mới, tín hiệu điều khiển tới engine qua processBlock thật, chặn NaN/Inf qua bypass và render ảnh UI.
- Tracking C++ `new/new[]`: **0** trong 50 callback warmup đã đo và **0** trong ca đổi endpoint tham số/Quality, xử lý block quá cỡ. Không diễn giải thành đo mọi loại cấp phát của toàn host/UI.
- **9 đối chiếu neutral với source gốc: 0 float khác nhau**, tại 44,1/48/96 kHz × 3 Quality. GCC O2, FP contraction off; finite input tĩnh, không đổi cấu hình trong khi chạy. Không phải chứng nhận null test mọi host/compiler hoặc mọi automation.
- Bảng TVerbTables sau tiền xử lý **trùng source gốc**. Dọn 142 comment có phép so riêng ở bước dọn; hash bước đó không đại diện toàn bộ source sau khi thêm các tính năng mới.
- VST3 **Linux x86_64** build và stage-install thành công. JUCE 8.0.12 bản source kèm gói đã configure/bootstrap juceaide thành công. Build native dùng SDK JUCE cùng phiên bản đã cài trong môi trường.

## Điều đã kiểm tra ở tính năng mới

Warmth 0% trả đúng tín hiệu trung tính trong ca tone; tăng Warmth làm tăng nhẹ quanh 320 Hz và hạ dải cao trong các sample rate nội bộ đã thử. Warmth không làm đổi nhánh Early khi Late bị mute. Early + Late khớp tổng hai nhánh khi các điều khiển trung tính.

Một frame NaN/+Inf/−Inf cho kết quả như thay frame đó bằng 0, ở cả replacing/accumulating trong ca thử, và vẫn có năng lượng tail sau đó. Tham số NaN/Inf giữ lại giá trị hợp lệ trước. Những ca này không giả lập mọi dạng hỏng trạng thái hoặc memory corruption.

Các bài chuyển cấu hình xác nhận: mạng mới thực sự được nhận, đoạn đầu fade vẫn mang tail cũ, yêu cầu mới nhất được áp dụng khi đổi tiếp, và có thể stop/prepare/destroy khi đang có công việc nền. Render offline cùng lịch block/event lặp lại cho cùng output trong bài thử. Handoff dùng acquire/release; mỗi tank chỉ có một chủ sở hữu tại một thời điểm.

## Thời gian callback — đo tham khảo trên cloud

Probe chỉ đo lõi DSP, 64 mẫu/callback, 48 kHz, GCC O2. Có pacing theo thời gian và đổi Room Type/Quality trong chuỗi callback. Budget danh nghĩa là 1,333 ms. Một lượt đo:

| Số đo | Source gốc | Nâng cấp |
|---|---:|---:|
| p95 callback ở khoảng steady | 0.092 ms | 0.102 ms |
| p95 ở khoảng có chuyển cấu hình | 0.073 ms | 0.140 ms |
| Callback lớn nhất quan sát | 2.359 ms | 1.602 ms |
| Max RSS của tiến trình probe | 31.1 MiB | 60.6 MiB |

Bản mới có thêm mạng dự phòng và worker, nên bộ nhớ tăng; trong fade 150 ms có hai mạng xử lý. Main callback live không chuẩn bị/xóa mạng dự phòng, không khóa mutex hoặc chờ worker ở đường điều khiển cấu hình. Chế độ offline được phép chờ để giữ kết quả lặp lại.

**Đây không phải hard real-time pass ở buffer 64:** giá trị max quan sát vẫn vượt 1,333 ms trong môi trường cloud chia sẻ. Đồng hồ bao gồm ảnh hưởng scheduling; các số này không chứng minh nguyên nhân mọi outlier hoặc dự báo Windows/ASIO. Chưa thử Cubase 15, soundcard và chuỗi plugin của anh. Không cam kết đổi mọi tham số đều tuyệt đối không gây tiếng chuyển hoặc không bao giờ dropout.

## Giới hạn giao hàng

- ZIP là **source + SDK + script build**, không kèm binary Windows/macOS hoặc bộ cài đã ký.
- Chưa build/chạy Windows/macOS, chưa có A/B vocal thật, chưa chấm nghe độ hay với bản phối của anh và chưa render plugin hãng.
- UI được chụp từ editor native thật, với các dependency shared bổ sung từ gói FX đã có; panel vẽ bằng mã để project độc lập khi input thiếu SkinData/HwKit mới.
- Các assertion audio của processor không vô hiệu hóa license. Phần kiểm chứng âm học chi tiết dùng engine độc lập; không coi badge chưa kích hoạt trong ảnh là một lượt host đã được unlock.
- Một worker cho mỗi instance, dừng/join ở lifecycle; không dùng thread detached hoặc cưỡng bức kill. ThreadSanitizer kiểm tra các ca đã chạy, không chứng minh mọi lịch thread có thể xảy ra.

## Tái hiện

Build/test lõi không cần GUI SDK:

```sh
cmake -S . -B build-core -DTVERB_BUILD_PLUGIN=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build-core --parallel 2
ctest --test-dir build-core --output-on-failure
```

Để bật probe state/UI khi có JUCE GUI dependencies, thêm `-DTVERB_BUILD_PROCESSOR_QA=ON`, build `TVerb_ProcessorQA`. Probe có thể nhận một đường dẫn PNG tuyệt đối làm đối số. Các script build ở root build VST3 bằng JUCE source kèm theo; trên macOS có thêm AU.

Dữ liệu: `validation-summary.json`, `legacy-neutral-comparison.json`, `comments-cleanup.json`, hai `callback-profile*.json`, và các log kèm thư mục này.
