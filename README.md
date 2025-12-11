📊 IoT Smart Energy Monitoring System with MQTT & Node-RED

Hệ thống giám sát năng lượng thông minh với ESP32, SHT31, PZEM-004T, MQTT Cloud và Node-RED Dashboard

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

Reset năng lượng PZEM bằng nút nhấn

Điều khiển relay qua MQTT

Auto-reconnect WiFi & MQTT

☁️ Cloud & Dashboard

MQTT over TLS

Last Will Testament (LWT)

Node-RED real-time graph, gauge, switch

Temperature protection

System info: heap, RSSI, uptime

## 🏗️ Kiến trúc hệ thống
Hình 2: Sơ đồ kiến trúc hệ thống 
<img width="2160" height="951" alt="Screenshot 2025-12-11 183854" src="https://github.com/user-attachments/assets/b83caf4f-b604-44be-b2a0-e885936b6622" />

Chú thích:

Khối cảm biến: PZEM004T + DHT22/SHT31 gửi dữ liệu về ESP32

ESP32: xử lý dữ liệu, điều khiển relay, publish/sub MQTT

Khối chấp hành: Relay điều khiển đóng/ngắt tải

MQTT Broker: trung chuyển dữ liệu giữa ESP32 ↔ Node-RED

Node-RED Dashboard: hiển thị dữ liệu & điều khiển từ xa

## 🔧 Sơ đồ mạch
Hình 3: Sơ đồ nguyên lý đầy đủ thực hiện trên Fritzing
<img width="2248" height="974" alt="Screenshot 2025-12-11 190727" src="https://github.com/user-attachments/assets/52d9bb47-4bdb-4646-8dab-3f0ae2fca1eb" />
Hình 3.2: Sơ đồ nguyên lý đầy đủ thực hiện trên KiCad
<img width="901" height="1014" alt="Screenshot 2025-12-11 120752" src="https://github.com/user-attachments/assets/7a3aaa61-e624-414e-bc14-96303d5d4df6" />
Hình 4: PCB Layout thực hiện trên KiCad
<img width="540" height="657" alt="Screenshot 2025-12-11 120809" src="https://github.com/user-attachments/assets/7bc55156-0b4a-463d-ae9a-c56c1a2c7d13" />
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

📦 Thư viện sử dụng (PlatformIO)

PubSubClient@^2.8 — MQTT

DHT sensor library@^1.4.6

PZEM-004T-v30 — đo điện năng

Adafruit SHT31 Library@^2.2.2

LiquidCrystal_I2C@^1.1.4

## 📊 Node-RED Dashboard

Node-RED: v4.1.0

Node.js: v20.19.5

Dashboard UI: node-red-dashboard

✔ Hình 8: Dashboard hiển thị real-time

(Chèn ảnh Node-RED vào đây)

## 🎥 Demo Video

▶ Xem video demo đầy đủ tại:
👉 (Thêm link Google Drive hoặc YouTube)

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
