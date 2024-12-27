# Use Ubuntu 18.04 to match the required package versions
FROM ubuntu:18.04

# Set the working directory inside the container
WORKDIR /app

# Install specific versions of g++, gcc, and cmake, along with other required packages
RUN apt-get update && \
    apt-get install -y \
    g++-7 \
    gcc-7 \
    cmake=3.10.2-1ubuntu2.18.04.2 \
    tmux \
    gdb \
    python3 \
    iproute2 \
    sudo \
    && apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Set gcc and g++ to use version 7
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 100

# Add sudo user with password "dcl"
RUN useradd -m -s /bin/bash -G sudo dcl && \
    echo "dcl:dcl" | chpasswd

# Copy your local project into the container
COPY . /app

# Start an interactive shell session by default
CMD ["/bin/bash"]
