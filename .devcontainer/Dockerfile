FROM archlinux:latest

RUN pacman --noconfirm -Syu && \
    pacman --noconfirm -S \
    git base-devel go github-cli \
    vim sudo bash-completion \
    wget curl zsh gnupg \
    lsb-release zip file \
    cmake make ninja \
    python python-pip python-virtualenv \
    clang gcc gdb \
    aarch64-linux-gnu-gcc aarch64-linux-gnu-binutils docker kubectl minikube helm\
    qemu-user qemu-user-static && \
    rm /var/cache/pacman/pkg/*

# Create a non-root user for using yay safely
RUN useradd -m -s /bin/bash dev && \
    echo 'dev ALL=(ALL) NOPASSWD:ALL' > /etc/sudoers.d/dev

# Switch to the new user
USER dev
WORKDIR /home/dev

RUN wget https://raw.githubusercontent.com/db-tu-dresden/TSL/refs/heads/main/requirements.txt && \
    python -m venv ./venv && \
    source ./venv/bin/activate && \
    pip install --no-cache-dir -r requirements.txt && \
    deactivate && \
    rm requirements.txt

ENV VIRTUAL_ENV=/home/dev/venv
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

# Install yay
RUN git clone https://aur.archlinux.org/yay-bin.git /home/dev/yay-bin && \
    cd /home/dev/yay-bin && \
    makepkg -si --noconfirm && \
    rm -rf /home/dev/yay-bin

# Set PATH (optional, but ensures yay is found)
ENV PATH="/home/dev/.local/bin:$PATH"

# Install an AUR package globally
RUN yay -Sy --noconfirm intel-sde oh-my-zsh-git

WORKDIR /home/dev/