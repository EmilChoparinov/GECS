FROM ubuntu:jammy-20240405

RUN apt-get update && apt-get install build-essential -y

WORKDIR /virtual