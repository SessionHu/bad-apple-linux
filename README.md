# Bad Apple for Linux Kernel

## ASCII Art

- Download video from Bilibili first

- Use FFmpeg to generate frames in PNG

- Use `jp2a` and redirect outputs to `ba.asc`

- Use `xxd` to generate `ba.h`

## Build

```sh
make
```

## Install

```sh
sudo make install
```

## Enjoy

> You may need remove `quiet` from linux boot command to watch module outputs.

```sh
sudo reboot
```

## License

- GPLv3
