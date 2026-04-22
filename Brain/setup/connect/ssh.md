# Setup SSH trên Pi 5 dùng ubuntu 24.04 LTS

Cắm màn hình, chuột, bàn phím
Kết nối wifi

## Cài đặt SSH
```bash
sudo apt update
sudo apt install openssh-server -y
sudo systemctl enable ssh
sudo systemctl start ssh
```

## Kết nối ethernet qua wifi
Xem IP wifi trên Pi
```bash
ssh aitek@192.168.1.6
```


## Kết nối pi qua ethernet
Trên pi5
```bash
sudo nano /etc/netplan/01-netcfg.yaml
```
```
network:
  version: 2
  ethernets:
    eth0:
      addresses:
        - 192.168.2.2/24
      dhcp4: false
      dhcp6: false
      match:
        macaddress: "2c:cf:67:65:8b:a1"
      set-name: "eth0"
```
```bash
sudo systemctl enable systemd-networkd
sudo systemctl start systemd-networkd
sudo netplan apply
```
Cắm dây ethernet vào máy tính
**Trên PC linux**
```bash
sudo ip addr add 192.168.2.1/24 dev eth0
sudo ip link set eth0 up
```
Thay eth0 bằng tên card mạng của bạn (kiểm tra bằng ip addr).
Kết nối qua ethernet
```bash
ssh aitek@192.168.2.2
```

### Setup wifi sau khi ssh được ethernet

**Kết nối qua Wifi**
```bash
sudo nmcli dev wifi connect "TênWifi" password "PasswordWifi"
```

**Xem địa chỉ ip wifi trên Pi5**
```bash
ip addr show wlan0
```
**ssh**
```bash
ssh aitek@...
```