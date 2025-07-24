# Webserv

[![C++98](https://img.shields.io/badge/C%2B%2B-98-blue.svg)](https://en.cppreference.com/w/cpp/98)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-informational)]()
[![License](https://img.shields.io/badge/license-42%20Project-lightgrey)]()

> 🖥️ A lightweight HTTP/1.1-compliant web server written in C++98 — from scratch.

---

## 📚 Table of Contents

- [About](#about)
- [Features](#features)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Configuration](#configuration)
- [Project Structure](#project-structure)
- [Examples](#examples)
- [Authors](#authors)
- [License](#license)

---

## 📖 About

**Webserv** is a complete, event-driven HTTP server built in C++98.  
It was developed as part of the advanced system programming module at [42 School](https://42.fr), with the goal of replicating the behavior of production-grade servers like **Nginx** or **Apache**.

Key goals:

- Learn network programming from scratch
- Understand event-driven architectures (`poll()`)
- Implement a custom configuration parser
- Handle concurrent clients and multiple servers

---

## ✨ Features

- 🌐 **HTTP/1.1** support (`GET`, `POST`, `DELETE`)
- 🖥️ **Non-blocking I/O** with `poll()`
- 🧠 **Custom configuration format**
- 💡 **Virtual hosts** and multiple ports
- 📁 Static file serving with auto-indexing
- 🐍 **CGI** support (e.g. Python, PHP)
- 📄 Custom **error pages**
- 🔄 Chunked transfer encoding
- ⏱ Connection timeout and socket reuse

---

## ⚙️ Getting Started

### ✅ Requirements

- Linux or macOS
- `g++` compiler supporting **C++98**
- `make`

### 🔧 Build the server

```bash
git clone https://github.com/ocussy/webserv.git
cd webserv
make
