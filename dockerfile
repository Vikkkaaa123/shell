# Этап 1: Сборка (компиляция C-кода)
FROM alpine:latest AS builder

# Устанавливаем компилятор и зависимости
RUN apk add --no-cache build-base

# Копируем исходный код
WORKDIR /app
COPY app/my_shell.c .

# Компилируем (без отладочных символов для уменьшения размера)
RUN gcc -static -Os -o my_shell my_shell.c && strip my_shell

# Этап 2: Финальный образ (только бинарник)
FROM alpine:latest

# Копируем бинарник из builder
COPY --from=builder /app/my_shell /usr/local/bin/my_shell

# Устанавливаем зависимости для работы (если нужны)
# RUN apk add --no-cache libc6-compat

# Запускаем терминал
CMD ["my_shell"]
