
# dirwarden

A fast, lightweight directory bruteforcing tool written in C++. Built as a learning project to understand how tools like gobuster work under the hood.

## ⚠️ Disclaimer

This tool is intended for **authorized security testing only** — use it against systems you own or have explicit permission to test (e.g. your own lab, HTB, pentest-ground.com). Unauthorized scanning of systems you don't have permission to test may be illegal in your jurisdiction.

## Features

- Directory/path bruteforcing via HTTP HEAD requests
- Color-coded status codes (2xx green, 3xx cyan, 4xx yellow, 5xx red)
- Automatic filtering of 404s and connection errors
- Request timing and requests/sec stats
- Simple CLI interface

## Requirements

- g++ (C++11 or later)
- libcurl

Install libcurl on Debian/Ubuntu:

```bash
sudo apt install libcurl4-openssl-dev
```

## Build

```bash
git clone https://github.com/Thamid1971/dirwarden.git
cd dirwarden
g++ main.cpp -o dirwarden -lcurl
```

## Usage

```bash
./dirwarden -u  -w
```

### Options

| Flag   | Description                              |
| ------ | ---------------------------------------- |
| `-u` | Target URL (e.g.`https://example.com`) |
| `-w` | Path to wordlist file                    |
| `-h` | Show help menu                           |

### Example

```bash
./dirwarden -u https://pentest-ground.com -w /usr/share/wordlists/dirb/common.txt
```

## Sample Output

Target:   https://pentest-ground.com
Wordlist: common.txt (4614 words)
┌───────┬────────────────────────────────────────────┐
│ Code  │ Path                                       │
├───────┼────────────────────────────────────────────┤
│   200 │ admin                                      │
│   301 │ images                                     │
│   403 │ config                                     │
└───────┴────────────────────────────────────────────┘
Summary:
Checked:  4614 paths
Found:    3 results
Time:     42.17s
Speed:    109.4 req/s

## Roadmap

- [ ] Multithreading for faster scans
- [ ] File extension bruteforcing (`-x .php,.html`)
- [ ] Custom status code filtering
- [ ] JSON/CSV export

## License

MIT