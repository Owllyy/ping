## 1. Ping basique

```bash
sudo ./ft_ping localhost
sudo ./ft_ping 8.8.8.8
sudo ./ft_ping google.com
```

## 2. Mode verbose (-v)

```bash
sudo ./ft_ping -v localhost
sudo ./ft_ping -v 8.8.8.8
```

## 3. TTL custom (--ttl)

```bash
sudo ./ft_ping --ttl 64 8.8.8.8

sudo ./ft_ping --ttl 1 8.8.8.8

sudo ./ft_ping -v --ttl 1 8.8.8.8
```

## 4. Host unreachable

```bash
sudo ./ft_ping 192.0.2.1
sudo ./ft_ping -v 192.0.2.1
```

## 5. DNS resolution

```bash
sudo ./ft_ping google.com
sudo ./ft_ping github.com
```

## 6. DNS inexistant

```bash
sudo ./ft_ping cettadressenexistepas.xyz
```

## 7. Filtrage par ID

```bash
sudo ./ft_ping 8.8.8.8 &
sudo ./ft_ping 8.8.8.8
```