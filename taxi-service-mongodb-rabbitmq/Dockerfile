# STAGE 1: Сборка
FROM ghcr.io/userver-framework/ubuntu-22.04-userver-pg-dev:latest AS builder

RUN apt-get update && apt-get install -y \
    libhiredis-dev \
    pkg-config \
    libmongocrypt-dev

WORKDIR /app

COPY cmake/ ./cmake/
COPY CMakeLists.txt .
COPY src/ ./src/
COPY configs/ ./configs/

RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --target taxi_api -j$(nproc) && \
    strip taxi_api

# STAGE 2: Рантайм
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
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
    libsnappy1v5 \
    libhiredis0.14 \
    libmongocrypt0 \
    libatomic1 \
    ca-certificates \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/taxi_api ./taxi_api
COPY --from=builder /app/configs ./configs
COPY openapi.yaml ./openapi.yaml

RUN useradd -m -u 1000 taxi && chown -R taxi:taxi /app
USER taxi

EXPOSE 8080

CMD ["./taxi_api", "--config", "/app/configs/static_config.yaml"]