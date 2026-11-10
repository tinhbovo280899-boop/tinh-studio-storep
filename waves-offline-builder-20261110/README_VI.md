# Bộ tạo Waves offline — EXE nhỏ và dữ liệu BIN

Bản sửa cho mục tiêu **cài máy mới**, từ file `Tao_Bo_Cai_Waves_Pro.bat` đã gửi. File này là **công cụ tạo bộ cài**, không chứa plugin Waves, runtime hoặc `Patch.exe`.

## Chạy trên máy đang có Waves V16

1. Giải nén ZIP. Đặt **`Tao_Bo_Cai_Waves_Pro_EXE_BIN.bat`** vào thư mục làm việc riêng, ngoài các thư mục đang cài Waves.
2. Máy đóng gói cần **Inno Setup 6**. Đặt `VC_redist.x64.exe` cạnh BAT; bản sửa cũng thử lấy file đó từ thư mục Waves Central nếu có trên máy nguồn. Thiếu runtime x64 thì dừng. Runtime x86 được lấy/đóng kèm khi có.
3. Nếu dùng `Patch.exe` trong quy trình cũ, đặt file đó cạnh BAT. Bản sửa giữ bước gọi công cụ này như file gốc; chưa kiểm chứng nội dung hoặc tham số silent của EXE đó.
4. Bấm đúp **đúng BAT mới** và chấp nhận UAC khi Windows hỏi. Công cụ tạo payload, sinh `Waves_V16_Pro.iss`, gọi ISCC và mở thư mục kết quả khi thành công.

Kết quả nằm trong **`Output\Build_<số>_<số>\`**, ví dụ:

```text
Setup_Waves_V16_Offline_Pro.exe
Setup_Waves_V16_Offline_Pro-1.bin
```

**Mang cả EXE và tất cả BIN, giữ nguyên tên, đặt cùng một thư mục.** Với gói cỡ 1 GB, dữ liệu thường nằm trong một BIN; gói lớn hơn có thể có thêm `-2.bin`, `-3.bin`... Chạy EXE để cài. Máy nhận không cần Inno Setup hoặc Waves Central để chạy trình cài này. Khả năng plugin hoạt động đầy đủ trên máy sạch vẫn cần được thử với payload thực.

## Hai phần đã sửa chính

- **Không nhúng toàn bộ dữ liệu trong EXE:** dùng `DiskSpanning=yes`, slice tối đa 2.100.000.000 byte, một slice mỗi disk. Giữ `SolidCompression=no` và `lzma2/fast`. EXE chứa trình cài, phần dữ liệu chuyển sang BIN.
- **Xin quyền đúng:** BAT kiểm tra token Administrator rồi gọi `RunAs`, không dùng `net session`. Setup sinh ra có `PrivilegesRequired=admin` và không bật đường override sang cài thường.

Nếu chạy từ cửa sổ đã có quyền Admin, hoặc Windows được cấu hình không hiện hộp UAC, **không thấy câu hỏi UAC lần nữa là bình thường**. Script không thay đổi cấu hình UAC của máy.

Việc tách BIN giúp EXE không mang khối payload 1 GB. Không cam kết một số giây mở cố định: antivirus, SmartScreen, ổ đĩa và vị trí chạy từ mạng/USB vẫn có thể làm chậm. Chưa đo thời gian mở bộ Waves thật trong lần sửa này.

## Những sửa kèm theo

- Dừng khi Robocopy thất bại, thiếu x64 WaveShell, bundle rỗng, thiếu runtime x64 hoặc ISCC trả lỗi; kiểm tra có EXE và BIN trước khi in hoàn tất.
- Mỗi lần build dùng thư mục output mới để tránh lấy nhầm EXE cũ. Cú pháp `&`/`>` trong tiêu đề BAT được sửa.
- Gom nguyên file hoặc nguyên thư mục WaveShell, gồm tài nguyên bên trong bundle. Pattern tên WaveShell vẫn theo cách nhận diện trong BAT gốc; chưa có kiểm đếm đủ 47 plugin.
- Thử dừng `WavesLocalServer.exe` trước khi chép file ở máy đích. Không tìm thấy tiến trình này trên máy mới không làm hỏng cài đặt.
- Giữ cách gom Waves/ProgramData/Registry và luồng xử lý license/công cụ bổ sung từ BAT gốc, gồm cleanup `.wle` trên máy đích. Bản này hướng tới **máy mới**, không phải công cụ nâng cấp/gỡ chọn lọc một bộ Waves đang dùng.
- Không tự chạy `Patch.exe` hoặc các redistributable trong lúc tạo bộ cài. Chúng chỉ được đóng kèm để Setup gọi theo script. Chưa xác nhận kết quả kích hoạt hoặc đầy đủ dependency của payload thật.

Nếu lỗi, đọc thông báo ở cửa sổ và **`Waves_Build.log`** cạnh BAT. Trình cài sinh ra cũng bật `SetupLogging=yes`.

## Phạm vi kiểm tra

Đã kiểm tra cú pháp các lệnh PowerShell nhúng, cấu hình ISS được sinh và các ca sao chép WaveShell file/bundle bằng dữ liệu giả trên Linux với PowerShell 7.5.4. Có ca đường dẫn chứa khoảng trắng, dấu nháy đơn, `&`, `!`, `%` và Unicode; có ca thiếu/rỗng WaveShell.

**Chưa chạy CMD/Windows PowerShell 5.1, UAC, ISCC hoặc cài Waves thật trên Windows.** Cần chạy trên máy đóng gói của anh và thử kết quả trên máy Windows mới. Thư mục `Validation` có log, kết quả kiểm tra và ISS minh họa; ISS đó chỉ để xem, không thay cho ISS được BAT sinh với dữ liệu thật.

Tài liệu Inno chính thức: [DiskSpanning](https://jrsoftware.org/ishelp/topic_setup_diskspanning.htm), [DiskSliceSize](https://jrsoftware.org/ishelp/topic_setup_diskslicesize.htm), [PrivilegesRequired](https://jrsoftware.org/ishelp/topic_setup_privilegesrequired.htm).
