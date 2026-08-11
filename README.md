# Simple Kernel Module

Selamlar. Bir türk olarak bu benim yaptığım ilk repom ve ilk yazıldığım kernel modülleri. Ilerde daha fazla modül vb. ekleyeceğim. İyi kullanımlar.

## Proje Hakkında

Bu repo, Linux kernel'ine yeni modüller yüklemek için temel örnekler içermektedir. Farklı tipte kernel modülleri kullanarak kernel programlamasına derinlemesine giriş yapabilirsiniz.

## Modüller

Bu repoda aşağıdaki kernel modülleri bulunmaktadır:

### 1. Hello Module
- **Açıklama:** Basit "Hello World" kernel modülü
- **Dizin:** `hello/`
- **Dosyalar:** `hello.c`, `Makefile`
- **İşlevsellik:** Kernel'e yük ve boşalt mesajları yazdırır

### 2. Character Device Module
- **Açıklama:** Karakter cihazı driver'ı
- **Dizin:** `char_device/`
- **Dosyalar:** `char_device.c`, `Makefile`
- **İşlevsellik:** `/dev/simple_char` cihazı oluşturur, okuma/yazma işlemleri yapar

### 3. Simple Driver Module
- **Açıklama:** Platform device driver'ı
- **Dizin:** `simple_driver/`
- **Dosyalar:** `simple_driver.c`, `Makefile`
- **İşlevsellik:** Platform device framework kullanarak probe ve remove işlemleri gerçekleştirir

### 4. Wi-Fi Driver Module
- **Açıklama:** Network cihazı driver'ı
- **Dizin:** `wifi_driver/`
- **Dosyalar:** `wifi_driver.c`, `Makefile`
- **İşlevsellik:** `wlan%d` network arayüzü oluşturur, paket gönderme işlemi yapabilir

## Gereksinimler

- Linux işletim sistemi
- Kernel headers (kernel development files)
- GCC derleyicisi
- Make aracı

### Debian/Ubuntu için:
```bash
sudo apt-get update
sudo apt-get install build-essential linux-headers-$(uname -r)
```

### RHEL/CentOS için:
```bash
sudo yum groupinstall "Development Tools"
sudo yum install kernel-devel-$(uname -r)
```

## Genel Derleme Talimatları

Her modülü derlemek için ilgili dizine gidip `make` komutunu çalıştırın:

```bash
cd <modül_dizini>
make
```

Bu komut `.ko` dosyasını oluşturacaktır.

## Modül Kullanımı

### Hello Module

```bash
# Dizine git
cd hello

# Derle
make

# Modülü yükle
sudo insmod hello.ko

# Çıktıyı kontrol et
dmesg | tail -n 1

# Modülü kaldır
sudo rmmod hello
```

**Beklenen Çıktı:**
```
[timestamp] Hello, Kernel!
[timestamp] Goodbye, Kernel!
```

### Character Device Module

```bash
cd char_device
make
sudo insmod char_device.ko

# Device'ı kontrol et
ls -la /dev/simple_char

# Yazma işlemi
echo "Merhaba Kernel" | sudo tee /dev/simple_char

# Okuma işlemi
sudo cat /dev/simple_char

# Modülü kaldır
sudo rmmod char_device
```

### Simple Driver Module

```bash
cd simple_driver
make
sudo insmod simple_driver.ko

# Modülün çalışıp çalışmadığını kontrol et
dmesg | tail -n 5

# Modülü kaldır
sudo rmmod simple_driver
```

### Wi-Fi Driver Module

```bash
cd wifi_driver
make
sudo insmod wifi_driver.ko

# Network arayüzünü kontrol et
ifconfig -a

# Modülü kaldır
sudo rmmod wifi_driver
```

## Modülü Doğrulama

Yüklü modülleri listelemek için:

```bash
lsmod | grep <modül_adı>
```

Örnek:
```bash
lsmod | grep hello
lsmod | grep char_device
lsmod | grep simple_driver
lsmod | grep wifi_driver
```

## Temizleme

Her modülün dizininde derlenmiş dosyaları silmek için:

```bash
cd <modül_dizini>
make clean
```

Tüm modülleri temizlemek için:

```bash
cd hello && make clean && cd ..
cd char_device && make clean && cd ..
cd simple_driver && make clean && cd ..
cd wifi_driver && make clean && cd ..
```

## Sorun Giderme

### "Permission denied" hatası
- Tüm komutları `sudo` ile çalıştırın.

### "Module not found" hatası
- Modülün doğru derlendiğini kontrol edin: `ls -la *.ko`
- Modülün tam yolunu belirtin: `sudo insmod ./module.ko`

### "Unknown symbol" hatası
- Kernel headers'ın doğru yüklendiğini kontrol edin
- Kernel versiyonunuzu kontrol edin: `uname -r`
- Uygun kernel headers'ı yükleyin

### Kernel headers bulunamadı
- Sistem için doğru kernel headers'ı yükleyin
- `uname -r` ile kernel versiyonunuzu kontrol edin
- Kernel headers paketini yeniden yükleyin

### Device oluşturulamıyor
- `/dev/` dizinine yazma izniniz olup olmadığını kontrol edin
- `sudo` ile komutları çalıştırın

## Kaynaklar

- [Linux Kernel Module Programming Guide](https://tldp.org/LDP/lkmpg/2.6/html/)
- [Kernel.org Documentation](https://www.kernel.org/doc/)
- [Linux Device Drivers Book](https://lwn.net/Kernel/LDD3/)
- [Linux Networking Documentation](https://www.kernel.org/doc/html/latest/networking/)

## Repo Yapısı

```
simple-kernel-module/
├── hello/
│   ├── hello.c
│   └── Makefile
├── char_device/
│   ├── char_device.c
│   └── Makefile
├── simple_driver/
│   ├── simple_driver.c
│   └── Makefile
├── wifi_driver/
│   ├── wifi_driver.c
│   └── Makefile
└── README.md

---

**Not:** Bu modüller eğitim amaçlıdır. Production ortamında kullanmadan önce iyi test edin ve kernel programlaması hakkında derinlemesine bilgi sahibi olun!

**Yazar:** emirerdin443-beep  
**Lisans:** GPL
