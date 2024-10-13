# Use a minimal base image like Ubuntu or Debian
FROM ubuntu:20.04

# Set the working directory inside the container
WORKDIR /app

# Install g++ and cmake
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    tmux \
    gdb \
    python3 \
    iproute2 \
    sudo \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Add sudo user with password "dcl"
RUN useradd -m -s /bin/bash -G sudo dcl
RUN echo "dcl:dcl" | chpasswd

# Copy your local project into the container (adjust the path as needed)
COPY . /app

# Start an interactive shell session by default
CMD ["/bin/bash"]
