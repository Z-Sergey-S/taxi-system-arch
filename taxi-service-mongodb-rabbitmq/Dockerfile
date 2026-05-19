# STAGE 1: Сборка
FROM ghcr.io/userver-framework/ubuntu-22.04-userver-pg-dev:latest AS builder

# Устанавливаем зависимости и обновляем CMake
RUN apt-get update && apt-get install -y \
    libhiredis-dev \
    pkg-config \
    libmongocrypt-dev \
    git \
    build-essential \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Устанавливаем свежую версию CMake
RUN wget -qO- "https://github.com/Kitware/CMake/releases/download/v3.29.3/cmake-3.29.3-linux-x86_64.tar.gz" | tar --strip-components=1 -xz -C /usr/local

# Клонируем и собираем AMQP-CPP
RUN git clone https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git /tmp/amqpcpp && \
    cd /tmp/amqpcpp && \
    mkdir build && cd build && \
    /usr/local/bin/cmake .. -DCMAKE_BUILD_TYPE=Release -DAMQP-CPP_BUILD_SHARED=OFF && \
    make -j$(nproc) && \
    make install && \
    ldconfig

WORKDIR /app

COPY cmake/ ./cmake/
COPY CMakeLists.txt .
COPY src/ ./src/
COPY configs/ ./configs/

# Включаем флаг поддержки RabbitMQ при сборке
RUN mkdir -p build && cd build && \
    /usr/local/bin/cmake .. -DCMAKE_BUILD_TYPE=Release -DUSERVER_FEATURE_RABBITMQ=ON && \
    make -j$(nproc) && \
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

# Копируем собранную библиотеку AMQP-CPP из builder
COPY --from=builder /usr/local/lib/libamqpcpp.a /usr/local/lib/
COPY --from=builder /usr/local/include/amqpcpp/ /usr/local/include/amqpcpp/

WORKDIR /app

COPY --from=builder /app/build/taxi_api ./taxi_api
COPY --from=builder /app/configs ./configs
COPY openapi.yaml ./openapi.yaml

RUN useradd -m -u 1000 taxi && chown -R taxi:taxi /app
USER taxi

EXPOSE 8080

CMD ["./taxi_api", "--config", "/app/configs/static_config.yaml"]