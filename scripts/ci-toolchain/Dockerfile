FROM ubuntu:24.04@sha256:f610ab94648195aa356059f5b41d6085c9d4d903c072430cdd1af7bdb646106b
COPY common.sh /opt/provision/common.sh
RUN bash /opt/provision/common.sh
COPY qt.sh /opt/provision/qt.sh
ARG CI_QT_VERSION
RUN bash /opt/provision/qt.sh
