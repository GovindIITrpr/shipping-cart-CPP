FROM ubuntu:22.04
RUN apt-get update && apt-get install -y build-essential ca-certificates && update-ca-certificates
WORKDIR /app
COPY . .
RUN mkdir -p build && cd build && cmake .. && make -j
# If your binary name differs, adjust path:
CMD ["./build/Release/dragon_shopping_cart.exe"]