FROM alpine:latest AS builder

RUN apk add --no-cache gcc g++ musl-dev cmake make zeromq-dev pkgconf

WORKDIR /app
COPY . .

RUN mkdir build && cd build && \
    cmake .. && \
    make

FROM alpine:latest AS client-image
RUN apk add --no-cache libzmq
WORKDIR /app
COPY --from=builder /app/build/client .
ENTRYPOINT ["./client"]

FROM alpine:latest AS server-image
RUN apk add --no-cache libzmq
WORKDIR /app
COPY --from=builder /app/build/server .
EXPOSE 5555
ENTRYPOINT ["./server"]