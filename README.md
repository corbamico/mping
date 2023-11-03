# mping
multiple addresses ping using asio as command as `mping 192.168.1.1/24`

output as:

```cmd
ping range from: 192.168.1.1 to: 192.168.1.254
Reply from 192.168.1.1: bytes=43 time=10ms TTL=64
Reply from 192.168.1.80: bytes=43 time=10ms TTL=128
Reply from 192.168.1.113: bytes=43 time=10ms TTL=64
Reply from 192.168.1.136: bytes=43 time=10ms TTL=128
stat:
    send address: 254, reach address: 4
```

## compile

```shell
#cl vs2022 buildtools in windows
meson setup build.vs2022
meson compile -C build.vs2022
```

```shell
#gcc msys2 in windows
CXX=gcc meson setup build.gcc
meson compile -C build.gcc
```

```shell
#gcc in devcontainer
CXX=gcc meson setup build.gcc
meson compile -C build.gcc
```

```shell
#clang msys2 in windows
CXX=clang meson setup build.clang
meson compile -C build.clang
```

```shell
#clang in devcontainer
CXX=clang meson setup build.clang
meson compile -C build.clang
```