FROM ubuntu:jammy-20240405

WORKDIR /virtual

RUN apt-get update && apt-get install build-essential clang lldb valgrind -y