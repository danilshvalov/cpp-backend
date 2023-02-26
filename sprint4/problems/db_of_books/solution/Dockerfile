FROM ubuntu/postgres:14-22.04_beta

ENV POSTGRES_PASSWORD=Sd147852
ENV POSTGRES_HOST=postgres
ENV POSTGRES_PORT=5432

RUN apt update && \
    apt install -y \
      python3-pip \
      cmake \
    && \
    pip3 install conan==1.59.0

COPY conanfile.txt /app/
RUN mkdir /app/build && cd /app/build && \
    conan install .. --build=missing -s compiler.libcxx=libstdc++11

COPY ./src /app/src
COPY CMakeLists.txt /app/

RUN cd /app/build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build .

FROM ubuntu:22.04 as run

RUN groupadd -r www && useradd -r -g www www
USER www

COPY --from=build /app/build/db_of_books /app/

ENTRYPOINT ["/app/db_of_books", "/app/data/config.json", "/app/static"]
