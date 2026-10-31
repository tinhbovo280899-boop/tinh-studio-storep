# Báo cáo T Studio — 31 VST

Ngày: 31/10/2026. Đối tượng sử dụng: hát live, Cubase 15 trên Windows.

**Kết luận:** đã tái hiện lỗi ở 6 plugin, chuẩn bị bản vá 5 file nguồn và thử lại thành công. T-Compander cũng được build/thử lại vì dùng chung file với ba plugin có lỗi. Các plugin còn lại chưa phát hiện lỗi trong phạm vi kiểm tra dưới đây; điều đó không có nghĩa đã chứng minh chúng không còn lỗi.

## 1. Lỗi đã tái hiện và bản vá

| Plugin | Hiện tượng và điều kiện | Bản vá và kết quả |
|---|---|---|
| T-EQ | Callback menu giữ con trỏ tham số thô. Khi trả kết quả menu sau khi editor/processor đã bị xóa, AddressSanitizer báo heap-use-after-free. Đây là lỗi bộ nhớ có thể liên quan thao tác xóa/thay plugin. | Giữ SafePointer tới editor, chỉ lấy tham số lại khi editor còn sống. Callback khi đang mở vẫn hoạt động; cùng callback trễ sau hủy không còn lỗi ASan. |
| T-Verb | Chèn một mẫu NaN rồi phát im lặng: mạng hồi tiếp tiếp tục sinh NaN ở cả 48/96 kHz; gọi reset trong phép thử gốc vẫn chưa phục hồi. | Thay riêng mẫu đầu vào không hữu hạn bằng 0 trước DSP. Tín hiệu hữu hạn, kể cả headroom float trên 0 dBFS, được giữ nguyên. Thử lại không còn mẫu không hữu hạn. |
| T-Mod, T-Opto, T-TapeFlanger | Một mẫu NaN làm trạng thái lọc/delay bị nhiễm và tiếp tục xuất NaN sau đó. | Chặn NaN/Inf trước khi đi vào DSP trong file T-ModSplit dùng chung. Cả ba và T-Compander đều vượt qua thử lại. |
| T-Limiter | Plugin báo 258/498 mẫu trễ ở 48/96 kHz, nhưng processBlockBypassed của nguồn gốc đi thẳng ở mẫu 0. JUCE dùng nhánh này cho bypass VST3 tổng hợp vì plugin không có getBypassParameter riêng. | Dùng lại dry delay đã cấp phát, chuyển 20 ms, giữ DSP đang chạy. Sau sửa impulse bypass nằm đúng 258/498 mẫu. Không thêm ID automation, không đổi chính sách cấp phép. |

NaN là giá trị float không hợp lệ. Phép thử chèn NaN kiểm tra khả năng chống lỗi từ plugin phía trước, **không chứng minh giọng hát bình thường tự làm plugin sinh NaN**. Việc vá đầu vào ngăn trạng thái mới bị nhiễm; không phải cơ chế sửa tùy ý mọi lỗi số học bên trong.

Phép thử T-EQ bắt đúng lambda nguồn tại showMenuAsync, trả kết quả menu khác 0 sau khi đã hủy editor/processor thật. Đó là mô phỏng callback trễ có kiểm soát, chưa phải tái hiện toàn bộ thao tác Cubase. Translation unit của callback bật ASan; runtime JUCE dùng chung không bật ASan, kiểm tra leak tắt. Không suy rộng thành ASan toàn bộ 31 plugin.

T-Limiter có xử lý block lớn hơn kích thước host đã thông báo bằng cách chia nhỏ. Bản vá giữ cách đó và bộ nhớ dry đã cấp phát sẵn. T-DeNoise/T-Pitch có bypass parameter riêng; kết quả gọi trực tiếp processBlockBypassed trong harness không đại diện tuyến bypass VST3 của hai plugin đó, nên không gắn lỗi T-Limiter cho chúng.

## 2. Kiểm tra đã chạy độc lập

| Nhóm kiểm tra | Kết quả | Phạm vi |
|---|---|---|
| Biên dịch nguồn | 31 processor/editor và các chương trình QA biên dịch được | Linux, JUCE 8.0.12; adapter CMake QA liên kết runtime JUCE đã build, không sinh DLL VST3 Windows |
| Vòng đời nguồn gốc | 186 lần tạo/hủy processor; 124 lần mở/đóng editor; 124 lần hủy dưới MessageManagerLock, không treo/crash | 6 instance/plugin, có scan không editor, lưu/khôi phục state, phát audio nền khi đóng editor |
| Vòng đời sau vá | Thêm 42 lần tạo/hủy processor, 28 editor, 28 lần hủy dưới UI lock | 7 plugin chịu tác động |
| DSP chung 31 plugin | Không có đầu ra không hữu hạn với tín hiệu hữu hạn và các endpoint tham số đã thử | 48/96 kHz, chuẩn bị block tối đa 512 rồi chạy block 1/127/511; tone, noise, stereo/đảo cực |
| Cấp phát C++ trong xử lý | Không quan sát thấy C++ new trong các callback ổn định và endpoint sweep đã đo | Không bao phủ mọi malloc, system call, tổ hợp tham số hoặc đường chất lượng cần prepare lại |
| Chống mẫu lỗi | Nguồn gốc phát hiện 4 plugin nêu trên; sau vá 7 plugin đều hữu hạn ở normal/recovery/reset | Chèn một NaN, theo dõi khoảng 1 giây, rồi reset và chạy tiếp |
| Regression DSP chuyên biệt | 6 chương trình đều báo 0 failures | T-Lex224, T-PultecEQ, T-PurePlate, T-VoxBox, T-D82 Sonic, T-Verb 2; chạy lại mã test đi kèm trong kho QA riêng |
| License chung | 68 kiểm tra đạt | RSA/SHA-256, đúng/sai product-brand-machine, khóa vĩnh viễn/có hạn/legacy, trial 7 ngày, sửa record, đổi đồng hồ |
| CapKey 1.7.3 | 19 kiểm tra logic nguồn đạt | Khởi tạo/tái sử dụng khóa thử, ký/xác minh, giới hạn ngày, request sai, giải mã fixture nhúng, blob hỏng, không ghi đè kho khóa thiếu một phần |
| AutoTune 1.7.3 | 89 ca DSP độc lập đạt dưới ASan/UBSan; 3.111.269 phép kiểm | Nguồn DSP đúng overlay 1.7.3 trên nền 1.7.2, nhiều sample rate, neutral delay, retune, vibrato controller, stress; không phải đánh giá nghe vocal thật |
| Script | 30 file shell đạt bash -n; 19 file PowerShell đạt parser chính thức PowerShell 7.5.4 | Chỉ phân tích cú pháp trên Linux, không chạy build/installer/branding script của đầu vào |

70 chương trình QA baseline đều kết thúc bình thường, nhưng harness Signal ghi các lỗi robustness vào JSON thay vì trả exit khác 0. Vì vậy **không tính 70 exit code 0 thành 70 bài kiểm không có lỗi**. 14 chương trình kiểm lại sau vá đều kết thúc bình thường và dữ liệu Signal không còn lỗi nonfinite đã nêu. Bốn ca callback T-EQ được ghi riêng; ca nguồn gốc trả muộn chủ động thất bại ASan.

Một số regression stress cho gain lớn tạo đỉnh float trên 0 dBFS. Việc test không thất bại không biến EQ/reverb thành limiter hay bảo đảm mọi preset/tổ hợp cực đoan không clip ở đầu ra soundcard. Thời gian CPU ghi trong log là từ máy Linux dùng chung, không phải benchmark Cubase Windows.

## 3. Độ trễ cần lưu ý khi hát live

Các con số dưới đây là độ trễ plugin báo cho host trong cấu hình harness, không phải tổng độ trễ mic–tai nghe. Chưa cộng ASIO input/output buffer, routing, phần cứng hoặc plugin khác trong chuỗi.

| Plugin | 48 kHz | 96 kHz | Ý nghĩa |
|---|---|---|---|
| T-AutoTune | 96 mẫu = 2 ms | 192 mẫu = 2 ms | Độ trễ danh định; đường giữ nguyên cao độ xác nhận ở DSP neutral. Không hứa mọi tỷ lệ dịch cao độ có cùng read age hoặc 0 ms. |
| T-Pitch | 5.760 mẫu = 120 ms | 11.520 mẫu = 120 ms | Cấu hình Music mặc định trong phép thử. Nguồn có chế độ Live khoảng 50 ms; vẫn đáng kể cho monitoring. Đây là đánh đổi thiết kế, không kết luận lỗi thuật toán. |
| T-DeNoise | 512 mẫu = 10,67 ms | 1.024 mẫu = 10,67 ms | Nguồn có HQ với độ trễ cao hơn, khoảng 42,67 ms ở 48 kHz. |
| T-Limiter | 258 mẫu = 5,375 ms | 498 mẫu = 5,188 ms | Theo lookahead/oversampling đã prepare. Bypass sau vá giữ cùng độ trễ. |
| T-LA-2A | 32 mẫu = 0,667 ms | 32 mẫu = 0,333 ms | Cấu hình harness. |
| T-DeEsser, T-RVox | 64 mẫu = 1,333 ms | 128 mẫu = 1,333 ms | Cấu hình harness. |

Các plugin còn lại báo 0 trong cấu hình đã thử. Với reverb/delay, predelay và thời gian tiếng lặp thuộc hiệu ứng, không thể hiểu host latency 0 là phần wet xuất hiện tức thì.

T-Pitch/T-DeNoise trong tuyến nghe live có thể làm cảm giác hát chậm hơn dù AutoTune báo 2 ms. Routing nghe đồng thời tín hiệu direct từ soundcard và tín hiệu qua Cubase cũng có thể gây giao thoa/âm sắc cứng khi hai đường lệch thời gian hoặc cao độ; trạng thái hai nút monitor chưa tự chứng minh có hai đường âm thanh thật, cần xác nhận routing trên máy anh.

## 4. Đồng nhất, license và VocalPack 1.7.3

- Đủ 31 sản phẩm trong 6 thư mục dự án: Compressor 2, Vocal FX 4, Vintage 4, Plugins 15, FX 4, VocalPack 2. Hai ZIP chứa nhiều thư mục dự án; không đếm số thư mục ZIP thành số plugin.
- Metadata CMake đã resolve không trùng tên sản phẩm, bundle ID hoặc cặp manufacturer/product code. Đây là kiểm tra định danh ở nguồn; chưa quét FUID của các binary đang cài trên máy anh. Không đổi các ID cũ để tránh hỏng project/preset đã lưu.
- LicenseCodec.cpp/.h, LicenseManager.cpp/.h và PluginLicense.h giống nhau giữa 6 dự án. Product cấp phép chung là TVOCALPACK, brand TSTUDIO, trial 7 ngày; một pack key dùng chung là chủ đích của nguồn hiện tại.
- Bộ lưu trial hiện có lastPersistAttemptMs, retry sau lỗi 60 giây, refresh định kỳ 10 phút. Không thấy lại kiểu thử ghi 20 lần/giây từng sửa ở phiên bản cũ. Kiểm thử license dùng store QA riêng, không sửa/xóa trial hay khóa thật của anh.
- Sáu file config/public_key.txt trong ZIP nguồn đều đang trống (một newline). Trước khi build bản bán hàng cần giữ/điền đúng public key của hệ cấp phép anh đang dùng theo workflow dự án. Không thể suy từ các file nguồn trống rằng binary anh đang dùng cũng có key trống. Không sinh hay thay private key cho anh.
- **Còn rủi ro cần điều tra:** shared/licensing/PluginLicense.h gọi stopThread(30000) khi hủy dịch vụ license. Nếu worker kẹt ở I/O, thread giao diện có thể phải đợi tối đa 30 giây; JUCE có đường force-kill khi quá hạn, dù comment của dự án nói không force. Các vòng đời bình thường đã thử đều đóng nhanh; chưa tái hiện I/O/Registry bị kẹt, chưa kết luận đây là nguyên nhân đơ Cubase của anh. Bản vá không thay timeout thành chờ vô hạn hoặc giải phóng worker còn đang chạy.
- ZIP 1.7.3 capkey_file_sua chỉ là overlay 9 file, không đủ dự án độc lập. Phần kiểm tra ghép trên nguồn 1.7.2 Live đã gửi trước. DSP chỉ đổi quy tắc hysteresis khi chọn nốt đích; giữ các sửa delay/timebase/vibrato/Formant mặc định tắt của 1.7.2. Hysteresis có thể giảm đổi qua lại ở ranh giới nhưng cũng có thể giữ nốt lâu hơn; chưa xác minh các con số đo vocal thật trong changelog đi kèm.
- CapKey cho phép đóng gói private key vào công cụ dành cho chủ sở hữu bằng XOR. XOR không giữ bí mật khóa nếu người nhận có file thực thi; công cụ đã nhúng khóa chỉ dành cho anh. Tùy chọn build mặc định tắt. Các phép thử dùng khóa tạm, không dùng khóa của anh; ZIP báo cáo/bản vá này không chứa private key hoặc CapKey đã nhúng khóa.
- README tiêu đề VocalPack còn ghi 1.7.2 trong khi branding overlay là 1.7.3: khác biệt tài liệu, không phải lỗi DSP. Không tự đổi phiên bản/ID của 31 sản phẩm trong bản vá nhỏ này.

## 5. Giới hạn và xác nhận trên Windows

Nguồn processor/editor được biên dịch thật trong môi trường Linux với runtime JUCE dùng chung. Adapter QA thay phần tìm SDK và cấp đường dẫn kho license thử; nó không chạy toàn bộ pipeline native trong CMake, không tạo VST3 Windows/macOS hay bộ cài. Các thay đổi adapter và đường dẫn QA không nằm trong Patch_Source.

Chưa chạy Cubase 15 Windows, pluginval Windows, Inno Setup/macOS pkg, tháo/nạp DLL thật, nhiều instance cùng project, hardware ASIO, hay so nghe vocal của anh với vendor. Không sử dụng log pluginval cũ đi kèm ZIP như kết quả tự chạy mới. Không tuyên bố đã khôi phục DSP độc quyền hoặc chất tiếng bằng bản hãng.

Sau khi build native, thứ tự xác nhận hữu ích là mở menu T-EQ rồi xóa/thay plugin; lặp thêm/xóa khi editor mở/đóng; kiểm tra bypass T-Limiter với PDC; lưu/mở lại project và thử live với routing monitor đã xác định. Nếu Cubase vẫn đơ, cần dump/thread stack đúng thời điểm để tách callback, DLL unload, graphics và worker license. Đây là các bước xác nhận còn lại, không phải kết quả đã thực hiện trên máy Linux.

## 6. Bảng 31 plugin

Bảng dưới ghi kết quả trong phạm vi harness. "Chưa thấy lỗi" không thay cho chứng nhận native DAW. CSV và JSON kèm theo có ID, version, số mẫu trễ và dữ liệu trước/sau.

| Plugin | Version | Trễ báo host ở 48 kHz | Kết quả trong phạm vi kiểm tra |
|---|---|---|---|
| T-1176 | 1.2.1 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-LA-2A | 1.2.1 | 32 mẫu / 0.667 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-LIQUID DELAY | 1.2.1 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-PANTHEON | 1.2.1 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-RVerb | 1.2.1 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-Verb | 1.2.1 | 0 mẫu / 0.000 ms | Đã vá chống mẫu NaN/Inf ở đầu vào |
| T-Comp | 2.1.4 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-EQ | 2.0.4 | 0 mẫu / 0.000 ms | Đã vá callback dùng bộ nhớ sau giải phóng |
| T-BE | 2.0.2 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-Compander | 1.2.1 | 0 mẫu / 0.000 ms | Không tái hiện lỗi; nhận guard đầu vào từ file dùng chung |
| T-Manny-EQ | 1.2.1 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-DeNoise | 1.1.2 | 512 mẫu / 10.667 ms | Chưa thấy lỗi; Live trễ 10,67 ms, Quality cao hơn |
| T-API550 | 1.2.1 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-Vitamin | 1.2.1 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-HDelay | 1.2.1 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-Limiter | 1.1.0 | 258 mẫu / 5.375 ms | Đã vá host bypass mất bù trễ |
| T-Mod | 1.2.1 | 0 mẫu / 0.000 ms | Đã vá chống mẫu NaN/Inf ở đầu vào |
| T-NS1 | 1.2.1 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-Opto | 1.2.1 | 0 mẫu / 0.000 ms | Đã vá chống mẫu NaN/Inf ở đầu vào |
| T-Pitch | 1.1.2 | 5760 mẫu / 120.000 ms | Chưa thấy lỗi; Music mặc định trễ 120 ms, Live khoảng 50 ms |
| T-TapeFlanger | 1.2.1 | 0 mẫu / 0.000 ms | Đã vá chống mẫu NaN/Inf ở đầu vào |
| T-Lex224 | 1.3.0 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-PultecEQ | 1.3.0 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-PurePlate | 1.3.0 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-VoxBox | 1.3.0 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-D82 Sonic | 2.4.2 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-Verb 2 | 3.0.2 | 0 mẫu / 0.000 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-DeEsser | 1.0.2 | 64 mẫu / 1.333 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-RVox | 1.0.2 | 64 mẫu / 1.333 ms | Chưa phát hiện lỗi trong các phép thử đã chạy |
| T-AutoKey | 1.7.3 | 0 mẫu / 0.000 ms | Chưa thấy lỗi; overlay 1.7.3 trên nền đầy đủ 1.7.2 |
| T-AutoTune | 1.7.3 | 96 mẫu / 2.000 ms | Chưa thấy lỗi; overlay 1.7.3 trên nền đầy đủ 1.7.2 |

## 7. Dữ liệu kiểm chứng

- Evidence/baseline/ và Evidence/after_fixes/: log vòng đời và Signal JSON trước/sau; JSON có kết quả cấp phát, nonfinite và impulse bypass.
- Evidence/special/: log 6 regression, license, CapKey, AutoTune DSP và bốn ca callback T-EQ.
- Evidence/*results.json, plugin-inventory.json và input-archives.json: tóm tắt kết quả, metadata và hash đầu vào.
- PATCH_MANIFEST.json: hash trước/sau đúng 5 file; SOURCE_CHANGES.patch: diff có thể rà soát; Patch_Source/: nội dung nguồn cuối cùng, không chứa SDK/runtime QA hay khóa thử.

Hash bản đầy đủ 1.7.2 dùng làm nền VocalPack: f66aa934689f055afc5af44c4cfd329e6e9194d50d0a2b2a9e228b6d25c2bf62. Giá trị đã đối chiếu trực tiếp với file nền, cũng có trong input-archives.json.
