FROM ubuntu:jammy-20240405

WORKDIR /virtual

RUN apt-get update && apt-get install build-essential clang lldb valgrind -y
RUN apt-get install python3 python3-pip graphviz -y
RUN pip3 install gprof2dot

CMD ["bash"]