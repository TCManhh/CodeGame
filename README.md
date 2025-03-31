    BÁO CÁO BÀI TẬP LỚP MÔN LẬP TRÌNH NÂNG CAO
Sinh viên: Trương Công Mạnh		Sinh ngày: 20/08/2006	
Ngành học: Công Nghệ Thông Tin		Lớp: IT9	
Chuyên ngành: Công Nghệ Thông Tin	
Tên game : Airplane Shooter
Video test game: test game.mkv
Lối chơi, logic của game: Game thuộc thể loại bắn máy bay 2D top-down. Người chơi điều khiển máy bay bằng chuột (kéo thả hoặc nhấn di chuyển rồi theo chuột), né tránh và tiêu diệt kẻ địch để đạt điểm cao.
 -Người chơi: Tự động bắn, có thể lên tối đa 4 cấp độ dựa trên điểm số, thay đổi hình ảnh và đạn khi lên cấp. Có thanh máu, thua khi hết máu.
 -Kẻ địch: 4 loại (Normal, Straight Shooter, Weaver, Tank) xuất hiện ngẫu nhiên từ trên xuống với máu, kiểu di chuyển (thẳng, hình sin) và kiểu bắn (thẳng, theo người chơi) khác nhau.
 -Logic chính: Vòng lặp game xử lý input -> cập nhật vị trí/trạng thái đối tượng (player, địch, đạn, nổ) -> xử lý va chạm (đạn-địch, địch-player, đạn địch-player) -> tính điểm -> kiểm tra lên cấp -> kiểm tra game over -> vẽ lại màn hình.
 -Điểm số: Tăng khi diệt địch, điểm cao nhất được lưu/tải từ file.
 -Trạng thái: Game có các state rõ ràng (Menu, Game, Paused, Game Over) điều khiển luồng chơi và hiển thị.
Đồ họa, âm thanh:
 -Đồ họa: 2D, sử dụng SDL2, SDL_image, SDL_ttf. Bao gồm sprite cho người chơi (4 cấp), kẻ địch (4 loại), các loại đạn, hoạt ảnh nổ (spritesheet), hình nền (Menu, Game, Game Over), và giao diện người dùng (văn bản điểm số/cấp độ, thanh máu, nút bấm).
 -Âm thanh: Sử dụng SDL2, SDL_mixer. Bao gồm nhạc nền MP3 cho Menu và Game, hiệu ứng âm thanh WAV (nổ, bắt đầu). Có chức năng bật/tắt âm thanh
Cấu trúc của project game: Project được chia thành nhiều file để dễ quản lý và bảo trì, tách biệt khai báo (.h) và định nghĩa (.cpp):
 -Headers (.h): 
  +Constants.h: Lưu các giá trị hằng số (tốc độ, kích thước, máu...).
  +Structs.h: Định nghĩa các cấu trúc dữ liệu (Entity, Button, ExplosionEffect) và enums (GameState, EnemyType).
  +Globals.h: Khai báo extern cho các biến toàn cục (textures, vectors, state...).
  +Utils.h: Khai báo các hàm tiện ích (tải tài nguyên, vẽ UI, I/O file...).
  +GameLogic.h: Khai báo các hàm xử lý logic game chính (update, spawn, shoot...).
 -Sources (.cpp): 
  +main.cpp: Chứa hàm main, vòng lặp game, xử lý sự kiện, định nghĩa biến toàn cục, điều phối chung.
  +Utils.cpp: Chứa phần triển khai các hàm tiện ích.
  +GameLogic.cpp: Chứa phần triển khai các hàm logic game.
Các chức năng đã cài được cho game:
 -Điều khiển player linh hoạt bằng chuột (kéo thả, nhấn di chuyển/theo chuột). 
 -Player tự động bắn, hệ thống lên cấp (1-4) thay đổi hình ảnh player/đạn. 
 -4 loại kẻ địch với hành vi di chuyển, tấn công, máu/sát thương riêng biệt. 
 -Hệ thống máu và va chạm (đạn-địch, địch-player, đạn địch-player). 
 -Tính điểm khi diệt địch, lưu/tải điểm cao nhất. 
 -Quản lý trạng thái game (Menu, Game, Pause, Game Over) với giao diện tương ứng. 
 -Chức năng Pause với menu (Continue, Mute/Unmute, Exit to Menu). 
 -Hiệu ứng nổ hoạt ảnh. 
 -Âm thanh nền và hiệu ứng, có thể bật/tắt. 
 -Giao diện hiển thị thông tin cần thiết (Score, Level, Health). 
 -Tích hợp các thư viện SDL2 (Core, Image, TTF, Mixer). 
 -Cấu trúc code rõ ràng, chia thành nhiều module.
