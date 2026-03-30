# STAGE 1: Сборка
FROM ghcr.io/userver-framework/ubuntu-22.04-userver-pg-dev:latest AS builder

WORKDIR /app

COPY cmake/ ./cmake/
COPY CMakeLists.txt .
COPY src/ ./src/
COPY configs/ ./configs/

RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --target taxi_api -j$(nproc)

# STAGE 2: Рантайм
FROM ghcr.io/userver-framework/ubuntu-22.04-userver-pg-dev:latest

WORKDIR /app

COPY --from=builder /app/build/taxi_api ./taxi_api
COPY --from=builder /app/configs ./configs
COPY openapi.yaml ./openapi.yaml

# Создаем пользователя для запуска
RUN useradd -m -u 1000 taxi && chown -R taxi:taxi /app
USER taxi

EXPOSE 8080

CMD ["./taxi_api", "--config", "/app/configs/static_config.yaml"]