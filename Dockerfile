FROM gcc

WORKDIR /app
ADD . /app

# Install prerequisites
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    libexpat1-dev \
    libevent-dev \
    libmecab-dev \
    mecab \
    cmake

# Build
RUN cmake . && make

EXPOSE 8842

CMD ["./nihongoparserd", "-p", "8842"]
