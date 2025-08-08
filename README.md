# my_shell - Простая оболочка Linux

### Описание
Оболочка командной строки, написанная на языке C, с базовыми функциями и специальными возможностями.  
**Доступна как Docker-образ** (исходный код скрыт).

---

## 🔧 Функционал

### Основные команды
- Чтение и выполнение пользовательских команд:
  - `echo [текст]` - вывод указанного текста
  - Поддержка выхода из оболочки (`exit`, `\q`, `Ctrl+D`)
  - `\e $VAR` - вывод переменных окружения

### Специальные возможности
- Проверка корректности вводимых команд
- Проверка загрузочных дисков (`\l sda`)
- Дамп памяти процесса (`\mem PID`)
- Обработка сигнала SIGHUP
- Сохранение истории команд в файл `history.txt`

---

## 🚀 Быстрый старт

### Вариант 1: Сборка из исходников
1. Скомпилируйте:
```bash
gcc -o my_shell my_shell.c
```
2. Запустите:
```bash
./my_shell
```

### Вариант 2: Запуск через Docker (без компиляции)
[![Docker Image](https://img.shields.io/docker/pulls/vikkkaaa/my-shell?label=Docker%20Hub)](https://hub.docker.com/r/vikkkaaa/my-shell)
1. Скачать образ
```bash
docker pull vikkkaaa/my-shell
```
2. Запустить
```bash
docker run -it vikkkaaa/my-shell
```

## 🐋 Docker-сборка (для разработчиков)
Если нужно пересобрать образ:
```bash
docker build -t my-shell .
docker run -it my-shell
```



---
[Мой Docker Hub](https://hub.docker.com/u/vikkkaaa)
