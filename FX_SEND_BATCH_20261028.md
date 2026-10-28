# Đợt sửa FX Send — T-Verb, vang ngắn và Delay

Giữ chất tiếng hiện tại theo lựa chọn của chủ dự án; sửa lỗi đã xác định và thêm preset FX Send riêng.

- [T-RVerb đã sửa](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/T-RVerb_V1_0_0_FIXED_WIN_MAC_SOURCE.zip)
- [T-Verb — preset mềm nhẹ FX Send](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/T-Verb_V1_0_0_FX_SEND_FIXED_WIN_MAC_SOURCE.zip)
- [PANTHEON II — preset vang ngắn FX Send](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/PANTHEON_II_V1_0_0_FX_SEND_FIXED_WIN_MAC_SOURCE.zip)
- [LIQUID Delay — đổi Time êm hơn + FX Send](https://github.com/tinhbovo280899-boop/tinh-studio-storep/raw/refs/heads/vst-fixed-downloads-20261027/LIQUID_DELAYS_II_V2_7_0_FX_SEND_FIXED_WIN_MAC_SOURCE.zip)

Các ZIP là **mã nguồn**, chưa phải plugin/bộ cài Windows hoặc macOS. Cả bốn đã build VST3 trên Linux và đạt các bộ test: T-RVerb 2/2, T-Verb 7/7 (320 kiểm tra engine), PANTHEON II 3/3, LIQUID Delay 4/4. Chưa build/test native Windows/macOS hoặc nghe đối chiếu với vocal thật.

Xem GHI_CHU_DOT_SUA_FX_SEND.md và TEST_RESULTS_BATCH trong từng ZIP để biết cách build, tên preset mới và các giới hạn. T-RVerb sửa NaN/Inf, signed predelay, mono→stereo, state/preset và tail; Delay thêm crossfade 20 ms khi đổi Time/BPM. Engine T-Verb/PANTHEON không đổi, preset mới thêm cuối danh sách. Giữ plugin ID và public license config; không có private key trong ZIP.

Với FX Send: chọn preset mới có hậu tố FX Send; return wet-only. Ở Delay, InLoop Off và cả hai delay On để tránh passthrough trực tiếp vào return.
