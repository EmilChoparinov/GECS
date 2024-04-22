#!/bin/bash

docker build -t gecs-image .
docker run -it --rm -v "$(pwd):/virtual" gecs-image