# STAGE 1: Сборка
FROM ghcr.io/userver-framework/ubuntu-22.04-userver-pg-dev:latest AS builder

WORKDIR /app

COPY cmake/ ./cmake/
COPY CMakeLists.txt .
COPY src/ ./src/
COPY configs/ ./configs/

RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --target taxi_api -j$(nproc) && \
    strip taxi_api

# STAGE 2: Минимальный рантайм
FROM ubuntu:22.04

# Устанавливаем зависимости 
RUN apt-get update && apt-get install -y --no-install-recommends \
    libpq5 \
    libssl3 \
    libcurl4 \
    libev4 \
    libnghttp2-14 \
    libyaml-cpp0.7 \
    libfmt8 \          
    libzstd1 \
    libre2-9 \
    libcctz2 \
    libicu70 \
    libboost-system1.74.0 \
    libboost-filesystem1.74.0 \
    libboost-program-options1.74.0 \
    libboost-locale1.74.0 \
    libboost-thread1.74.0 \
    libboost-atomic1.74.0 \
    libboost-chrono1.74.0 \
    libboost-date-time1.74.0 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/taxi_api ./taxi_api
COPY --from=builder /app/configs ./configs
COPY openapi.yaml ./openapi.yaml

USER 1000

EXPOSE 8080

CMD ["./taxi_api", "--config-path", "configs/static_config.yaml"]