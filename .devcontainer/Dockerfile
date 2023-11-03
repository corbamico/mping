FROM mcr.microsoft.com/devcontainers/cpp

RUN wget --no-check-certificate https://github.com/chriskohlhoff/asio/archive/asio-1-28-2.tar.gz \
    && tar -xzf asio-1-28-2.tar.gz \
    && sudo cp    asio-asio-1-28-2/asio/include/asio.hpp  /usr/include \
    && sudo cp -r asio-asio-1-28-2/asio/include/asio    /usr/include 

# [Optional] Uncomment this section to install additional packages.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
     && apt-get -y install --no-install-recommends meson clang