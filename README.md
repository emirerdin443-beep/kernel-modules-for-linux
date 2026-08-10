# Simple Kernel Module

Selamlar. Bir türk olarak bu benim yaptığım ilk repom ve ilk yazıldığım kernel modülü. Ilerde daha fazla modül vb. ekleyeceğim. İyi kullanımlar!

## Proje Hakkında

Bu repo, Linux kernel'ine yeni bir modül yüklemek için temel bir örnek içermektedir. "Hello World" kernel modülü kullanarak kernel programlamasına giriş yapabilirsiniz.

## İçindekiler

- `hello.c` - Kernel modülü kaynak kodu
- `Makefile` - Derleme dosyası
- `README.md` - Bu dosya

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

## Derleme

Modülü derlemek için:

```bash
make
```

Bu komut `hello.ko` dosyasını oluşturacaktır.

## Kullanım

### Modülü Yüklemek

```bash
sudo insmod hello.ko
```

Başarılı olup olmadığını kontrol etmek için:

```bash
dmesg | tail -n 1
```

Çıktı şuna benzer olmalıdır:
```
[timestamp] Hello, Kernel!
```

### Modülü Kaldırmak

```bash
sudo rmmod hello
```

Çıktı şu şekilde görünmelidir:
```
[timestamp] Goodbye, Kernel!
```

## Modülü Doğrulama

Yüklü modülleri listelemek için:

```bash
lsmod | grep hello
```

## Temizleme

Derlenmiş dosyaları silmek için:

```bash
make clean
```

## Sorun Giderme

### "Permission denied" hatası
- Tüm komutları `sudo` ile çalıştırın.

### "Module not found" hatası
- Modülün doğru derlendiğini kontrol edin: `ls -la hello.ko`
- Modülün tam yolunu belirtin: `sudo insmod ./hello.ko`

### Kernel headers bulunamadı
- Sistem için doğru kernel headers'ı yükleyin
- `uname -r` ile kernel versiyonunuzu kontrol edin

## Kaynaklar

- [Linux Kernel Module Programming Guide](https://tldp.org/LDP/lkmpg/2.6/html/)
- [Kernel.org Documentation](https://www.kernel.org/doc/)

---

**Not:** Bu modül eğitim amaçlıdır. Production ortamında kullanmadan önce iyi test edin!
