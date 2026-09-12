FROM debian:bookworm
RUN apt-get update && apt-get install -y \
    build-essential gdb git \
    libseccomp-dev \
    libcgroup-dev \
    strace \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /workspace