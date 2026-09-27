## ESP-IDF Project

```bash
$ idf.py --version
ESP-IDF v6.1
```

## Schematics

```mermaid
graph LR
    VCC["3.3V"] --- POT["Potentiometer"]
    POT --- GND1["GND"]
    POT -- Vout --- GPIO2["gpio02"]
```

## Commands

```shell
# Build project
$ idf.py app

# Flash firmware
$ idf.py flash

# Monitor
$ idf.py monitor

# Clean build files
$ idf.py fullclean
```
