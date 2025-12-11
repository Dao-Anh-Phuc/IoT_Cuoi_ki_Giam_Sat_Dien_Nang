📊 IoT Smart Energy Monitoring System with MQTT & Node-RED

Hệ thống giám sát năng lượng thông minh với ESP32, SHT31, PZEM-004T, LCD 16X2, Relay, MQTT Cloud và Node-RED Dashboard

## 📋 Mục lục

🎯 Giới thiệu

⚡ Tính năng

🏗️ Kiến trúc hệ thống

🔧 Sơ đồ mạch

🎨 Thiết kế vỏ hộp 3D

💻 Cài đặt phần mềm

📊 Node-RED Dashboard

🎥 Demo Video

🐛 Troubleshooting

📚 Tài liệu tham khảo

👨‍💻 Tác giả

## 🎯 Giới thiệu

Dự án Smart Energy Monitoring & Protection System là giải pháp IoT toàn diện giúp giám sát, cảnh báo và bảo vệ thiết bị điện trong gia đình, doanh nghiệp và đặc biệt là các máy móc công nghiệp nhạy cảm với nhiệt độ.

Hình 1: Hình ảnh mô hình thực tế
<img width="960" height="1280" alt="image" src="https://github.com/user-attachments/assets/46fc7264-54da-4b89-9d57-f3c69ed0619f" />

Hệ thống sử dụng ESP32 làm bộ xử lý trung tâm, kết hợp với:

PZEM-004T v3.0 – đo điện áp, dòng, công suất, năng lượng (kWh)

SHT31 / DHT22 – cảm biến nhiệt độ & độ ẩm

Relay cách ly 220VAC – điều khiển đóng/ngắt tải

MQTT Cloud (EMQX) – truyền dữ liệu bảo mật TLS/SSL

Node-RED Dashboard – giao diện trực quan real-time

## ⚡ Tính năng
🔌 Giám sát điện năng (PZEM-004T)

Điện áp: 80–260V AC

Dòng điện: 0–100A

Công suất: 0–22kW

Năng lượng: 0–9999.99kWh

Tần số: 45–65Hz

Hệ số công suất: 0.00–1.00

🌡️ Giám sát môi trường (SHT31)

Nhiệt độ: -40°C → +125°C

Độ ẩm: 0–100% RH

Giao tiếp: I2C (0x44)

Response time: < 8s

📊 Hiển thị & Điều khiển

LCD 16x2 — hiển thị luân phiên Voltage/Current, Power/Energy, Temp/Humidity

Reset năng lượng PZEM bằng nút nhấn và có thể thông qua giao diện trên dashboard

Điều khiển relay qua MQTT 

Auto-reconnect WiFi & MQTT 

☁️ Cloud & Dashboard

MQTT over TLS

Last Will Testament (LWT)

Node-RED real-time graph, gauge, switch

Temperature protection (có bảo vệ tải khi quá nhiệt độ)

System info: heap, RSSI, uptime

## 🏗️ Kiến trúc hệ thống
Hình 2.1: Sơ đồ kiến trúc hệ thống (theo khối)
<img width="2041" height="935" alt="Screenshot 2025-12-11 233625" src="https://github.com/user-attachments/assets/6351b9fd-daba-4061-a4cb-7e0df4106a53" />

Chú thích:

Khối cảm biến: PZEM004T + DHT22/SHT31 gửi dữ liệu về ESP32

ESP32: xử lý dữ liệu, điều khiển relay, publish/sub MQTT

Khối chấp hành: Relay điều khiển đóng/ngắt tải

MQTT Broker: trung chuyển dữ liệu giữa ESP32 ↔ Node-RED

Node-RED Dashboard: hiển thị dữ liệu & điều khiển từ xa

Hình 2.2: Sơ đồ tổng quan hoạt động của code 


<img width="747" height="989" alt="Screenshot 2025-12-12 002309" src="https://github.com/user-attachments/assets/07a17a5b-ca48-4a6b-8bcf-7af5a0b90a9f" />


## 🔧 Sơ đồ mạch
Hình 3.1: Sơ đồ nguyên lý đầy đủ thực hiện trên Fritzing
<img width="2248" height="974" alt="Screenshot 2025-12-11 190727" src="https://github.com/user-attachments/assets/52d9bb47-4bdb-4646-8dab-3f0ae2fca1eb" />

Chú thích:
Cấu hình chân (Pin Configuration)

LED_RESET_PIN (GPIO 5): Chân điều khiển LED dùng để báo hiệu khi thực hiện thao tác reset PZEM.

RELAY_PIN (GPIO 18): Chân điều khiển relay, hoạt động ở mức logic LOW để kích hoạt.

BUTTON_PIN (GPIO 23): Chân kết nối nút nhấn, sử dụng để reset chỉ số năng lượng.

SHT31_SDA (GPIO 21): Chân SDA cho giao tiếp I2C với cảm biến nhiệt độ – độ ẩm SHT31.

SHT31_SCL (GPIO 22): Chân SCL cho giao tiếp I2C với cảm biến SHT31.

PZEM_RX (GPIO 26): Chân nhận dữ liệu (RX) từ module đo điện PZEM-004T.

PZEM_TX (GPIO 27): Chân truyền dữ liệu (TX) đến module PZEM-004T.

Hình 3.2: Sơ đồ nguyên lý đầy đủ thực hiện trên KiCad
<img width="1030" height="1154" alt="image" src="https://github.com/user-attachments/assets/3fe5eed9-16d1-4947-94f3-23e72ac7d67a" />


Hình 4: PCB Layout thực hiện trên KiCad


<img width="583" height="701" alt="Screenshot 2025-12-11 222242" src="https://github.com/user-attachments/assets/9b1ae63f-7d4c-4c41-9938-a73bb56fe15c" />


Lưu ý: Chúng ta lên cách ly nguồn 220V ra khỏi mạch
       Có thể dùng nguồn chung 220V hạ xuống 5V cấp cho vi điều khiển nhưng không an toàn

Hình 5: Mạch hoàn thiện sau khi hàn linh kiện
<img width="2568" height="1926" alt="image" src="https://github.com/user-attachments/assets/c3cab5bc-79a2-4162-be97-c3c237e6c51a" />

## 🎨 Thiết kế vỏ hộp 3D
📏 Thông số thiết kế
Kích thước: 145 × 125 × 60 mm
Vật liệu: PLA / ABS
Độ dày: 2.5 mm
Infill: 20%
Layer height: 0.2 mm
Thời gian in: ~10 giờ

Hình 6: View từ Fusion 360
<img width="1660" height="1026" alt="image" src="https://github.com/user-attachments/assets/33b2b4e8-84a1-4eba-bc06-4716890f1d0a" />

Hình 7: Hộp hoàn thiện
<img width="960" height="1280" alt="image" src="https://github.com/user-attachments/assets/9f1deebc-eae2-4741-aae3-0d7ca7138991" />

## 💻 Cài đặt phần mềm
🔧 Yêu cầu hệ thống

Visual Studio Code + PlatformIO

Git

Node-RED (tùy chọn cho Dashboard)

📦 Thư viện sử dụng (PlatformIO) dùng cho dự án 

PubSubClient@^2.8 — MQTT

DHT sensor library@^1.4.6

PZEM-004T-v30 — đo điện năng

Adafruit SHT31 Library@^2.2.2

LiquidCrystal_I2C@^1.1.4

## 📊 Node-RED Dashboard (Giao diện hiển thị) 

Node-RED: v4.1.0

Node.js: v20.19.5

Dashboard UI: node-red-dashboard

✔ Hình 8.1: Dashboard hiển thị real-time -1

<img width="2039" height="1216" alt="Screenshot 2025-12-11 231956" src="https://github.com/user-attachments/assets/9f738893-ffd1-4b30-8770-9d7fe1afc125" />


✔ Hình 8.2: Dashboard hiển thị real-time -2

<img width="2164" height="1168" alt="Screenshot 2025-12-11 232012" src="https://github.com/user-attachments/assets/78014f4c-4c4a-46ca-9896-d171f941a7fc" />


## 🎥 Demo Video và hình ảnh demo

Hình 9: Chạy với quạt là tải 

<img width="960" height="1280" alt="image" src="https://github.com/user-attachments/assets/877d7a07-389a-4422-8d07-568591fc89e9" />


▶ Xem video demo đầy đủ tại:
https://drive.google.com/drive/folders/18y1VxEVjbnSewBJiIceqecjl6ZEWsht0?usp=sharing

## 🐛 Troubleshooting
1️⃣ LCD hiển thị lỗi

Nguyên nhân: nhiễu I2C, sụt áp, tốc độ I2C cao
Khắc phục:

Rút ngắn dây I2C

Thêm tụ 100nF + 10µF

Wire.setClock(50000);

2️⃣ ESP32 tự reset / Brownout

Nguyên nhân: nguồn yếu, nhiễu tải, relay gây sụt áp
Khắc phục:

Tụ 470–1000µF gần ESP32

Relay có opto cách ly

Dùng HLK-PM01 chất lượng tốt

3️⃣ PZEM trả về 0 hoặc NaN

Nguyên nhân: nhiễu UART, dây dài, không chung GND
Khắc phục:

Rút ngắn dây

Thêm ferrite bead

Retry khi đọc lỗi

## 📚 Tài liệu tham khảo

ESP32 Technical Reference Manual

PZEM-004T v3.0 Datasheet

SHT31 Datasheet

## 👨‍💻 Tác giả

Đào Anh Phúc
📧 Email: daoanhphuc_t67@hus.edu.vn

🌐 GitHub: https://github.com/Dao-Anh-Phuc

🤝 Đóng góp, issues và feature requests luôn được chào đón!
