#!/bin/bash

mkdir -p "$HOME"/.local/{bin,lib,share}

post_create_git()
{
    # SSH urls don't work for external repos in codespaces, we use HTTPS instead
    # Additionally, --local insteadOf doesn't seem to be honored by the current git version, so we have to use --global
    git config --global url."https://github.com/".insteadOf "git@github.com:"
    git submodule update --init --recursive
    git config --global --unset url."https://github.com/".insteadOf
}

post_create_tldr()
{
    tldr --update
}

post_create_clangd()
{
    sudo update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-"$DEVCONTAINER_CLANG_VERSION" 100
}

post_create_git
post_create_tldr
post_create_clangd

