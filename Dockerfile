FROM alpine:latest AS builder

RUN apk add --no-cache gcc g++ musl-dev cmake make zeromq-dev pkgconf

WORKDIR /app
COPY . .

RUN mkdir build && cd build && \
    cmake .. && \
    make

FROM alpine:latest AS log-agent-image
RUN apk add --no-cache libzmq
WORKDIR /app
COPY --from=builder /app/build/log_agent .
ENTRYPOINT ["./log_agent"]

FROM alpine:latest AS log-collector-image
RUN apk add --no-cache libzmq
WORKDIR /app
COPY --from=builder /app/build/log_collector .
EXPOSE 5555
ENTRYPOINT ["./log_collector"]